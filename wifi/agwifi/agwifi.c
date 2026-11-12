#include "agwifi.h"

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_STOPPED_BIT BIT3

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define AGWIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define AGWIFI_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define AGWIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define AGWIFI_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#else
#define AGWIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define AGWIFI_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define AGWIFI_AUTH_MODE WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define AGWIFI_AUTH_MODE WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define AGWIFI_AUTH_MODE WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define AGWIFI_AUTH_MODE WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define AGWIFI_AUTH_MODE WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define AGWIFI_AUTH_MODE WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define AGWIFI_AUTH_MODE WIFI_AUTH_WAPI_PSK
#else
#define AGWIFI_AUTH_MODE WIFI_AUTH_WPA2_PSK
#endif

static const char *TAG = "agwifi";
static EventGroupHandle_t s_events;
static esp_netif_t *s_netif;
static esp_event_handler_instance_t s_wifi_handler;
static esp_event_handler_instance_t s_ip_handler;
static bool s_owns_netif;
static bool s_driver_initialized;
static bool s_started;
static bool s_ready;
static unsigned int s_retry_num;

_Static_assert(sizeof(CONFIG_ESP_WIFI_SSID) - 1 <= sizeof(((wifi_config_t *)0)->sta.ssid),
               "Wi-Fi SSID exceeds driver capacity");
_Static_assert(sizeof(CONFIG_ESP_WIFI_PASSWORD) - 1 <= sizeof(((wifi_config_t *)0)->sta.password),
               "Wi-Fi password exceeds driver capacity");
_Static_assert(sizeof(AGWIFI_H2E_IDENTIFIER) <= sizeof(((wifi_config_t *)0)->sta.sae_h2e_identifier),
               "SAE identifier exceeds driver capacity");

/* Event bits synchronize the application task with the default event loop.
 * Los bits de evento sincronizan la tarea de la aplicación con el bucle de eventos predeterminado. */
#define WIFI_RECONNECT_BIT BIT2

static void connect_station(void)
{
    esp_err_t err = esp_wifi_connect();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start connection: %s", esp_err_to_name(err));
        xEventGroupClearBits(s_events, WIFI_RECONNECT_BIT);
        xEventGroupSetBits(s_events, WIFI_FAIL_BIT);
    }
}

static void event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    (void)data;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        s_retry_num = 0;
        if (xEventGroupGetBits(s_events) & WIFI_RECONNECT_BIT) {
            connect_station();
        }
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(s_events, WIFI_CONNECTED_BIT);
        if (xEventGroupGetBits(s_events) & WIFI_RECONNECT_BIT) {
            if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
                s_retry_num++;
                connect_station();
            } else {
                xEventGroupClearBits(s_events, WIFI_RECONNECT_BIT);
                xEventGroupSetBits(s_events, WIFI_FAIL_BIT);
            }
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        if (xEventGroupGetBits(s_events) & WIFI_RECONNECT_BIT) {
            s_retry_num = 0;
            xEventGroupClearBits(s_events, WIFI_FAIL_BIT);
            xEventGroupSetBits(s_events, WIFI_CONNECTED_BIT);
            ESP_LOGI(TAG, "Wi-Fi connected");
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_LOST_IP) {
        xEventGroupClearBits(s_events, WIFI_CONNECTED_BIT);
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_STOP) {
        xEventGroupClearBits(s_events, WIFI_CONNECTED_BIT | WIFI_RECONNECT_BIT);
        xEventGroupSetBits(s_events, WIFI_STOPPED_BIT);
    }
}

static esp_err_t stop_station(void)
{
    xEventGroupClearBits(s_events, WIFI_RECONNECT_BIT | WIFI_CONNECTED_BIT);
    if (!s_started) {
        return ESP_OK;
    }
    if (!(xEventGroupGetBits(s_events) & WIFI_STOPPED_BIT)) {
        esp_err_t err = esp_wifi_stop();
        if (err != ESP_OK) {
            return err;
        }
    }
    /* Wait until the default netif handlers and our callback have processed
     * STOP. This also drains earlier connection events before a new attempt.
     * Espera hasta que los manejadores predeterminados de netif y nuestro callback
     * hayan procesado STOP. Esto también procesa los eventos de conexión pendientes
     * antes de un nuevo intento. */
    EventBits_t bits = xEventGroupWaitBits(s_events, WIFI_STOPPED_BIT,
        pdFALSE, pdFALSE, pdMS_TO_TICKS(CONFIG_AGWIFI_CONNECT_TIMEOUT_MS));
    if (!(bits & WIFI_STOPPED_BIT)) {
        return ESP_ERR_TIMEOUT;
    }
    s_started = false;
    return ESP_OK;
}

