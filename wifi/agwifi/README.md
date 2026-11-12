# agwifi

An ESP-IDF Wi-Fi helper for station mode: the ESP32 joins an existing access point, waits for an IPv4 address and can automatically retry after disconnection. It does not create an access point or start an HTTP server.

Componente auxiliar Wi-Fi de ESP-IDF para modo estación: el ESP32 se conecta a un punto de acceso existente, espera una dirección IPv4 y puede reintentar automáticamente tras una desconexión. No crea un punto de acceso ni inicia un servidor HTTP.

## Integration / Integración

Register `wifi/agwifi` as an ESP-IDF component directory and declare `agwifi` in the consuming component's `REQUIRES`. Include `agwifi.h` from C or C++; it exposes the public functions without requiring your application to include the component's internal FreeRTOS or driver headers. See the [root README](../../README.md) for complete CMake integration.

Registra `wifi/agwifi` como directorio de componente ESP-IDF y declara `agwifi` en `REQUIRES` del componente consumidor. Incluye `agwifi.h` desde C o C++; expone las funciones públicas sin exigir que tu aplicación incluya los encabezados internos de FreeRTOS o del driver. Consulta el [README raíz](../../README.md) para ver la integración CMake completa.

## Configuration / Configuración

Run `idf.py menuconfig` from the consuming application, or `idf.py -C examples/wifi menuconfig` from this repository's root. Settings are under **Component config → AG Wi-Fi configuration** and are defined by this component's [`Kconfig`](Kconfig).

Ejecuta `idf.py menuconfig` desde la aplicación consumidora, o `idf.py -C examples/wifi menuconfig` desde la raíz de este repositorio. Las opciones están en **Component config → AG Wi-Fi configuration** y las define el archivo [`Kconfig`](Kconfig) de este componente.

- `CONFIG_ESP_WIFI_SSID` and `CONFIG_ESP_WIFI_PASSWORD`: network credentials, both empty by default. An empty SSID prevents connection and returns `ESP_ERR_INVALID_ARG`. The SSID and password must fit the driver's fields; oversized compile-time values produce a compilation error.

  `CONFIG_ESP_WIFI_SSID` y `CONFIG_ESP_WIFI_PASSWORD`: credenciales de red, ambas vacías de forma predeterminada. Un SSID vacío impide la conexión y devuelve `ESP_ERR_INVALID_ARG`. El SSID y la contraseña deben caber en los campos del driver; los valores de compilación demasiado largos producen un error al compilar.
- `CONFIG_ESP_MAXIMUM_RETRY`: additional reconnection attempts after a disconnection, from 0 to 100, default 5. This limit is separate from the connection timeout; the retry count resets after acquiring an IP.

  `CONFIG_ESP_MAXIMUM_RETRY`: intentos adicionales de reconexión tras una desconexión, de 0 a 100, con valor predeterminado 5. Este límite es independiente del timeout de conexión; el contador se reinicia al obtener una IP.
- `CONFIG_AGWIFI_CONNECT_TIMEOUT_MS`: IP acquisition wait, from 1000 to 3600000 ms, default 30000 ms. The same timeout is used when waiting for the station's stop event, so connection failure followed by shutdown can take longer than a single timeout interval.

  `CONFIG_AGWIFI_CONNECT_TIMEOUT_MS`: espera para obtener IP, de 1000 a 3600000 ms, con valor predeterminado 30000 ms. Se utiliza el mismo timeout para esperar el evento de parada de la estación, por lo que un fallo de conexión seguido de la parada puede tardar más que un único intervalo de timeout.
- Authentication threshold: the weakest authentication mode to accept, default WPA2 PSK. Choose a mode compatible with the access point; an intentionally open network needs an appropriate threshold and an empty password.

  Umbral de autenticación: modo de autenticación más débil que se acepta, WPA2 PSK de forma predeterminada. Elige un modo compatible con el punto de acceso; una red intencionadamente abierta necesita un umbral adecuado y una contraseña vacía.
- WPA3 SAE: choose Hunt and Peck, Hash to Element, or Both; Both is the default. `CONFIG_ESP_WIFI_PW_ID` is an optional password identifier for modes using Hash to Element, not a replacement for the Wi-Fi password.

  WPA3 SAE: elige Hunt and Peck, Hash to Element o Both; Both es el valor predeterminado. `CONFIG_ESP_WIFI_PW_ID` es un identificador de contraseña opcional para los modos que utilizan Hash to Element, no un sustituto de la contraseña Wi-Fi.

