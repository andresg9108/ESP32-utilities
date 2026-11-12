# AG utilities ESP32

Reusable C components for ESP-IDF, distributed as the npm package `ag-util-esp32`. npm installs the source files; ESP-IDF compiles the firmware. There are no JavaScript entrypoints or npm build/test scripts.

Componentes reutilizables en C para ESP-IDF, distribuidos como el paquete npm `ag-util-esp32`. npm instala los archivos fuente; ESP-IDF compila el firmware. No hay puntos de entrada de JavaScript ni scripts de compilación o pruebas de npm.

The example builds have been verified with **ESP-IDF 5.4.2** for the **esp32** target. Host regressions check simulated lifecycle/error paths; actual Wi-Fi, HTTP and BLE behavior still requires a board and the corresponding network or peer.

La compilación de los ejemplos se ha verificado con **ESP-IDF 5.4.2** para el target **esp32**. Las pruebas en el equipo de desarrollo comprueban escenarios simulados de ciclo de vida y errores; el comportamiento real de Wi-Fi, HTTP y BLE requiere además una placa y la red o cliente correspondientes.

## Components and examples / Componentes y ejemplos

- [`agexample`](agexample/README.md): a minimal component exposing `hello()`, useful as an integration example.

  [`agexample`](agexample/README.md): componente mínimo que expone `hello()`, útil como ejemplo de integración.
- [`agwifi`](wifi/agwifi/README.md): station-mode Wi-Fi connection, connection queries, reconnection, disconnection and resource cleanup.

  [`agwifi`](wifi/agwifi/README.md): conexión Wi-Fi en modo estación, consulta del estado, reconexión, desconexión y liberación de recursos.
- [`agHttpServer`](httpServer/agHttpServer/README.md): startup, access and shutdown of a single ESP-IDF HTTP server; the application supplies URI handlers.

  [`agHttpServer`](httpServer/agHttpServer/README.md): arranque, acceso y parada de un único servidor HTTP de ESP-IDF; la aplicación proporciona los manejadores de rutas URI.
- [`agBLE`](BLE/agBLE/README.md): NimBLE central/client and peripheral/server modes with exclusive host ownership and configurable compilation.

  [`agBLE`](BLE/agBLE/README.md): modos central/cliente y periférico/servidor NimBLE con propiedad exclusiva del host y compilación configurable.
- [`examples/agexample`](examples/agexample/README.md): a minimal application that prints a greeting, without networking.

  [`examples/agexample`](examples/agexample/README.md): aplicación mínima que imprime un saludo, sin conexión de red.
- [`examples/wifi`](examples/wifi/README.md): connects to Wi-Fi and prints network information.

  [`examples/wifi`](examples/wifi/README.md): conecta Wi-Fi y muestra la información de red.
- [`examples/http_server`](examples/http_server/README.md): uses Wi-Fi and serves a greeting at `GET /`.

  [`examples/http_server`](examples/http_server/README.md): utiliza Wi-Fi y sirve un saludo en `GET /`.
- [`examples/ble`](examples/ble/README.md): receives BLE commands to control GPIO 4 and publishes its state, adapted from `bot/main`.

  [`examples/ble`](examples/ble/README.md): recibe comandos BLE para controlar GPIO 4 y publica su estado, adaptado de `bot/main`.
- [`tests/host`](tests/host/README.md): automated regressions that run on the development computer without a board or a local ESP-IDF installation.

  [`tests/host`](tests/host/README.md): pruebas de regresión automatizadas que se ejecutan en el equipo de desarrollo sin placa ni instalación local de ESP-IDF.

## Build an example / Compilar un ejemplo

Install ESP-IDF and activate its environment in the current terminal. Each example is an independent application. From this repository's root, select the target, configure Wi-Fi and build in this order for the HTTP example:

Instala ESP-IDF y activa su entorno en la terminal actual. Cada ejemplo es una aplicación independiente. Desde la raíz de este repositorio, selecciona el target, configura Wi-Fi y compila en este orden para el ejemplo HTTP:

```sh
idf.py -C examples/http_server set-target esp32
idf.py -C examples/http_server menuconfig
idf.py -C examples/http_server build
```

In **Component config → AG Wi-Fi configuration**, set the SSID and password to match your network. The SSID is empty by default: the firmware can compile, but connection returns `ESP_ERR_INVALID_ARG` until it is configured. For the Wi-Fi-only example, replace `examples/http_server` with `examples/wifi`. For the greeting example, use `examples/agexample` and skip menuconfig; it needs no credentials.