esp_err_t agwifi_deinitialize(void)
{
    if (!s_events) {
        return ESP_OK;
    }
    s_ready = false;
    esp_err_t stop_err = stop_station();
    if (stop_err != ESP_OK) {
        return stop_err;
    }
    if (s_driver_initialized) {
        esp_err_t err = esp_wifi_deinit();
        if (err != ESP_OK) {
            return err;
        }
        s_driver_initialized = false;
    }
    /* Unregister before deleting the event group used by callbacks.
     * Elimina el registro de los manejadores antes de borrar el grupo de eventos usado por los callbacks. */
    if (s_wifi_handler) {
        esp_err_t err = esp_event_handler_instance_unregister(
            WIFI_EVENT, ESP_EVENT_ANY_ID, s_wifi_handler);
        if (err != ESP_OK) {
            return err;
        }
        s_wifi_handler = NULL;
    }
    if (s_ip_handler) {
        esp_err_t err = esp_event_handler_instance_unregister(
            IP_EVENT, ESP_EVENT_ANY_ID, s_ip_handler);
        if (err != ESP_OK) {
            return err;
        }
        s_ip_handler = NULL;
    }
    if (s_owns_netif) {
        esp_netif_destroy_default_wifi(s_netif);
    }
    s_netif = NULL;
    s_owns_netif = false;
    vEventGroupDelete(s_events);
    s_events = NULL;
    s_retry_num = 0;
    /* NVS, esp-netif and the default event loop are process-wide resources.
     * NVS, esp-netif y el bucle de eventos predeterminado son recursos compartidos por todo el proceso. */
    return ESP_OK;
}

static esp_err_t initialize(void)
{
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        return err;
    }
    err = esp_netif_init();
    if (err != ESP_OK) {
        return err;
    }
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }
    wifi_mode_t mode;
    err = esp_wifi_get_mode(&mode);
    if (err != ESP_ERR_WIFI_NOT_INIT) {
        return err == ESP_OK ? ESP_ERR_INVALID_STATE : err;
    }
    s_events = xEventGroupCreate();
    if (!s_events) {
        return ESP_ERR_NO_MEM;
    }
    s_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!s_netif) {
        s_netif = esp_netif_create_default_wifi_sta();
        s_owns_netif = s_netif != NULL;
    }
    if (!s_netif) {
        err = ESP_ERR_NO_MEM;
        goto fail;
    }
    wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&init);
    if (err != ESP_OK) {
        goto fail;
    }
    s_driver_initialized = true;
    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                              event_handler, NULL, &s_wifi_handler);
    if (err != ESP_OK) {
        goto fail;
    }
    err = esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID,
                                              event_handler, NULL, &s_ip_handler);
    if (err != ESP_OK) {
        goto fail;
    }
    wifi_config_t config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .threshold.authmode = AGWIFI_AUTH_MODE,
            .sae_pwe_h2e = AGWIFI_SAE_MODE,
            .sae_h2e_identifier = AGWIFI_H2E_IDENTIFIER,
        },
    };
    err = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (err == ESP_OK) {
        err = esp_wifi_set_mode(WIFI_MODE_STA);
    }
    if (err == ESP_OK) {
        err = esp_wifi_set_config(WIFI_IF_STA, &config);
    }
    if (err == ESP_OK) {
        s_ready = true;
        return ESP_OK;
    }
fail:
    {
        esp_err_t cleanup = agwifi_deinitialize();
        if (cleanup != ESP_OK) {
            ESP_LOGE(TAG, "Cleanup failed: %s", esp_err_to_name(cleanup));
        }
    }
    return err;
}

esp_err_t agwifi_connect_to_wifi_network(void)
{
    if (CONFIG_ESP_WIFI_SSID[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }
    if (agwifi_is_wifi_connected()) {
        return ESP_OK;
    }
    if (!s_events) {
        esp_err_t err = initialize();
        if (err != ESP_OK) {
            return err;
        }
    }
    if (!s_ready) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!(xEventGroupGetBits(s_events) & WIFI_RECONNECT_BIT)) {
        esp_err_t err = stop_station();
        if (err != ESP_OK) {
            return err;
        }
        xEventGroupClearBits(s_events, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT | WIFI_STOPPED_BIT);
        xEventGroupSetBits(s_events, WIFI_RECONNECT_BIT);
        err = esp_wifi_start();
        if (err != ESP_OK) {
            xEventGroupClearBits(s_events, WIFI_RECONNECT_BIT);
            return err;
        }
        s_started = true;
    }
    EventBits_t bits = xEventGroupWaitBits(s_events,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE,
        pdMS_TO_TICKS(CONFIG_AGWIFI_CONNECT_TIMEOUT_MS));
    if (bits & WIFI_CONNECTED_BIT) {
        return ESP_OK;
    }
    esp_err_t stop_err = stop_station();
    if (stop_err != ESP_OK) {
        return stop_err;
    }
    return (bits & WIFI_FAIL_BIT) ? ESP_FAIL : ESP_ERR_TIMEOUT;
}

esp_err_t agwifi_disconnect_from_wifi_network(void)
{
    if (!s_events) {
        return ESP_OK;
    }
    return stop_station();
}

bool agwifi_is_wifi_connected(void)
{
    return s_events && (xEventGroupGetBits(s_events) & WIFI_CONNECTED_BIT) != 0;
}

void agwifi_print_wifi_network_information(void)
{
    esp_netif_ip_info_t info;
    if (!agwifi_is_wifi_connected() || !s_netif) {
        ESP_LOGW(TAG, "Wi-Fi is not connected");
        return;
    }
    esp_err_t err = esp_netif_get_ip_info(s_netif, &info);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not read network information: %s", esp_err_to_name(err));
        return;
    }
    printf("IP: " IPSTR "\n", IP2STR(&info.ip));
    printf("Gateway: " IPSTR "\n", IP2STR(&info.gw));
    printf("Mask: " IPSTR "\n", IP2STR(&info.netmask));
}
