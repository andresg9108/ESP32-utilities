# agHttpServer

A lifecycle wrapper for one ESP-IDF HTTP server using `HTTPD_DEFAULT_CONFIG()`, including the default HTTP port 80. The application establishes networking and registers URI handlers; this component does not connect Wi-Fi or provide web content.

Componente que gestiona el ciclo de vida de un servidor HTTP de ESP-IDF mediante `HTTPD_DEFAULT_CONFIG()`, incluido el puerto HTTP predeterminado 80. La aplicación establece la red y registra los manejadores de rutas URI; este componente no conecta Wi-Fi ni proporciona contenido web.

## Integration / Integración

Register `httpServer/agHttpServer` as a component directory, add `agHttpServer` to the consuming component's `REQUIRES`, and include `agHttpServer.h`. The public header supports C and C++ and exposes ESP-IDF's HTTP server types. See the [root README](../../README.md) for complete integration and the [Wi-Fi README](../../wifi/agwifi/README.md) for network configuration.

Registra `httpServer/agHttpServer` como directorio de componente, añade `agHttpServer` a `REQUIRES` del componente consumidor e incluye `agHttpServer.h`. El encabezado público es compatible con C y C++ y expone los tipos del servidor HTTP de ESP-IDF. Consulta el [README raíz](../../README.md) para la integración completa y el [README de Wi-Fi](../../wifi/agwifi/README.md) para configurar la red.

## Start and register a route / Iniciar y registrar una ruta

After establishing the network, call the following setup function once per server lifetime. It starts HTTP, registers `GET /` and returns setup errors. The response is kept inline, so this example does not require an embedded HTML file. The error path also reports a failure to stop the server.

Después de establecer la red, llama a la siguiente función de configuración una vez por cada ciclo de vida del servidor. Inicia HTTP, registra `GET /` y devuelve los errores de configuración. La respuesta se incluye directamente en el código, por lo que este ejemplo no necesita un archivo HTML embebido. La ruta de error también informa si falla la parada del servidor.

```c
#include "agHttpServer.h"
#include "esp_log.h"

static esp_err_t home_get(httpd_req_t *request)
{
    return httpd_resp_send(request, "ESP32", HTTPD_RESP_USE_STRLEN);
}

esp_err_t start_web_server(void)
{
    esp_err_t err = agHttpServer_initialize_http_server();
    if (err != ESP_OK) {
        return err;
    }
    const httpd_uri_t home = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = home_get,
    };
    err = httpd_register_uri_handler(agHttpServer_get_http_server(), &home);
    if (err != ESP_OK) {
        esp_err_t stop_err = agHttpServer_stop_http_server();
        if (stop_err != ESP_OK) {
            ESP_LOGE("app", "httpd_stop: %s", esp_err_to_name(stop_err));
        }
    }
    return err;
}
```

Initialization is idempotent, but route registration is a separate application operation. Calling the setup function above again while the server is running attempts to register the same route again and can fail. Register handlers once; register them again only after a successful stop and a new server startup.

La inicialización es idempotente, pero el registro de rutas es una operación independiente de la aplicación. Volver a llamar a la función anterior mientras el servidor está funcionando intenta registrar otra vez la misma ruta y puede fallar. Registra los manejadores una sola vez; vuelve a registrarlos únicamente después de una parada correcta y un nuevo arranque del servidor.

## Public API / API pública

### `esp_err_t agHttpServer_initialize_http_server(void)`

Starts the server with the SDK's default configuration and returns the result of `httpd_start()`. On success it retains the server handle; on failure no handle is exposed. If already running, it returns `ESP_OK` without starting a second server or replacing registered handlers. This wrapper does not accept a custom `httpd_config_t`.

Inicia el servidor con la configuración predeterminada del SDK y devuelve el resultado de `httpd_start()`. Si tiene éxito, conserva el handle del servidor; si falla, no expone ningún handle. Si ya está funcionando, devuelve `ESP_OK` sin iniciar un segundo servidor ni sustituir los manejadores registrados. Este componente no recibe un `httpd_config_t` personalizado.

### `httpd_handle_t agHttpServer_get_http_server(void)`

Returns a borrowed handle for use with functions such as `httpd_register_uri_handler()`. It returns `NULL` before successful startup and after successful shutdown. The component owns the server: stop it through this wrapper rather than calling `httpd_stop()` directly.

