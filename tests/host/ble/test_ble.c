#include "fake_nimble.h"
#include "agBLE.h"
#include <assert.h>
#include <stdio.h>

struct fake_hs_cfg ble_hs_cfg;
static int app_task, host_task, semaphore;
static TaskHandle_t current_task = &app_task;
static int allocations, tasks, init_calls, deinit_calls, stop_calls, advertisements, notifications;
static bool allocation_failure, task_failure, wait_timeout;
static int port_init_error, port_deinit_error, port_stop_error, gatt_error, connect_error, terminate_error;
static bool exit_ready;
static gap_cb_t gap_callback;
static svc_cb_t service_callback;
static chr_cb_t characteristic_callback;
static dsc_cb_t descriptor_callback;
static attr_cb_t attribute_callback;
static void *attribute_arg;
static const struct ble_gatt_svc_def *server_services;
static agble_event_t last_event;
static agble_server_event_t last_server_event;
static uint8_t last_data[600];
static int event_count, server_event_count;
static bool stop_in_callback;

SemaphoreHandle_t xSemaphoreCreateBinary(void)
{ if (allocation_failure) return NULL; allocations++; exit_ready = false; return &semaphore; }
void vSemaphoreDelete(SemaphoreHandle_t sem)
{ assert(sem == &semaphore && !tasks); allocations--; }
BaseType_t xSemaphoreGive(SemaphoreHandle_t sem)
{ assert(sem == &semaphore); exit_ready = true; return pdTRUE; }
BaseType_t xSemaphoreTake(SemaphoreHandle_t sem, TickType_t wait)
{
    assert(sem == &semaphore && wait == CONFIG_AGBLE_STOP_TIMEOUT_MS);
    if (wait_timeout) return pdFALSE;
    assert(exit_ready);
    exit_ready = false;
    return pdTRUE;
}
BaseType_t xTaskCreatePinnedToCore(void (*fn)(void *), const char *name, uint32_t stack,
    void *arg, unsigned priority, TaskHandle_t *handle, int core)
{
    assert(fn && name && stack == 4096 && !arg && priority == 21 && core == 0);
    if (task_failure) return pdFALSE;
    *handle = &host_task;
    tasks++;
    return pdPASS;
}
TaskHandle_t xTaskGetCurrentTaskHandle(void) { return current_task; }
void vTaskDelete(TaskHandle_t task) { assert(task == &host_task); tasks--; }
void vTaskSuspend(TaskHandle_t task) { (void)task; }
esp_err_t nimble_port_init(void) { init_calls++; return port_init_error; }
esp_err_t nimble_port_deinit(void) { deinit_calls++; assert(!tasks); return port_deinit_error; }
void nimble_port_run(void) {}
int nimble_port_stop(void)
{
    assert(current_task == &app_task);
    stop_calls++;
    if (port_stop_error) return port_stop_error;
    /* SDK stop can generate disconnect events; they must not restart advertising.
     * La parada del SDK puede generar desconexiones; no deben reiniciar los anuncios. */
    if (gap_callback) {
        struct ble_gap_event event = { .type = BLE_GAP_EVENT_DISCONNECT };
        gap_callback(&event, NULL);
    }
    exit_ready = true;
    return 0;
}
int ble_hs_util_ensure_addr(int prefer_random) { assert(!prefer_random); return 0; }
int ble_hs_id_infer_auto(int privacy, uint8_t *type) { assert(!privacy); *type = 0; return 0; }
int ble_uuid_cmp(const ble_uuid_t *a, const ble_uuid_t *b)
{
    if (a->type != b->type) return 1;
    if (a->type == BLE_UUID_TYPE_16) return ((const ble_uuid16_t *)a)->value != ((const ble_uuid16_t *)b)->value;
    return memcmp(((const ble_uuid128_t *)a)->value, ((const ble_uuid128_t *)b)->value, 16);
}
uint16_t ble_uuid_u16(const ble_uuid_t *uuid)
{ return uuid->type == BLE_UUID_TYPE_16 ? ((const ble_uuid16_t *)uuid)->value : 0; }
int os_mbuf_copydata(const struct os_mbuf *om, int off, int length, void *data)
{ assert(off >= 0 && length >= 0 && off + length <= om->length); memcpy(data, om->data + off, length); return 0; }
int os_mbuf_append(struct os_mbuf *om, const void *data, uint16_t length)
{ assert(om->length + length <= sizeof(om->data)); memcpy(om->data + om->length, data, length); om->length += length; return 0; }
int ble_gap_disc(uint8_t type, int32_t duration, const struct ble_gap_disc_params *params, gap_cb_t cb, void *arg)
{ assert(!type && duration > 0 && params->passive && !params->filter_duplicates && !arg); gap_callback = cb; return 0; }
int ble_gap_disc_cancel(void) { return 0; }
int ble_gap_connect(uint8_t type, const ble_addr_t *peer, int32_t timeout, const void *params, gap_cb_t cb, void *arg)
{ assert(!type && peer && timeout > 0 && !params && !arg); gap_callback = cb; return connect_error; }
int ble_gap_conn_cancel(void) { return 0; }
int ble_gap_terminate(uint16_t conn, uint8_t reason) { assert(conn == 7 && reason == BLE_ERR_REM_USER_CONN_TERM); return terminate_error; }
int ble_gattc_exchange_mtu(uint16_t conn, mtu_cb_t cb, void *arg)
{ assert(conn == 7 && cb && !arg); return 1; /* Rejected exchange uses default MTU. / Intercambio rechazado: MTU predeterminado. */ }
int ble_gattc_disc_svc_by_uuid(uint16_t conn, const ble_uuid_t *uuid, svc_cb_t cb, void *arg)
{ assert(conn == 7 && ble_uuid_u16(uuid) == 0xfff0 && !arg); service_callback = cb; return 0; }
int ble_gattc_disc_all_chrs(uint16_t conn, uint16_t start, uint16_t end, chr_cb_t cb, void *arg)
{ assert(conn == 7 && start == 1 && end == 20 && !arg); characteristic_callback = cb; return 0; }
int ble_gattc_disc_all_dscs(uint16_t conn, uint16_t start, uint16_t end, dsc_cb_t cb, void *arg)
{ assert(conn == 7 && start == 5 && end == 20 && !arg); descriptor_callback = cb; return 0; }
int ble_gattc_write_flat(uint16_t conn, uint16_t attr, const void *data, uint16_t length, attr_cb_t cb, void *arg)
{
    assert(conn == 7 && data && length);
    if (attr == 6) assert(length == 2 && ((const uint8_t *)data)[0] == 1);
    else assert(attr == 3);
    attribute_callback = cb;
    attribute_arg = arg;
    return 0;
}
int ble_gattc_read(uint16_t conn, uint16_t attr, attr_cb_t cb, void *arg)
{ assert(conn == 7 && attr == 5); attribute_callback = cb; attribute_arg = arg; return 0; }
uint16_t ble_att_mtu(uint16_t conn) { assert(conn == 7); return 23; }
int ble_gatts_count_cfg(const struct ble_gatt_svc_def *services)
{ assert(ble_uuid_u16(services[0].uuid) == AGBLE_SERVER_SERVICE_UUID); return gatt_error; }
int ble_gatts_add_svcs(const struct ble_gatt_svc_def *services)
{ server_services = services; *services[0].characteristics[1].val_handle = 42; return 0; }
void ble_gatts_chr_updated(uint16_t attr) { assert(attr == 42); notifications++; }
void ble_svc_gap_init(void) {}
int ble_svc_gap_device_name_set(const char *name) { assert(strlen(name) <= 20); return 0; }
void ble_svc_gatt_init(void) {}
int ble_gap_adv_set_fields(const struct ble_hs_adv_fields *fields)
{ assert(fields->name_len <= 20 && fields->uuids16->value == 0xfff0); return 0; }
int ble_gap_adv_start(uint8_t type, const ble_addr_t *direct, int32_t duration,
                      const struct ble_gap_adv_params *params, gap_cb_t cb, void *arg)
{ assert(!type && !direct && duration == BLE_HS_FOREVER && params && !arg); gap_callback = cb; advertisements++; return 0; }