## Minimal application / Aplicación mínima

This example connects once, prints network information and leaves Wi-Fi running on success. On failure, it reports both the connection error and any cleanup error. Configure the credentials before running it.

Este ejemplo realiza una conexión, muestra la información de red y deja Wi-Fi en funcionamiento si tiene éxito. En caso de fallo, informa tanto del error de conexión como de cualquier error al liberar recursos. Configura las credenciales antes de ejecutarlo.

```c
#include "agwifi.h"
#include "esp_log.h"

void app_main(void)
{
    esp_err_t err = agwifi_connect_to_wifi_network();
    if (err != ESP_OK) {
        ESP_LOGE("app", "Wi-Fi: %s", esp_err_to_name(err));
        esp_err_t cleanup_err = agwifi_deinitialize();
        if (cleanup_err != ESP_OK) {
            ESP_LOGE("app", "agwifi_deinitialize: %s", esp_err_to_name(cleanup_err));
        }
        return;
    }
    agwifi_print_wifi_network_information();
}
```

## Public API / API pública

### `esp_err_t agwifi_connect_to_wifi_network(void)`

Initializes resources on first use, starts the station and blocks until an IP is obtained, retries fail or the wait expires. Returns `ESP_OK` if already connected; if automatic reconnection is in progress, it waits for that attempt instead of starting a second one. After failure or timeout, another call can try again.

Inicializa los recursos en el primer uso, arranca la estación y bloquea hasta obtener IP, fallar los reintentos o agotarse la espera. Devuelve `ESP_OK` si ya está conectado; si hay una reconexión automática en curso, espera ese intento en lugar de iniciar otro. Tras un fallo o timeout, otra llamada puede volver a intentarlo.

- `ESP_OK`: an IP-acquired event has been processed.

  `ESP_OK`: se ha procesado un evento de obtención de IP.
- `ESP_ERR_INVALID_ARG`: the configured SSID is empty.

  `ESP_ERR_INVALID_ARG`: el SSID configurado está vacío.
- `ESP_FAIL`: retry exhaustion or an asynchronous failure to initiate connection, provided station shutdown succeeds.

  `ESP_FAIL`: agotamiento de reintentos o fallo asíncrono al iniciar la conexión, siempre que la parada de la estación tenga éxito.
- `ESP_ERR_TIMEOUT`: the IP wait or the station-stop wait expired.

  `ESP_ERR_TIMEOUT`: se agotó la espera de IP o del evento de parada de la estación.
- `ESP_ERR_INVALID_STATE`: another component already owns an initialized Wi-Fi driver, or an incomplete teardown requires another call to `agwifi_deinitialize()` before reconnecting.

  `ESP_ERR_INVALID_STATE`: otro componente ya tiene un driver Wi-Fi inicializado, o una liberación incompleta requiere otra llamada a `agwifi_deinitialize()` antes de reconectar.

Other initialization, startup or shutdown errors are propagated from ESP-IDF, including NVS and memory allocation errors handled by the component. If shutdown fails after a connection failure, the shutdown error takes precedence.

Los demás errores de inicialización, arranque o parada se propagan desde ESP-IDF, incluidos los errores de NVS y de asignación de memoria que maneja el componente. Si la parada falla después de un fallo de conexión, tiene prioridad el error de parada.

### `bool agwifi_is_wifi_connected(void)`

Reports the latest processed connection-event state. It is safe before initialization and after successful teardown, when it returns `false`. Disconnect and lost-IP events clear the state; `true` does not guarantee Internet access or reachability of a remote service.

Informa del último estado procesado de los eventos de conexión. Puede llamarse antes de inicializar y después de una liberación correcta, cuando devuelve `false`. Los eventos de desconexión y pérdida de IP limpian el estado; `true` no garantiza acceso a Internet ni disponibilidad de un servicio remoto.

### `void agwifi_print_wifi_network_information(void)`

Prints the IP address, gateway and netmask when connected. Otherwise it logs a message; errors reading interface information are logged without printing uninitialized data. This function does not return an error code.

Muestra la dirección IP, la puerta de enlace y la máscara de red cuando existe conexión. En caso contrario registra un mensaje; los errores al leer la información de la interfaz se registran sin mostrar datos no inicializados. Esta función no devuelve un código de error.

### `esp_err_t agwifi_disconnect_from_wifi_network(void)`

