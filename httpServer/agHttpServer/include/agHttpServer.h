#ifndef AG_HTTP_SERVER_H
#define AG_HTTP_SERVER_H

#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Serialize start/stop/get calls in the application. Stop must run outside URI
 * handlers; previously obtained handles are invalid after a successful stop.
 * Serializa las llamadas de inicio, parada y consulta en la aplicación. La parada debe
 * ejecutarse fuera de los manejadores de URI; los identificadores obtenidos previamente
 * dejan de ser válidos tras una parada exitosa. */
esp_err_t agHttpServer_initialize_http_server(void);
esp_err_t agHttpServer_stop_http_server(void);
httpd_handle_t agHttpServer_get_http_server(void);

#ifdef __cplusplus
}
#endif

#endif