En **Component config → AG Wi-Fi configuration**, configura el SSID y la contraseña de tu red. El SSID está vacío de forma predeterminada: el firmware puede compilar, pero la conexión devuelve `ESP_ERR_INVALID_ARG` hasta configurarlo. Para el ejemplo solo de Wi-Fi, sustituye `examples/http_server` por `examples/wifi`. Para el ejemplo del saludo, utiliza `examples/agexample` y omite menuconfig; no necesita credenciales.

Flash and open the serial monitor, replacing `/dev/ttyUSB0` with your board's port. Once the console prints an IP address, open `http://<IP>/` from a device that can reach the ESP32 on that network. Exit the ESP-IDF monitor with `Ctrl+]`.

Graba el firmware y abre el monitor serie, sustituyendo `/dev/ttyUSB0` por el puerto de tu placa. Cuando la consola muestre una dirección IP, abre `http://<IP>/` desde un dispositivo que pueda acceder al ESP32 en esa red. Sal del monitor de ESP-IDF con `Ctrl+]`.

```sh
idf.py -C examples/http_server -p /dev/ttyUSB0 flash monitor
```

The BLE example uses its own `sdkconfig.defaults`, requires no Wi-Fi credentials and can be built with `idf.py -C examples/ble set-target esp32` followed by `idf.py -C examples/ble build`. Its README explains the advertised name, UUIDs and one-byte commands.

El ejemplo BLE utiliza su propio `sdkconfig.defaults`, no necesita credenciales Wi-Fi y puede compilarse con `idf.py -C examples/ble set-target esp32` seguido de `idf.py -C examples/ble build`. Su README explica el nombre anunciado, los UUID y los comandos de un byte.

## Install and integrate / Instalar e integrar

To use these components in another ESP-IDF application, install the package from that application's root, or use a local checkout of this repository. Installing through npm does not configure or build an ESP-IDF application.

Para utilizar estos componentes en otra aplicación ESP-IDF, instala el paquete desde la raíz de esa aplicación o utiliza una copia local de este repositorio. La instalación con npm no configura ni compila una aplicación ESP-IDF.

```sh
npm install ag-util-esp32 --save
```

In the consuming application's root `CMakeLists.txt`, register the component directories **before** including ESP-IDF's `project.cmake`. This example uses the npm installation:

En el `CMakeLists.txt` raíz de la aplicación consumidora, registra los directorios de componentes **antes** de incluir `project.cmake` de ESP-IDF. Este ejemplo utiliza la instalación mediante npm:

```cmake
cmake_minimum_required(VERSION 3.16)

set(AG_UTILS_DIR "${CMAKE_CURRENT_LIST_DIR}/node_modules/ag-util-esp32")
set(EXTRA_COMPONENT_DIRS
    "${AG_UTILS_DIR}/agexample"
    "${AG_UTILS_DIR}/wifi/agwifi"
    "${AG_UTILS_DIR}/httpServer/agHttpServer"
    "${AG_UTILS_DIR}/BLE/agBLE")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(my_esp32_app)
```

For a local checkout, change `AG_UTILS_DIR` to its path, for example `${CMAKE_CURRENT_LIST_DIR}/../ESP32-utilities`. Use either the local checkout or the npm package, not both. ESP-IDF can also discover the components through the repository root, `wifi/`, `httpServer/` and `BLE/` as grouped `EXTRA_COMPONENT_DIRS` entries. Bluetooth stays disabled unless configured; see the agBLE README for the SDK role prerequisites.

Para una copia local, cambia `AG_UTILS_DIR` por su ruta, por ejemplo `${CMAKE_CURRENT_LIST_DIR}/../ESP32-utilities`. Utiliza la copia local o el paquete npm, no ambos a la vez. ESP-IDF también puede descubrir los componentes mediante la raíz del repositorio, `wifi/`, `httpServer/` y `BLE/` como entradas agrupadas de `EXTRA_COMPONENT_DIRS`. Bluetooth permanece deshabilitado salvo que se configure; consulta los requisitos de roles del SDK en el README de agBLE.

In the consuming component's `CMakeLists.txt`, declare the components it uses. List web assets in `EMBED_FILES` only when that application serves them; the HTTP component does not supply an `index.html` of its own.

En el `CMakeLists.txt` del componente consumidor, declara los componentes que utiliza. Incluye los recursos web en `EMBED_FILES` solo cuando esa aplicación los sirva; el componente HTTP no proporciona un `index.html` propio.

```cmake
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES agexample agwifi agHttpServer
                       EMBED_FILES "index.html")
```

There is no root firmware application to build in this repository. Build one of the applications under `examples/` or your consuming application, rather than running `idf.py build` at the repository root. Public headers support C and C++.

