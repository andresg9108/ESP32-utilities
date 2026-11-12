#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fake_idf.h"
#include "agwifi.h"
#include "agHttpServer.h"

const char WIFI_EVENT[] = "wifi";
const char IP_EVENT[] = "ip";
struct fake_group { EventBits_t bits; };
static struct { esp_event_base_t base; int32_t id; } queue[64];
static unsigned queued, consumed;
static esp_event_handler_t wifi_handler, ip_handler;
static esp_netif_t netif;
static bool netif_exists, driver_initialized, fail_alloc;
static bool drop_ip, reject_connection, loop_exists, drop_stop;
static int groups, init_calls, connect_calls, starts, destroys;
static int http_starts, http_stops;
static esp_err_t nvs_error, mode_error, start_error, stop_error;
static esp_err_t registration_error, http_start_error, http_stop_error, ip_info_error;
static esp_err_t connect_error, deinit_error, unregister_error;
static int registration_count;

static void post(esp_event_base_t base, int32_t id)
{
    assert(queued < sizeof(queue) / sizeof(queue[0]));
    queue[queued].base = base;
    queue[queued++].id = id;
}

static void emit(esp_event_base_t base, int32_t id)
{
    esp_event_handler_t handler = base == WIFI_EVENT ? wifi_handler : ip_handler;
    if (handler) {
        handler(NULL, base, id, NULL);
    }
}

static void drain(void)
{
    while (consumed < queued) {
        unsigned index = consumed++;
        emit(queue[index].base, queue[index].id);
    }
    consumed = queued = 0;
}

const char *esp_err_to_name(esp_err_t err) { (void)err; return "fake error"; }
void fake_log(const char *tag, const char *format, ...)
{
    (void)tag;
    char output[256];
    va_list args;
    va_start(args, format);
    vsnprintf(output, sizeof(output), format, args);
    va_end(args);
    assert(strstr(output, CONFIG_ESP_WIFI_PASSWORD) == NULL);
}

EventGroupHandle_t xEventGroupCreate(void)
{
    if (fail_alloc) { return NULL; }
    groups++;
    return calloc(1, sizeof(struct fake_group));
}
void vEventGroupDelete(EventGroupHandle_t group)
{
    assert(!wifi_handler && !ip_handler);
    assert(group);
    groups--;
    free(group);
}
EventBits_t xEventGroupGetBits(EventGroupHandle_t group) { assert(group); return group->bits; }
EventBits_t xEventGroupSetBits(EventGroupHandle_t group, EventBits_t bits)
{ assert(group); return group->bits |= bits; }
EventBits_t xEventGroupClearBits(EventGroupHandle_t group, EventBits_t bits)
{ assert(group); EventBits_t before = group->bits; group->bits &= ~bits; return before; }
EventBits_t xEventGroupWaitBits(EventGroupHandle_t group, EventBits_t bits,
                               int clear, int all, TickType_t timeout)
{
    assert(group && !clear && !all && timeout > 0);
    while (!(group->bits & bits) && consumed < queued) {
        unsigned index = consumed++;
        emit(queue[index].base, queue[index].id);
    }
    return group->bits;
}

