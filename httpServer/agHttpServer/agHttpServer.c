#include "agHttpServer.h"

static httpd_handle_t s_server;

esp_err_t agHttpServer_initialize_http_server(void)
{
    if (s_server) {
        return ESP_OK;
    }
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    esp_err_t err = httpd_start(&server, &config);
    if (err == ESP_OK) {
        s_server = server;
    }
    return err;
}

esp_err_t agHttpServer_stop_http_server(void)
{
    if (!s_server) {
        return ESP_OK;
    }
    esp_err_t err = httpd_stop(s_server);
    if (err == ESP_OK) {
        s_server = NULL;
    }
    return err;
}

httpd_handle_t agHttpServer_get_http_server(void)
{
    return s_server;
}