Disables automatic reconnection, clears the connected state, stops the station and waits for its stop event. The driver remains initialized, so a later connect can restart it. Before initialization or after a completed stop, it returns `ESP_OK`; stop errors or a stop timeout are returned to the caller.

Desactiva la reconexión automática, limpia el estado de conexión, detiene la estación y espera su evento de parada. El driver permanece inicializado, por lo que una llamada posterior a la conexión puede reiniciarlo. Antes de inicializar o después de una parada completada devuelve `ESP_OK`; los errores de parada o el timeout se devuelven al llamador.

### `esp_err_t agwifi_deinitialize(void)`

Stops Wi-Fi and releases the owned driver, event handlers, event group and any default station interface created by this component. It is safe to call repeatedly after successful cleanup. On failure, it preserves the remaining cleanup state: retry teardown before reconnecting.

Detiene Wi-Fi y libera el driver propio, los manejadores de eventos, el grupo de eventos y cualquier interfaz de estación predeterminada creada por este componente. Puede llamarse repetidamente después de liberar los recursos correctamente. En caso de fallo, conserva el estado de limpieza restante: vuelve a intentar la liberación antes de reconectar.

## Ownership and call context / Propiedad de los recursos y contexto de llamada

The component owns the Wi-Fi driver but borrows a pre-existing `WIFI_STA_DEF` interface. That borrowed interface must remain valid until teardown and is not destroyed by the component. NVS, esp-netif and the default event loop may already be initialized; they are shared process-wide resources and remain available after cleanup.

El componente es propietario del driver Wi-Fi, pero utiliza como recurso prestado una interfaz `WIFI_STA_DEF` preexistente. Esa interfaz debe permanecer válida hasta la liberación y el componente no la destruye. NVS, esp-netif y el bucle de eventos predeterminado pueden estar inicializados previamente; son recursos compartidos de todo el proceso y siguen disponibles después de la limpieza.

NVS initialization failures are returned **without erasing the NVS partition**. The application decides how to recover its stored data. The component configures Wi-Fi with RAM storage and never prints credentials in its own logs; credentials selected through Kconfig are still part of the firmware build.

Los fallos de inicialización de NVS se devuelven **sin borrar la partición NVS**. La aplicación decide cómo recuperar sus datos almacenados. El componente configura Wi-Fi con almacenamiento en RAM y nunca imprime credenciales en sus propios registros; las credenciales seleccionadas mediante Kconfig siguen formando parte de la compilación del firmware.

Serialize all API calls in the application, including queries during teardown. Blocking connect/disconnect/deinitialize calls must run in an application task, outside default event-loop and URI handlers. Do not independently reconfigure or release the owned driver or destroy its interface. Stop HTTP and other dependent services before releasing Wi-Fi.

Serializa todas las llamadas a la API en la aplicación, incluidas las consultas durante la liberación. Las llamadas bloqueantes de conexión, desconexión y desinicialización deben ejecutarse en una tarea de la aplicación, fuera de los manejadores del bucle de eventos predeterminado y de rutas URI. No reconfigures ni liberes por separado el driver del componente ni destruyas su interfaz. Detén HTTP y los demás servicios dependientes antes de liberar Wi-Fi.

## Migration and complete example / Migración y ejemplo completo

Remove duplicate Wi-Fi settings previously copied into the application's `main/Kconfig.projbuild`. This component now supplies them, preserving the existing `ESP_WIFI_*` and `ESP_MAXIMUM_RETRY` names. Update callers to check the new `esp_err_t` result from `agwifi_connect_to_wifi_network()`.

Elimina las opciones Wi-Fi duplicadas que hayas copiado antes a `main/Kconfig.projbuild` de la aplicación. Ahora las proporciona este componente, conservando los nombres `ESP_WIFI_*` y `ESP_MAXIMUM_RETRY` existentes. Actualiza las llamadas para comprobar el nuevo resultado `esp_err_t` de `agwifi_connect_to_wifi_network()`.

See [`examples/wifi`](../../examples/wifi/README.md) for a minimal connection application, [`examples/http_server`](../../examples/http_server/README.md) for HTTP over Wi-Fi, and [`tests/host`](../../tests/host/README.md) for simulated regressions.

Consulta [`examples/wifi`](../../examples/wifi/README.md) para ver una aplicación mínima de conexión, [`examples/http_server`](../../examples/http_server/README.md) para HTTP sobre Wi-Fi y [`tests/host`](../../tests/host/README.md) para las pruebas de regresión simuladas.