esp_err_t nvs_flash_init(void) { return nvs_error; }
esp_err_t esp_netif_init(void) { return ESP_OK; }
esp_err_t esp_event_loop_create_default(void)
{ if (loop_exists) { return ESP_ERR_INVALID_STATE; } loop_exists = true; return ESP_OK; }
esp_netif_t *esp_netif_get_handle_from_ifkey(const char *key)
{ assert(!strcmp(key, "WIFI_STA_DEF")); return netif_exists ? &netif : NULL; }
esp_netif_t *esp_netif_create_default_wifi_sta(void)
{ assert(!netif_exists); netif_exists = true; return &netif; }
void esp_netif_destroy_default_wifi(void *handle)
{ assert(handle == &netif && netif_exists); netif_exists = false; destroys++; }
esp_err_t esp_netif_get_ip_info(esp_netif_t *handle, esp_netif_ip_info_t *info)
{
    assert(handle == &netif);
    if (ip_info_error != ESP_OK) { return ip_info_error; }
    memset(info, 0, sizeof(*info));
    info->ip.addr = 0x0100007f;
    return ESP_OK;
}
esp_err_t esp_event_handler_instance_register(esp_event_base_t base, int32_t id,
    esp_event_handler_t handler, void *arg, esp_event_handler_instance_t *instance)
{
    assert(id == ESP_EVENT_ANY_ID && !arg);
    if (++registration_count == 2 && registration_error != ESP_OK) {
        return registration_error;
    }
    if (base == WIFI_EVENT) { wifi_handler = handler; *instance = &wifi_handler; }
    else { ip_handler = handler; *instance = &ip_handler; }
    return ESP_OK;
}
esp_err_t esp_event_handler_instance_unregister(esp_event_base_t base, int32_t id,
    esp_event_handler_instance_t instance)
{
    assert(id == ESP_EVENT_ANY_ID && instance);
    if (unregister_error != ESP_OK) { return unregister_error; }
    if (base == WIFI_EVENT) { wifi_handler = NULL; }
    else { ip_handler = NULL; }
    return ESP_OK;
}
esp_err_t esp_wifi_get_mode(wifi_mode_t *mode)
{ *mode = WIFI_MODE_STA; return driver_initialized ? ESP_OK : ESP_ERR_WIFI_NOT_INIT; }
esp_err_t esp_wifi_init(const wifi_init_config_t *config)
{ assert(config && !driver_initialized); driver_initialized = true; init_calls++; return ESP_OK; }
esp_err_t esp_wifi_deinit(void)
{
    assert(driver_initialized);
    if (deinit_error != ESP_OK) { return deinit_error; }
    driver_initialized = false;
    return ESP_OK;
}
esp_err_t esp_wifi_set_storage(int storage) { assert(storage == WIFI_STORAGE_RAM); return ESP_OK; }
esp_err_t esp_wifi_set_mode(wifi_mode_t mode) { assert(mode == WIFI_MODE_STA); return mode_error; }
esp_err_t esp_wifi_set_config(int interface, wifi_config_t *config)
{
    assert(interface == WIFI_IF_STA);
    assert(!strcmp((const char *)config->sta.ssid, CONFIG_ESP_WIFI_SSID));
    assert(!strcmp((const char *)config->sta.password, CONFIG_ESP_WIFI_PASSWORD));
    return ESP_OK;
}
esp_err_t esp_wifi_start(void)
{
    if (start_error != ESP_OK) { return start_error; }
    starts++;
    post(WIFI_EVENT, WIFI_EVENT_STA_START);
    return ESP_OK;
}
esp_err_t esp_wifi_stop(void)
{
    if (stop_error != ESP_OK) { return stop_error; }
    if (!drop_stop) { post(WIFI_EVENT, WIFI_EVENT_STA_STOP); }
    return ESP_OK;
}
esp_err_t esp_wifi_connect(void)
{
    connect_calls++;
    if (connect_error != ESP_OK) { return connect_error; }
    if (reject_connection) { post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED); }
    else if (!drop_ip) { post(IP_EVENT, IP_EVENT_STA_GOT_IP); }
    return ESP_OK;
}
esp_err_t httpd_start(httpd_handle_t *server, const httpd_config_t *config)
{
    assert(config->server_port == 80);
    http_starts++;
    /* Simulate an SDK that writes the output even on failure.
     * Simula un SDK que escribe el parámetro de salida incluso cuando falla. */
    *server = &http_starts;
    return http_start_error;
}
esp_err_t httpd_stop(httpd_handle_t server)
{ assert(server == &http_starts); http_stops++; return http_stop_error; }

static void reset(void)
{
    stop_error = ESP_OK;
    deinit_error = unregister_error = ESP_OK;
    drop_stop = false;
    assert(agwifi_deinitialize() == ESP_OK);
    drain();
    assert(groups == 0 && !wifi_handler && !ip_handler && !driver_initialized);
    assert(!netif_exists);
    nvs_error = mode_error = start_error = registration_error = ip_info_error = ESP_OK;
    connect_error = ESP_OK;
    reject_connection = drop_ip = fail_alloc = false;
    registration_count = init_calls = connect_calls = starts = destroys = 0;
}

