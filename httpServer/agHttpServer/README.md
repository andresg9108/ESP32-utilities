```c
#include <string.h>
#include "esp_http_server.h"
#include "agwifi.h"
#include "agHttpServer.h"

extern const char index_start[] asm("_binary_index_html_start");
extern const char index_end[] asm("_binary_index_html_end");
extern const char logo_start[] asm("_binary_logo_svg_start");
extern const char logo_end[] asm("_binary_logo_svg_end");

static esp_err_t homePage_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");

    const uint32_t index_len = index_end - index_start;
    httpd_resp_send(req, index_start, index_len);

    return ESP_OK;
}

static const httpd_uri_t homePage = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = homePage_get_handler
};

void web_server_init(void){
    esp_err_t err = agHttpServer_initialize_http_server();

    if(err != ESP_OK){
        printf("Error al iniciar el servidor HTTP: %s\n", esp_err_to_name(err));
        return;
    }

    httpd_handle_t server = agHttpServer_get_http_server();
    
    // httpd_register_uri_handler(server, &api);
    httpd_register_uri_handler(server, &homePage);
    // httpd_register_uri_handler(server, &chroma);
}

void app_main(void){
    agwifi_connect_to_wifi_network();

    if(!agwifi_is_wifi_connected()){
        printf("No hay ninguna conexión.\n");
    }else{
        agwifi_print_wifi_network_information();

        web_server_init();
    }
}
```

~~~
# main/Kconfig.projbuild

menu "Networking Configuration"

    config ESP_WIFI_SSID
        string "WiFi SSID"
        default "myssid"
        help
            SSID (network name) for the example to connect to.

    config ESP_WIFI_PASSWORD
        string "WiFi Password"
        default "mypassword"
        help
            WiFi password (WPA or WPA2) for the example to use.

    choice ESP_WIFI_SAE_MODE
        prompt "WPA3 SAE mode selection"
        default ESP_WPA3_SAE_PWE_BOTH
        help
            Select mode for SAE as Hunt and Peck, H2E or both.
        config ESP_WPA3_SAE_PWE_HUNT_AND_PECK
            bool "HUNT AND PECK"
        config ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
            bool "H2E"
        config ESP_WPA3_SAE_PWE_BOTH
            bool "BOTH"
    endchoice

    config ESP_WIFI_PW_ID
        string "PASSWORD IDENTIFIER"
        depends on  ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT|| ESP_WPA3_SAE_PWE_BOTH
        default ""
        help
            password identifier for SAE H2E

    config ESP_MAXIMUM_RETRY
        int "Maximum retry"
        default 5
        help
            Set the Maximum retry to avoid station reconnecting to the AP unlimited when the AP is really inexistent.

    choice ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD
        prompt "WiFi Scan auth mode threshold"
        default ESP_WIFI_AUTH_WPA2_PSK
        help
            The weakest authmode to accept in the scan mode.
            This value defaults to ESP_WIFI_AUTH_WPA2_PSK incase password is present and ESP_WIFI_AUTH_OPEN is used.
            Please select ESP_WIFI_AUTH_WEP/ESP_WIFI_AUTH_WPA_PSK incase AP is operating in WEP/WPA mode.

        config ESP_WIFI_AUTH_OPEN
            bool "OPEN"
        config ESP_WIFI_AUTH_WEP
            bool "WEP"
        config ESP_WIFI_AUTH_WPA_PSK
            bool "WPA PSK"
        config ESP_WIFI_AUTH_WPA2_PSK
            bool "WPA2 PSK"
        config ESP_WIFI_AUTH_WPA_WPA2_PSK
            bool "WPA/WPA2 PSK"
        config ESP_WIFI_AUTH_WPA3_PSK
            bool "WPA3 PSK"
        config ESP_WIFI_AUTH_WPA2_WPA3_PSK
            bool "WPA2/WPA3 PSK"
        config ESP_WIFI_AUTH_WAPI_PSK
            bool "WAPI PSK"
    endchoice

endmenu
~~~