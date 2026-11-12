# Host regression tests / Pruebas de regresión en el equipo de desarrollo

These tests compile the production components against deterministic ESP-IDF fakes and execute simulated lifecycle/error scenarios on the development computer. They require **Python 3, GCC and G++**, with UBSan support. A board, network connection, ESP-IDF installation and npm installation are not required.

Estas pruebas compilan los componentes de producción contra simulaciones deterministas de ESP-IDF y ejecutan escenarios simulados de ciclo de vida y errores en el equipo de desarrollo. Requieren **Python 3, GCC y G++**, con soporte para UBSan. No necesitan placa, conexión de red, instalación de ESP-IDF ni instalación mediante npm.

## Run / Ejecutar

From the repository root, run the following command. A successful run exits with status 0 and reports the Wi-Fi, HTTP, BLE, empty-SSID and C++ linkage checks. Assertion, compilation, linkage or detected undefined-behavior failures cause a nonzero exit status.

Desde la raíz del repositorio, ejecuta el siguiente comando. Una ejecución correcta termina con código 0 e informa de las comprobaciones de Wi-Fi, HTTP, BLE, SSID vacío y enlace C++. Los fallos de aserción, compilación, enlace o comportamiento indefinido detectado producen un código de salida distinto de cero.

```sh
python3 tests/host/run.py
```

The runner resolves source paths relative to itself, so it can also be invoked from another directory using its absolute path. It currently runs the complete suite; there is no command-line selector for a single scenario. To select compiler executables, set `CC` and `CXX`:

El ejecutor resuelve las rutas del código a partir de su propia ubicación, por lo que también puede invocarse desde otro directorio mediante su ruta absoluta. Actualmente ejecuta toda la batería; no hay un selector por línea de comandos para un escenario individual. Para seleccionar los ejecutables de los compiladores, define `CC` y `CXX`:

```sh
CC=gcc CXX=g++ python3 tests/host/run.py
```

## Coverage / Cobertura

- Wi-Fi: queries before initialization, NVS errors, driver ownership, allocation/setup/start failures, repeated connection calls, disconnect/lost-IP state, reconnection, retry exhaustion, timeouts and teardown recovery.

  Wi-Fi: consultas antes de inicializar, errores de NVS, propiedad del driver, fallos de asignación, configuración y arranque, llamadas repetidas de conexión, estado tras desconexión o pérdida de IP, reconexión, agotamiento de reintentos, timeouts y recuperación al liberar recursos.
- Resource ownership: a borrowed default station interface survives teardown, owned resources are released and event handlers are removed before their event group is deleted.

  Propiedad de recursos: una interfaz de estación predeterminada prestada sobrevive a la liberación, los recursos propios se liberan y los manejadores de eventos se eliminan antes de borrar su grupo de eventos.
- Logging and configuration: the test password must not appear in the component's log output, and an empty SSID must be rejected without initializing the driver or allocating an event group.

  Registros y configuración: la contraseña de prueba no debe aparecer en los registros del componente, y un SSID vacío debe rechazarse sin inicializar el driver ni asignar un grupo de eventos.
- HTTP: startup/shutdown errors, handle state, repeated start/stop calls and restart after shutdown.

  HTTP: errores de arranque y parada, estado del handle, llamadas repetidas de arranque y parada y reinicio después de detener el servidor.
- C++ compatibility: repeated inclusion of public headers and linkage from a C++ consumer into the production C objects. The C++ consumer is linked but not executed.

  Compatibilidad con C++: inclusión repetida de encabezados públicos y enlace de un consumidor C++ con los objetos C de producción. El consumidor C++ se enlaza, pero no se ejecuta.