static void test_wifi(void)
{
    reset();
    assert(!agwifi_is_wifi_connected());
    agwifi_print_wifi_network_information();
    assert(agwifi_disconnect_from_wifi_network() == ESP_OK);

    nvs_error = ESP_ERR_NVS_NO_FREE_PAGES;
    assert(agwifi_connect_to_wifi_network() == ESP_ERR_NVS_NO_FREE_PAGES);
    assert(!driver_initialized && groups == 0);
    reset();

    connect_error = ESP_FAIL;
    assert(agwifi_connect_to_wifi_network() == ESP_FAIL);
    assert(!agwifi_is_wifi_connected());
    connect_error = ESP_OK;
    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    reset();

    driver_initialized = true;
    assert(agwifi_connect_to_wifi_network() == ESP_ERR_INVALID_STATE);
    assert(driver_initialized && groups == 0);
    driver_initialized = false;
    reset();

    fail_alloc = true;
    assert(agwifi_connect_to_wifi_network() == ESP_ERR_NO_MEM);
    reset();

    mode_error = ESP_FAIL;
    assert(agwifi_connect_to_wifi_network() == ESP_FAIL);
    assert(groups == 0 && !driver_initialized && destroys == 1);
    reset();

    registration_error = ESP_ERR_NO_MEM;
    assert(agwifi_connect_to_wifi_network() == ESP_ERR_NO_MEM);
    assert(groups == 0 && !wifi_handler && !driver_initialized);
    reset();

    start_error = ESP_FAIL;
    assert(agwifi_connect_to_wifi_network() == ESP_FAIL);
    start_error = ESP_OK;
    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    assert(init_calls == 1);
    reset();

    /* The application may have created the event loop and station interface.
     * La aplicación puede haber creado el bucle de eventos y la interfaz de estación. */
    loop_exists = netif_exists = true;
    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    assert(agwifi_is_wifi_connected());
    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    assert(init_calls == 1 && starts == 1 && connect_calls == 1);
    agwifi_print_wifi_network_information();
    ip_info_error = ESP_FAIL;
    agwifi_print_wifi_network_information();

    /* Losing the link must be visible immediately, before reacquiring IP.
     * La pérdida del enlace debe ser visible de inmediato, antes de volver a obtener una IP. */
    emit(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED);
    assert(!agwifi_is_wifi_connected());
    int calls = connect_calls;
    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    /* Wait for the in-flight reconnection.
     * Espera a que termine la reconexión en curso. */
    assert(connect_calls == calls);
    emit(IP_EVENT, IP_EVENT_STA_LOST_IP);
    assert(!agwifi_is_wifi_connected());
    emit(IP_EVENT, IP_EVENT_STA_GOT_IP);
    assert(agwifi_is_wifi_connected());

    assert(agwifi_disconnect_from_wifi_network() == ESP_OK);
    assert(!agwifi_is_wifi_connected());
    calls = connect_calls;
    emit(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED);
    emit(IP_EVENT, IP_EVENT_STA_GOT_IP);
    assert(connect_calls == calls && !agwifi_is_wifi_connected());
    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    assert(init_calls == 1 && starts == 2);
    assert(agwifi_deinitialize() == ESP_OK);
    /* Borrowed interface survives.
     * La interfaz prestada se conserva. */
    assert(netif_exists && destroys == 0);
    netif_exists = false;
    reset();

    reject_connection = true;
    assert(agwifi_connect_to_wifi_network() == ESP_FAIL);
    assert(connect_calls == CONFIG_ESP_MAXIMUM_RETRY + 1);
    assert(!agwifi_is_wifi_connected());
    reject_connection = false;
    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    reset();

    drop_ip = true;
    assert(agwifi_connect_to_wifi_network() == ESP_ERR_TIMEOUT);
    assert(!agwifi_is_wifi_connected());
    emit(IP_EVENT, IP_EVENT_STA_GOT_IP);
    assert(!agwifi_is_wifi_connected());
    drop_ip = false;
    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    stop_error = ESP_FAIL;
    assert(agwifi_deinitialize() == ESP_FAIL);
    assert(groups == 1 && driver_initialized);
    assert(agwifi_connect_to_wifi_network() == ESP_ERR_INVALID_STATE);
    reset();
    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    reset();

    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    deinit_error = ESP_FAIL;
    assert(agwifi_deinitialize() == ESP_FAIL);
    assert(groups == 1 && driver_initialized);
    reset();

    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    unregister_error = ESP_FAIL;
    assert(agwifi_deinitialize() == ESP_FAIL);
    assert(groups == 1 && !driver_initialized);
    assert(agwifi_connect_to_wifi_network() == ESP_ERR_INVALID_STATE);
    reset();

    assert(agwifi_connect_to_wifi_network() == ESP_OK);
    drop_stop = true;
    assert(agwifi_disconnect_from_wifi_network() == ESP_ERR_TIMEOUT);
    assert(!agwifi_is_wifi_connected());
    assert(groups == 1 && driver_initialized);
    reset();
    puts("Wi-Fi lifecycle, state, retries, timeout, ownership and error tests passed");
}

static void test_http(void)
{
    assert(agHttpServer_get_http_server() == NULL);
    assert(agHttpServer_stop_http_server() == ESP_OK && http_stops == 0);
    http_start_error = ESP_FAIL;
    assert(agHttpServer_initialize_http_server() == ESP_FAIL);
    assert(agHttpServer_get_http_server() == NULL);
    http_start_error = ESP_OK;
    assert(agHttpServer_initialize_http_server() == ESP_OK);
    assert(agHttpServer_get_http_server() != NULL);
    assert(agHttpServer_initialize_http_server() == ESP_OK && http_starts == 2);
    http_stop_error = ESP_FAIL;
    assert(agHttpServer_stop_http_server() == ESP_FAIL);
    assert(agHttpServer_get_http_server() != NULL);
    http_stop_error = ESP_OK;
    assert(agHttpServer_stop_http_server() == ESP_OK);
    assert(agHttpServer_get_http_server() == NULL);
    assert(agHttpServer_stop_http_server() == ESP_OK && http_stops == 2);
    assert(agHttpServer_initialize_http_server() == ESP_OK);
    assert(agHttpServer_stop_http_server() == ESP_OK);
    puts("HTTP start/stop, repeated calls and error recovery tests passed");
}

int main(void)
{
    if (CONFIG_ESP_WIFI_SSID[0] == '\0') {
        assert(agwifi_connect_to_wifi_network() == ESP_ERR_INVALID_ARG);
        assert(!driver_initialized && groups == 0);
        puts("Empty SSID rejected without allocating resources");
        return 0;
    }
    test_wifi();
    test_http();
    return 0;
}