Devuelve un handle prestado para utilizarlo con funciones como `httpd_register_uri_handler()`. Devuelve `NULL` antes de un arranque correcto y después de una parada correcta. El componente es propietario del servidor: detenlo mediante esta API en lugar de llamar directamente a `httpd_stop()`.

### `esp_err_t agHttpServer_stop_http_server(void)`

Stops the server and clears the stored handle only when `httpd_stop()` succeeds. Returns `ESP_OK` if already stopped. If shutdown fails, it returns the SDK error and retains the handle so the application can handle the failure and retry. Previously obtained handles must not be used after a successful stop.

Detiene el servidor y limpia el handle almacenado únicamente cuando `httpd_stop()` tiene éxito. Devuelve `ESP_OK` si ya está detenido. Si la parada falla, devuelve el error del SDK y conserva el handle para que la aplicación gestione el fallo y vuelva a intentarlo. Los handles obtenidos previamente no deben utilizarse después de una parada correcta.

## Serve an embedded HTML file / Servir un archivo HTML embebido

Assets belong to the consuming application. Place `index.html` beside its source and register it in that component's `CMakeLists.txt`; `EMBED_FILES` makes the file's bytes available through linker symbols.

Los recursos pertenecen a la aplicación consumidora. Coloca `index.html` junto a su código fuente y regístralo en el `CMakeLists.txt` de ese componente; `EMBED_FILES` permite acceder a los bytes del archivo mediante símbolos del enlazador.

```cmake
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES agHttpServer
                       EMBED_FILES "index.html")
```

Use this handler instead of the inline-response handler above. For the filename `index.html`, ESP-IDF generates `_binary_index_html_start` and `_binary_index_html_end`. Pass their difference as the response length; do not assume the binary data is null-terminated. If the embedded path changes, its generated symbol names can change as well.

Utiliza este manejador en lugar del manejador de respuesta directa anterior. Para el nombre `index.html`, ESP-IDF genera `_binary_index_html_start` y `_binary_index_html_end`. Pasa su diferencia como longitud de respuesta; no supongas que los datos binarios terminan en un carácter nulo. Si cambia la ruta del archivo embebido, también pueden cambiar los nombres de los símbolos generados.

```c
extern const char index_start[] asm("_binary_index_html_start");
extern const char index_end[] asm("_binary_index_html_end");

static esp_err_t home_get(httpd_req_t *request)
{
    esp_err_t err = httpd_resp_set_type(request, "text/html");
    if (err != ESP_OK) {
        return err;
    }
    return httpd_resp_send(request, index_start, index_end - index_start);
}
```

## Shutdown and call context / Parada y contexto de llamada

Serialize start/stop/getter calls and coordinate all borrowed-handle uses with shutdown. Start and stop from an application task, not from a URI handler or the default event loop. A URI handler may send responses, but must not perform blocking lifecycle operations that depend on its own task making progress.

Serializa las llamadas de arranque, parada y consulta del handle, y coordina todos los usos del handle prestado con la parada. Inicia y detén desde una tarea de la aplicación, no desde un manejador URI ni desde el bucle de eventos predeterminado. Un manejador URI puede enviar respuestas, pero no debe realizar operaciones bloqueantes del ciclo de vida que dependan del progreso de su propia tarea.

Stop HTTP successfully before disconnecting or deinitializing Wi-Fi. If HTTP shutdown fails, handle that error before continuing with network teardown. After restarting HTTP, obtain the new handle and register the routes again.

Detén HTTP correctamente antes de desconectar o desinicializar Wi-Fi. Si falla la parada de HTTP, gestiona ese error antes de continuar con la liberación de la red. Después de reiniciar HTTP, obtén el nuevo handle y vuelve a registrar las rutas.

The minimal [`examples/http_server`](../../examples/http_server/README.md) application connects Wi-Fi and serves a greeting directly from C code. The [host tests](../../tests/host/README.md) cover startup/shutdown errors and repeated lifecycle calls without opening real sockets.

La aplicación mínima [`examples/http_server`](../../examples/http_server/README.md) conecta Wi-Fi y sirve un saludo directamente desde el código C. Las [pruebas en el equipo de desarrollo](../../tests/host/README.md) cubren errores de arranque y parada y llamadas repetidas del ciclo de vida sin abrir sockets reales.