static void on_client(const agble_event_t *event, void *context)
{
    assert(context == &event_count);
    last_event = *event;
    if (event->length) memcpy(last_data, event->data, event->length);
    event_count++;
    if (stop_in_callback) assert(agble_deinit() == ESP_ERR_INVALID_STATE);
}
static void on_server(const agble_server_event_t *event, void *context)
{
    assert(context == &server_event_count);
    last_server_event = *event;
    if (event->length) memcpy(last_data, event->data, event->length);
    server_event_count++;
    if (stop_in_callback) assert(agble_server_stop() == ESP_ERR_INVALID_STATE);
}
static agble_config_t config = {
    .service = { .length = 2, .bytes = {0xf0, 0xff} },
    .write_char = { .length = 2, .bytes = {0xf1, 0xff} },
    .receive_char = { .length = 2, .bytes = {0xf2, 0xff} },
    .callback = on_client, .context = &event_count,
};
static void sync_host(void)
{
    current_task = &host_task;
    assert(ble_hs_cfg.sync_cb);
    ble_hs_cfg.sync_cb();
    current_task = &app_task;
}
static void emit_gap(struct ble_gap_event *event)
{
    current_task = &host_task;
    assert(gap_callback);
    gap_callback(event, NULL);
    current_task = &app_task;
}
static void disconnected(void)
{
    struct ble_gap_event event = { .type = BLE_GAP_EVENT_DISCONNECT,
        .disconnect = { .conn = { .conn_handle = 7 }, .reason = 19 } };
    emit_gap(&event);
}
static void assert_clean(void)
{
    assert(!allocations && !tasks && !ble_hs_cfg.sync_cb && !ble_hs_cfg.reset_cb);
    gap_callback = NULL;
}
static void connect_client(bool notify)
{
    agble_address_t address = {0};
    assert(agble_connect(&address, 1000) == ESP_OK);
    struct ble_gap_event event = { .type = BLE_GAP_EVENT_CONNECT, .connect = { .conn_handle = 7 } };
    emit_gap(&event);
    assert(last_event.type == AGBLE_CONNECTED);
    assert(agble_read() == ESP_ERR_INVALID_STATE);
    const struct ble_gatt_error ok = {0}, done = { .status = BLE_HS_EDONE };
    const struct ble_gatt_svc service = { .start_handle = 1, .end_handle = 20 };
    service_callback(7, &ok, &service, NULL);
    service_callback(7, &done, NULL, NULL);
    struct ble_gatt_chr chr = { .def_handle = 2, .val_handle = 3, .properties = BLE_GATT_CHR_PROP_WRITE,
        .uuid.u16 = BLE_UUID16_INIT(0xfff1) };
    characteristic_callback(7, &ok, &chr, NULL);
    chr.def_handle = 4;
    chr.val_handle = 5;
    chr.properties = BLE_GATT_CHR_PROP_READ | (notify ? BLE_GATT_CHR_PROP_NOTIFY : 0);
    chr.uuid.u16.value = 0xfff2;
    characteristic_callback(7, &ok, &chr, NULL);
    characteristic_callback(7, &done, NULL, NULL);
    if (notify) {
        const struct ble_gatt_dsc descriptor = { .handle = 6, .uuid.u16 = BLE_UUID16_INIT(0x2902) };
        descriptor_callback(7, &ok, 5, &descriptor, NULL);
        descriptor_callback(7, &done, 5, NULL, NULL);
        attribute_callback(7, &ok, NULL, attribute_arg);
    }
    assert(last_event.type == AGBLE_READY);
}