- BLE: argument/state validation, exclusive ownership, discovery/subscription, MTU limits, read/write completion, notifications, late I/O callbacks, server commands/status, restart, stop errors, task-exit timeout and retained ownership after partial SDK cleanup. Disabled public APIs and C++ declarations are also checked.

  BLE: validación de argumentos/estados, propiedad exclusiva, descubrimiento/suscripción, límites de MTU, finalización de lectura/escritura, notificaciones, callbacks tardíos de E/S, comandos/estado del servidor, reinicio, errores de parada, timeout de salida de tarea y conservación de la propiedad tras limpieza parcial del SDK. También se comprueban las API deshabilitadas y las declaraciones C++.

## Implementation and artifacts / Implementación y archivos generados

[`run.py`](run.py) compiles C11 (GNU C11 for the NimBLE sources) and C++11 with `-Wall -Wextra -Werror`, `-fsanitize=undefined` and `-fno-sanitize-recover=undefined`. [`fake_idf.h`](fake_idf.h) supplies the fake SDK types; [`test_components.c`](test_components.c) implements the Wi-Fi/HTTP operations and assertions; [`headers.cpp`](headers.cpp) checks the public C++ interface. [`ble/fake_nimble.h`](ble/fake_nimble.h), [`ble/test_ble.c`](ble/test_ble.c) and [`ble/test_disabled.c`](ble/test_disabled.c) exercise the production BLE implementation and disabled modes.

[`run.py`](run.py) compila C11 (GNU C11 para el código NimBLE) y C++11 con `-Wall -Wextra -Werror`, `-fsanitize=undefined` y `-fno-sanitize-recover=undefined`. [`fake_idf.h`](fake_idf.h) proporciona los tipos simulados del SDK; [`test_components.c`](test_components.c) implementa las operaciones y aserciones de Wi-Fi/HTTP; [`headers.cpp`](headers.cpp) comprueba la interfaz pública desde C++. [`ble/fake_nimble.h`](ble/fake_nimble.h), [`ble/test_ble.c`](ble/test_ble.c) y [`ble/test_disabled.c`](ble/test_disabled.c) comprueban la implementación BLE de producción y los modos deshabilitados.

Generated forwarding headers, object files and executables live in a temporary directory and are removed when the runner exits normally or raises a test failure. Keep these source tests in version control; generated binaries are not needed in the repository. Test credentials are fixtures, not real network credentials.

Los encabezados de redirección generados, archivos objeto y ejecutables se guardan en un directorio temporal y se eliminan cuando el ejecutor termina normalmente o propaga un fallo de prueba. Conserva el código fuente de estas pruebas en el control de versiones; los binarios generados no son necesarios en el repositorio. Las credenciales de prueba son datos de simulación, no credenciales de una red real.

## Limitations / Limitaciones

The fakes do not execute real FreeRTOS scheduling, radio operations, DHCP or HTTP sockets. Passing these tests does not prove timing correctness under concurrent hardware events, compatibility with every ESP-IDF version, or connectivity to a network/BLE peer. Validate SDK compilation with the [`agexample`](../../examples/agexample/README.md), [`wifi`](../../examples/wifi/README.md), [`http_server`](../../examples/http_server/README.md) and [`ble`](../../examples/ble/README.md) applications, plus the [BLE SDK link variants](../idf/README.md), then check radio, HTTP and GPIO behavior on the actual board.

Las simulaciones no ejecutan la planificación real de FreeRTOS, operaciones de radio, DHCP ni sockets HTTP. Superar estas pruebas no demuestra la corrección temporal ante eventos concurrentes en hardware, la compatibilidad con todas las versiones de ESP-IDF ni la conectividad con una red/cliente BLE. Valida la compilación con el SDK mediante las aplicaciones [`agexample`](../../examples/agexample/README.md), [`wifi`](../../examples/wifi/README.md), [`http_server`](../../examples/http_server/README.md) y [`ble`](../../examples/ble/README.md), además de las [variantes de enlace BLE con el SDK](../idf/README.md), y después comprueba el comportamiento de radio, HTTP y GPIO en la placa real.
