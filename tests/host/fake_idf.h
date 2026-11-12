#ifndef FAKE_IDF_H
#define FAKE_IDF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_NO_MEM 0x101
#define ESP_ERR_INVALID_ARG 0x102
#define ESP_ERR_INVALID_STATE 0x103
#define ESP_ERR_INVALID_SIZE 0x104
#define ESP_ERR_NOT_SUPPORTED 0x106
#define ESP_ERR_TIMEOUT 0x107
#define ESP_ERR_WIFI_NOT_INIT 0x3001
#define ESP_ERR_NVS_NO_FREE_PAGES 0x110d
const char *esp_err_to_name(esp_err_t err);
void fake_log(const char *tag, const char *format, ...);
#define ESP_LOGE(...) fake_log(__VA_ARGS__)
#define ESP_LOGW(...) fake_log(__VA_ARGS__)
#define ESP_LOGI(...) fake_log(__VA_ARGS__)

#ifndef CONFIG_ESP_WIFI_SSID
#define CONFIG_ESP_WIFI_SSID "test-network"
#endif
#define CONFIG_ESP_WIFI_PASSWORD "do-not-log-this-password"
#define CONFIG_ESP_MAXIMUM_RETRY 2
#define CONFIG_AGWIFI_CONNECT_TIMEOUT_MS 1000
#define CONFIG_ESP_WPA3_SAE_PWE_BOTH 1
#define CONFIG_ESP_WIFI_PW_ID ""
#define CONFIG_ESP_WIFI_AUTH_WPA2_PSK 1

typedef uint32_t EventBits_t;
typedef uint32_t TickType_t;
typedef struct fake_group *EventGroupHandle_t;
#define BIT0 1u
#define BIT1 2u
#define BIT2 4u
#define BIT3 8u
#define pdFALSE 0
#define pdMS_TO_TICKS(ms) (ms)
EventGroupHandle_t xEventGroupCreate(void);
void vEventGroupDelete(EventGroupHandle_t group);
EventBits_t xEventGroupGetBits(EventGroupHandle_t group);
EventBits_t xEventGroupSetBits(EventGroupHandle_t group, EventBits_t bits);
EventBits_t xEventGroupClearBits(EventGroupHandle_t group, EventBits_t bits);
EventBits_t xEventGroupWaitBits(EventGroupHandle_t group, EventBits_t bits,
                               int clear, int all, TickType_t timeout);

typedef const char *esp_event_base_t;
typedef void *esp_event_handler_instance_t;
typedef void (*esp_event_handler_t)(void *, esp_event_base_t, int32_t, void *);
extern const char WIFI_EVENT[];
extern const char IP_EVENT[];
#define ESP_EVENT_ANY_ID -1
enum { WIFI_EVENT_STA_START, WIFI_EVENT_STA_DISCONNECTED, WIFI_EVENT_STA_STOP };
enum { IP_EVENT_STA_GOT_IP, IP_EVENT_STA_LOST_IP };
esp_err_t esp_event_loop_create_default(void);
esp_err_t esp_event_handler_instance_register(esp_event_base_t base, int32_t id,
    esp_event_handler_t handler, void *arg, esp_event_handler_instance_t *instance);
esp_err_t esp_event_handler_instance_unregister(esp_event_base_t base, int32_t id,
    esp_event_handler_instance_t instance);

typedef struct { int unused; } esp_netif_t;
typedef struct { uint32_t addr; } esp_ip4_addr_t;
typedef struct { esp_ip4_addr_t ip, netmask, gw; } esp_netif_ip_info_t;
#define IPSTR "%u.%u.%u.%u"
#define IP2STR(ip) (unsigned)((ip)->addr & 255), (unsigned)(((ip)->addr >> 8) & 255), \
    (unsigned)(((ip)->addr >> 16) & 255), (unsigned)(((ip)->addr >> 24) & 255)
esp_err_t esp_netif_init(void);
esp_netif_t *esp_netif_get_handle_from_ifkey(const char *key);
esp_netif_t *esp_netif_create_default_wifi_sta(void);
void esp_netif_destroy_default_wifi(void *netif);
esp_err_t esp_netif_get_ip_info(esp_netif_t *netif, esp_netif_ip_info_t *info);

typedef int wifi_mode_t;
typedef struct { int unused; } wifi_init_config_t;
#define WIFI_INIT_CONFIG_DEFAULT() ((wifi_init_config_t){0})
enum { WIFI_MODE_STA, WIFI_IF_STA, WIFI_STORAGE_RAM };
enum { WPA3_SAE_PWE_HUNT_AND_PECK, WPA3_SAE_PWE_HASH_TO_ELEMENT, WPA3_SAE_PWE_BOTH };
enum { WIFI_AUTH_OPEN, WIFI_AUTH_WEP, WIFI_AUTH_WPA_PSK, WIFI_AUTH_WPA2_PSK,
       WIFI_AUTH_WPA_WPA2_PSK, WIFI_AUTH_WPA3_PSK, WIFI_AUTH_WPA2_WPA3_PSK, WIFI_AUTH_WAPI_PSK };
typedef union {
    struct {
        uint8_t ssid[32];
        uint8_t password[64];
        struct { int authmode; } threshold;
        int sae_pwe_h2e;
        char sae_h2e_identifier[33];
    } sta;
} wifi_config_t;
esp_err_t esp_wifi_get_mode(wifi_mode_t *mode);
esp_err_t esp_wifi_init(const wifi_init_config_t *config);
esp_err_t esp_wifi_deinit(void);
esp_err_t esp_wifi_set_storage(int storage);
esp_err_t esp_wifi_set_mode(wifi_mode_t mode);
esp_err_t esp_wifi_set_config(int interface, wifi_config_t *config);
esp_err_t esp_wifi_start(void);
esp_err_t esp_wifi_stop(void);
esp_err_t esp_wifi_connect(void);
esp_err_t nvs_flash_init(void);

typedef void *httpd_handle_t;
typedef struct { int server_port; } httpd_config_t;
#define HTTPD_DEFAULT_CONFIG() ((httpd_config_t){.server_port = 80})
esp_err_t httpd_start(httpd_handle_t *server, const httpd_config_t *config);
esp_err_t httpd_stop(httpd_handle_t server);

#ifdef __cplusplus
}
#endif
#endif
