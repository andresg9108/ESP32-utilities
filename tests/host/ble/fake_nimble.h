#ifndef FAKE_NIMBLE_H
#define FAKE_NIMBLE_H

#include "fake_idf.h"
#include <string.h>

#ifndef CONFIG_AGBLE_CLIENT_ENABLED
#define CONFIG_AGBLE_CLIENT_ENABLED 1
#endif
#ifndef CONFIG_AGBLE_SERVER_ENABLED
#define CONFIG_AGBLE_SERVER_ENABLED 1
#endif
#define CONFIG_AGBLE_STOP_TIMEOUT_MS 5000
#define CONFIG_BT_NIMBLE_HOST_TASK_STACK_SIZE 4096
#define CONFIG_BT_NIMBLE_PINNED_TO_CORE 0
#define CONFIG_BT_NIMBLE_GAP_SERVICE 1
#define configMAX_PRIORITIES 25
#define pdTRUE 1
#define pdPASS 1
typedef int BaseType_t;
typedef int portMUX_TYPE;
typedef void *TaskHandle_t;
typedef void *SemaphoreHandle_t;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(lock) ((void)(lock))
#define portEXIT_CRITICAL(lock) ((void)(lock))
BaseType_t xTaskCreatePinnedToCore(void (*fn)(void *), const char *name, uint32_t stack,
    void *arg, unsigned priority, TaskHandle_t *handle, int core);
TaskHandle_t xTaskGetCurrentTaskHandle(void);
void vTaskDelete(TaskHandle_t task);
void vTaskSuspend(TaskHandle_t task);
SemaphoreHandle_t xSemaphoreCreateBinary(void);
void vSemaphoreDelete(SemaphoreHandle_t semaphore);
BaseType_t xSemaphoreGive(SemaphoreHandle_t semaphore);
BaseType_t xSemaphoreTake(SemaphoreHandle_t semaphore, TickType_t wait);
esp_err_t nimble_port_init(void);
esp_err_t nimble_port_deinit(void);
int nimble_port_stop(void);
void nimble_port_run(void);

enum {
    BLE_HS_ENOTCONN = 1, BLE_HS_EDONE, BLE_HS_ENOENT, BLE_HS_EMSGSIZE, BLE_HS_EALREADY,
    BLE_ERR_REM_USER_CONN_TERM,
    BLE_UUID_TYPE_16 = 16, BLE_UUID_TYPE_128 = 128,
    BLE_GATT_DSC_CLT_CFG_UUID16 = 0x2902,
};
#define BLE_HS_FOREVER INT32_MAX
typedef struct { uint8_t type; } ble_uuid_t;
typedef struct { ble_uuid_t u; uint16_t value; } ble_uuid16_t;
typedef struct { ble_uuid_t u; uint8_t value[16]; } ble_uuid128_t;
typedef union { ble_uuid_t u; ble_uuid16_t u16; ble_uuid128_t u128; } ble_uuid_any_t;
#define BLE_UUID16_INIT(value_) { .u = { .type = BLE_UUID_TYPE_16 }, .value = value_ }
#define BLE_UUID16_DECLARE(value_) (&((const ble_uuid16_t)BLE_UUID16_INIT(value_)).u)
int ble_uuid_cmp(const ble_uuid_t *a, const ble_uuid_t *b);
uint16_t ble_uuid_u16(const ble_uuid_t *uuid);
typedef struct { uint8_t type; uint8_t val[6]; } ble_addr_t;

struct os_mbuf { uint8_t data[600]; uint16_t length; };
#define OS_MBUF_PKTLEN(om) ((om)->length)
int os_mbuf_copydata(const struct os_mbuf *om, int off, int length, void *data);
int os_mbuf_append(struct os_mbuf *om, const void *data, uint16_t length);
struct ble_gatt_error { int status; };
struct ble_gatt_attr { struct os_mbuf *om; };
struct ble_gatt_svc { uint16_t start_handle, end_handle; };
struct ble_gatt_chr { uint16_t def_handle, val_handle; uint8_t properties; ble_uuid_any_t uuid; };
struct ble_gatt_dsc { uint16_t handle; ble_uuid_any_t uuid; };
typedef int (*attr_cb_t)(uint16_t, const struct ble_gatt_error *, struct ble_gatt_attr *, void *);
typedef int (*svc_cb_t)(uint16_t, const struct ble_gatt_error *, const struct ble_gatt_svc *, void *);
typedef int (*chr_cb_t)(uint16_t, const struct ble_gatt_error *, const struct ble_gatt_chr *, void *);
typedef int (*dsc_cb_t)(uint16_t, const struct ble_gatt_error *, uint16_t, const struct ble_gatt_dsc *, void *);
typedef int (*mtu_cb_t)(uint16_t, const struct ble_gatt_error *, uint16_t, void *);
#define BLE_GATT_CHR_PROP_WRITE 0x08
#define BLE_GATT_CHR_PROP_READ 0x02
#define BLE_GATT_CHR_PROP_NOTIFY 0x10
#define BLE_GATT_CHR_PROP_INDICATE 0x20
#define BLE_GATT_CHR_F_WRITE BLE_GATT_CHR_PROP_WRITE
#define BLE_GATT_CHR_F_READ BLE_GATT_CHR_PROP_READ
#define BLE_GATT_CHR_F_NOTIFY BLE_GATT_CHR_PROP_NOTIFY
enum { BLE_GATT_ACCESS_OP_WRITE_CHR, BLE_GATT_ACCESS_OP_READ_CHR,
       BLE_ATT_ERR_UNLIKELY = 10, BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN,
       BLE_ATT_ERR_INSUFFICIENT_RES, BLE_GATT_SVC_TYPE_PRIMARY };
