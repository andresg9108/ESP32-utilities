#include "agwifi.h"
#include "agHttpServer.h"

static esp_err_t home_get(httpd_req_t *request)
{
    return httpd_resp_send(request, "Hello, ESP32!\n¡Hola, ESP32!\n", HTTPD_RESP_USE_STRLEN);
}

void app_main(void)
{
    ESP_ERROR_CHECK(agwifi_connect_to_wifi_network());
    agwifi_print_wifi_network_information();
    ESP_ERROR_CHECK(agHttpServer_initialize_http_server());

    const httpd_uri_t home = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = home_get,
    };
    ESP_ERROR_CHECK(httpd_register_uri_handler(agHttpServer_get_http_server(), &home));
}