Este repositorio no contiene una aplicación de firmware en su raíz. Compila una de las aplicaciones de `examples/` o tu aplicación consumidora, en lugar de ejecutar `idf.py build` en la raíz del repositorio. Los encabezados públicos son compatibles con C y C++.

## Verification / Verificación

Run host regressions from the repository root with Python 3, GCC and G++ available. They compile the production C with `-Werror` and UBSan, run simulated Wi-Fi/HTTP/BLE scenarios and check C++ linkage. See the [test README](tests/host/README.md) for coverage and limitations and the [SDK link checks](tests/idf/README.md) for BLE configuration variants.

Ejecuta las pruebas de regresión desde la raíz del repositorio con Python 3, GCC y G++ disponibles. Compilan el código C de producción con `-Werror` y UBSan, ejecutan escenarios simulados de Wi-Fi/HTTP/BLE y comprueban el enlace desde C++. Consulta el [README de pruebas](tests/host/README.md) para conocer la cobertura y las limitaciones y las [comprobaciones de enlace con el SDK](tests/idf/README.md) para las variantes de configuración BLE.

```sh
python3 tests/host/run.py
```

For an SDK build without installing ESP-IDF locally, run the following Docker command from the repository root. It mounts the source read-only, builds the HTTP example and removes the container, including its build artifacts, on exit. Replace `/project/examples/http_server` with `/project/examples/agexample`, `/project/examples/wifi` or `/project/examples/ble` to check the other applications. This checks compilation; it does not flash a board or test networking.

Para compilar con el SDK sin instalar ESP-IDF localmente, ejecuta el siguiente comando Docker desde la raíz del repositorio. Monta el código en modo de solo lectura, compila el ejemplo HTTP y elimina el contenedor, incluidos sus archivos de compilación, al salir. Sustituye `/project/examples/http_server` por `/project/examples/agexample`, `/project/examples/wifi` o `/project/examples/ble` para comprobar las otras aplicaciones. Comprueba la compilación; no graba una placa ni prueba la red.

```sh
docker run --rm -v "$PWD:/project:ro" -w /project/examples/http_server espressif/idf:v5.4.2 idf.py -B /tmp/ag-build -D SDKCONFIG=/tmp/ag-sdkconfig build
```

## Migration and lifecycle / Migración y ciclo de vida

`agwifi_connect_to_wifi_network()` returns `esp_err_t`; check the result before starting network services. Wi-Fi configuration now belongs to `wifi/agwifi/Kconfig`: remove duplicate networking definitions previously copied into your application's `main/Kconfig.projbuild`. Existing `ESP_WIFI_*` and `ESP_MAXIMUM_RETRY` setting names are preserved.

`agwifi_connect_to_wifi_network()` devuelve `esp_err_t`; comprueba el resultado antes de iniciar servicios de red. La configuración Wi-Fi ahora pertenece a `wifi/agwifi/Kconfig`: elimina las definiciones de red duplicadas que hayas copiado antes a `main/Kconfig.projbuild` de tu aplicación. Se conservan los nombres de configuración `ESP_WIFI_*` y `ESP_MAXIMUM_RETRY` existentes.

Serialize component API calls, including queries during teardown. Call blocking lifecycle operations from an application task, outside event and URI handlers. Stop HTTP before disconnecting or deinitializing Wi-Fi. NVS errors are returned without automatically erasing application data; consult the component READMEs for ownership and error handling.

Serializa las llamadas a las API de los componentes, incluidas las consultas durante la liberación de recursos. Ejecuta las operaciones bloqueantes del ciclo de vida desde una tarea de la aplicación, fuera de los manejadores de eventos y de rutas URI. Detén HTTP antes de desconectar o desinicializar Wi-Fi. Los errores de NVS se devuelven sin borrar automáticamente los datos de la aplicación; consulta los README de los componentes para conocer la propiedad de los recursos y el manejo de errores.

## License / Licencia

Distributed under the MIT license. See [`LICENSE.txt`](LICENSE.txt).

Distribuido bajo la licencia MIT. Consulta [`LICENSE.txt`](LICENSE.txt).

## OpenCode component agent / Agente de componentes de OpenCode

The project includes an `esp32-components` agent and three skills for component implementation, minimal examples/bilingual documentation and verification. Start it with `opencode --agent esp32-components` after restarting OpenCode. See [`.opencode/README.md`](.opencode/README.md) for usage.

El proyecto incluye un agente `esp32-components` y tres skills para implementación de componentes, ejemplos mínimos/documentación bilingüe y verificación. Inícialo con `opencode --agent esp32-components` después de reiniciar OpenCode. Consulta [`.opencode/README.md`](.opencode/README.md) para su uso.
