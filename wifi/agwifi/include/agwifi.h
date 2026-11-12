#ifndef AGWIFI_H
#define AGWIFI_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Lifecycle calls must be serialized by the application, outside event handlers.
 * La aplicación debe serializar las llamadas de ciclo de vida, fuera de los manejadores de eventos.
 * Connect blocks until an IP is acquired, retries fail, or the timeout expires.
 * La conexión bloquea hasta obtener una IP, agotar los reintentos o vencer el tiempo de espera.
 * NVS errors are returned without erasing application data. The Wi-Fi driver
 * must not already be owned by another component.
 * Los errores de NVS se devuelven sin borrar los datos de la aplicación. El controlador
 * Wi-Fi no debe estar ya bajo el control de otro componente. */
esp_err_t agwifi_connect_to_wifi_network(void);
esp_err_t agwifi_disconnect_from_wifi_network(void);
esp_err_t agwifi_deinitialize(void);
bool agwifi_is_wifi_connected(void);
void agwifi_print_wifi_network_information(void);

#ifdef __cplusplus
}
#endif

#endif