static void test_client(void)
{
    assert(agble_scan_start(1) == ESP_ERR_INVALID_STATE);
    assert(agble_write(NULL, 1) == ESP_ERR_INVALID_ARG);
    assert(agble_init(NULL) == ESP_ERR_INVALID_ARG);
    agble_config_t invalid = config;
    invalid.service.length = 3;
    assert(agble_init(&invalid) == ESP_ERR_INVALID_ARG);
    allocation_failure = true;
    assert(agble_init(&config) == ESP_ERR_NO_MEM);
    allocation_failure = false;
    port_init_error = ESP_FAIL;
    assert(agble_init(&config) == ESP_FAIL);
    port_init_error = 0;
    task_failure = true;
    assert(agble_init(&config) == ESP_ERR_NO_MEM);
    assert_clean();
    task_failure = false;
    assert(agble_init(&config) == ESP_OK);
    assert(agble_init(&config) == ESP_ERR_INVALID_STATE);
    assert(agble_server_start("test", on_server, &server_event_count) == ESP_ERR_INVALID_STATE);
    assert(agble_scan_start(10) == ESP_ERR_INVALID_STATE);
    stop_in_callback = true;
    sync_host();
    assert(last_event.type == AGBLE_STACK_READY);
    assert(agble_scan_start(UINT32_MAX) == ESP_ERR_INVALID_ARG);
    assert(agble_scan_start(0) == ESP_OK);
    assert(agble_deinit() == ESP_ERR_INVALID_STATE);
    const uint8_t data[] = {2, 1, 6};
    struct ble_gap_event report = { .type = BLE_GAP_EVENT_DISC,
        .disc = { .addr = { .type = 1, .val = {1, 2, 3, 4, 5, 6} }, .rssi = -40,
                  .data = data, .length_data = sizeof(data) } };
    emit_gap(&report);
    assert(last_event.type == AGBLE_SCAN_RESULT && last_event.address.bytes[0] == 1);
    assert(last_event.length == sizeof(data) && !memcmp(last_data, data, sizeof(data)));
    assert(agble_scan_stop() == ESP_OK);
    assert(last_event.type == AGBLE_SCAN_DONE);
    stop_in_callback = false;
    agble_address_t address = {0};
    assert(agble_connect(&address, 0) == ESP_ERR_INVALID_ARG);
    connect_error = 1;
    assert(agble_connect(&address, 1000) == ESP_FAIL);
    connect_error = 0;
    connect_client(true);
    assert(agble_deinit() == ESP_ERR_INVALID_STATE);
    char large[21] = {0};
    assert(agble_write(large, sizeof(large)) == ESP_ERR_INVALID_SIZE);
    assert(agble_write("1", 1) == ESP_OK);
    assert(agble_read() == ESP_ERR_INVALID_STATE);
    const struct ble_gatt_error ok = {0};
    attribute_callback(7, &ok, NULL, attribute_arg);
    assert(last_event.type == AGBLE_WRITE_DONE);
    assert(agble_read() == ESP_OK);
    struct os_mbuf packet = { .data = {'O', 'K'}, .length = 2 };
    struct ble_gatt_attr attr = { .om = &packet };
    attribute_callback(7, &ok, &attr, attribute_arg);
    assert(last_event.type == AGBLE_DATA && last_event.from_read && !memcmp(last_data, "OK", 2));
    struct ble_gap_event update = { .type = BLE_GAP_EVENT_NOTIFY_RX,
        .notify_rx = { .conn_handle = 7, .attr_handle = 5, .om = &packet } };
    emit_gap(&update);
    assert(last_event.type == AGBLE_DATA && !last_event.from_read);
    assert(agble_read() == ESP_OK);
    attr_cb_t late_callback = attribute_callback;
    void *late_arg = attribute_arg;
    assert(agble_disconnect() == ESP_OK);
    disconnected();
    int before = event_count;
    late_callback(7, &ok, &attr, late_arg);
    assert(event_count == before && last_event.type == AGBLE_DISCONNECTED);
    connect_client(false);
    disconnected();
    assert(agble_connect(&address, 1000) == ESP_OK);
    struct ble_gap_event connected = { .type = BLE_GAP_EVENT_CONNECT, .connect = { .conn_handle = 7 } };
    emit_gap(&connected);
    const struct ble_gatt_error done = { .status = BLE_HS_EDONE };
    service_callback(7, &done, NULL, NULL);
    assert(last_event.type == AGBLE_ERROR && last_event.phase == AGBLE_PHASE_DISCOVERY);
    disconnected();
    assert(agble_deinit() == ESP_OK);
    assert(agble_deinit() == ESP_OK);
    assert_clean();
    puts("BLE central: validation, ownership, discovery, subscription, I/O and late events passed");
}

