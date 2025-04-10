#include <stdio.h>
#include "agHttpServer.h"

httpd_handle_t server = NULL;
httpd_config_t config = HTTPD_DEFAULT_CONFIG();

/*
Initialize http server.
Inicializar el servidor http.
Amdrés González
09-04-2025
*/
esp_err_t agHttpServer_initialize_http_server(void){
    esp_err_t err = ESP_OK;

    err = httpd_start(&server, &config);

    return err;
}

/*
Get http server.
Obtener servidor http.
Amdrés González
09-04-2025
*/
httpd_handle_t agHttpServer_get_http_server(void){
    return server;
}