struct ble_gatt_access_ctxt { int op; struct os_mbuf *om; };
struct ble_gatt_chr_def {
    const ble_uuid_t *uuid;
    int (*access_cb)(uint16_t, uint16_t, struct ble_gatt_access_ctxt *, void *);
    uint16_t flags;
    uint16_t *val_handle;
};
struct ble_gatt_svc_def {
    int type;
    const ble_uuid_t *uuid;
    const struct ble_gatt_chr_def *characteristics;
};
int ble_gattc_exchange_mtu(uint16_t conn, mtu_cb_t cb, void *arg);
int ble_gattc_disc_svc_by_uuid(uint16_t conn, const ble_uuid_t *uuid, svc_cb_t cb, void *arg);
int ble_gattc_disc_all_chrs(uint16_t conn, uint16_t start, uint16_t end, chr_cb_t cb, void *arg);
int ble_gattc_disc_all_dscs(uint16_t conn, uint16_t start, uint16_t end, dsc_cb_t cb, void *arg);
int ble_gattc_write_flat(uint16_t conn, uint16_t attr, const void *data, uint16_t length, attr_cb_t cb, void *arg);
int ble_gattc_read(uint16_t conn, uint16_t attr, attr_cb_t cb, void *arg);
uint16_t ble_att_mtu(uint16_t conn);
int ble_gatts_count_cfg(const struct ble_gatt_svc_def *services);
int ble_gatts_add_svcs(const struct ble_gatt_svc_def *services);
void ble_gatts_chr_updated(uint16_t attr);
void ble_svc_gap_init(void);
int ble_svc_gap_device_name_set(const char *name);
void ble_svc_gatt_init(void);

enum { BLE_GAP_EVENT_DISC, BLE_GAP_EVENT_DISC_COMPLETE, BLE_GAP_EVENT_CONNECT,
       BLE_GAP_EVENT_DISCONNECT, BLE_GAP_EVENT_NOTIFY_RX, BLE_GAP_EVENT_ADV_COMPLETE,
       BLE_GAP_EVENT_SUBSCRIBE, BLE_GAP_CONN_MODE_UND, BLE_GAP_DISC_MODE_GEN };
#define BLE_HS_ADV_F_DISC_GEN 2
#define BLE_HS_ADV_F_BREDR_UNSUP 4
struct ble_gap_event {
    int type;
    union {
        struct { ble_addr_t addr; int8_t rssi; const uint8_t *data; uint8_t length_data; } disc;
        struct { int reason; } disc_complete;
        struct { int status; uint16_t conn_handle; } connect;
        struct { int reason; struct { uint16_t conn_handle; } conn; } disconnect;
        struct { uint16_t conn_handle, attr_handle; struct os_mbuf *om; } notify_rx;
        struct { uint16_t attr_handle; uint8_t cur_notify; } subscribe;
    };
};
typedef int (*gap_cb_t)(struct ble_gap_event *, void *);
struct ble_gap_disc_params { uint8_t passive, filter_duplicates; };
struct ble_gap_adv_params { int conn_mode, disc_mode; };
struct ble_hs_adv_fields {
    uint8_t flags, *name, name_len, name_is_complete;
    const ble_uuid16_t *uuids16;
    uint8_t num_uuids16, uuids16_is_complete;
};
extern struct fake_hs_cfg { void (*sync_cb)(void); void (*reset_cb)(int); } ble_hs_cfg;
int ble_hs_util_ensure_addr(int prefer_random);
int ble_hs_id_infer_auto(int privacy, uint8_t *type);
int ble_gap_disc(uint8_t type, int32_t duration, const struct ble_gap_disc_params *params, gap_cb_t cb, void *arg);
int ble_gap_disc_cancel(void);
int ble_gap_connect(uint8_t type, const ble_addr_t *peer, int32_t timeout, const void *params, gap_cb_t cb, void *arg);
int ble_gap_conn_cancel(void);
int ble_gap_terminate(uint16_t conn, uint8_t reason);
int ble_gap_adv_set_fields(const struct ble_hs_adv_fields *fields);
int ble_gap_adv_start(uint8_t type, const ble_addr_t *direct, int32_t duration,
                      const struct ble_gap_adv_params *params, gap_cb_t cb, void *arg);
#endif