static void test_server(void)
{
    assert(agble_server_publish("0", 1) == ESP_ERR_INVALID_STATE);
    assert(agble_server_start("", on_server, &server_event_count) == ESP_ERR_INVALID_ARG);
    assert(agble_server_start("012345678901234567890", on_server, &server_event_count) == ESP_ERR_INVALID_ARG);
    gatt_error = 1;
    assert(agble_server_start("test", on_server, &server_event_count) == ESP_FAIL);
    gatt_error = 0;
    assert_clean();
    assert(agble_server_start("test", on_server, &server_event_count) == ESP_OK);
    assert(agble_init(&config) == ESP_ERR_INVALID_STATE);
    assert(agble_server_publish("0", 1) == ESP_OK);
    assert(!notifications);
    stop_in_callback = true;
    sync_host();
    assert(last_server_event.type == AGBLE_SERVER_ADVERTISING);
    struct os_mbuf packet = {0};
    struct ble_gatt_access_ctxt access = { .op = BLE_GATT_ACCESS_OP_READ_CHR, .om = &packet };
    const struct ble_gatt_chr_def *chars = server_services[0].characteristics;
    assert(chars[1].access_cb(7, 42, &access, NULL) == 0);
    assert(packet.length == 1 && packet.data[0] == '0');
    access.op = BLE_GATT_ACCESS_OP_WRITE_CHR;
    packet.length = 0;
    assert(chars[0].access_cb(7, 41, &access, NULL) == BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN);
    packet.length = 513;
    assert(chars[0].access_cb(7, 41, &access, NULL) == BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN);
    packet.length = 1;
    packet.data[0] = '1';
    current_task = &host_task;
    assert(chars[0].access_cb(7, 41, &access, NULL) == 0);
    current_task = &app_task;
    assert(last_server_event.type == AGBLE_SERVER_COMMAND && last_data[0] == '1');
    assert(agble_server_publish("012345678901234567890", 21) == ESP_ERR_INVALID_ARG);
    assert(agble_server_publish("1", 1) == ESP_OK && notifications == 1);
    stop_in_callback = false;
    int ads = advertisements;
    disconnected();
    assert(advertisements == ads + 1);
    port_stop_error = 1;
    assert(agble_server_stop() == ESP_FAIL);
    assert(agble_server_publish("1", 1) == ESP_ERR_INVALID_STATE);
    port_stop_error = 0;
    wait_timeout = true;
    assert(agble_server_stop() == ESP_ERR_TIMEOUT);
    int stops = stop_calls;
    ads = advertisements;
    wait_timeout = false;
    assert(agble_server_stop() == ESP_OK);
    assert(stop_calls == stops && advertisements == ads);
    assert(agble_server_stop() == ESP_OK);
    assert_clean();
    assert(agble_server_start("test", on_server, &server_event_count) == ESP_OK);
    assert(agble_server_stop() == ESP_OK);
    assert_clean();
    puts("BLE server: commands, status, limits, restart, shutdown errors and task-exit timeout passed");
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        bool client = strstr(argv[1], "client") != NULL;
        bool startup = strstr(argv[1], "startup") != NULL;
        if (startup) task_failure = true;
        if (startup) port_deinit_error = ESP_FAIL;
        esp_err_t err = client ? agble_init(&config) : agble_server_start("test", on_server, &server_event_count);
        assert(err == (startup ? ESP_FAIL : ESP_OK));
        port_deinit_error = ESP_FAIL;
        assert((client ? agble_deinit() : agble_server_stop()) == ESP_FAIL);
        int calls = deinit_calls;
        assert((client ? agble_deinit() : agble_server_stop()) == ESP_FAIL);
        assert(calls == deinit_calls && allocations == 1);
        task_failure = false;
        assert(agble_init(&config) == ESP_ERR_INVALID_STATE);
        assert(agble_server_start("test", on_server, &server_event_count) == ESP_ERR_INVALID_STATE);
        puts("BLE partial SDK cleanup: ownership retained and unsafe repeated deinit prevented");
        return 0;
    }
    test_client();
    test_server();
    return 0;
}
