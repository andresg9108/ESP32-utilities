# SDK link checks / Comprobaciones de enlace con el SDK

The `ble/` application links every agBLE public function from C++ against the real ESP-IDF SDK. It is a compile/link check, not a functional firmware example or a radio test. Its guarded API calls are not executed. For a runnable BLE application, use [`examples/ble`](../../examples/ble/README.md).

La aplicación `ble/` enlaza todas las funciones públicas de agBLE desde C++ contra el SDK real de ESP-IDF. Es una comprobación de compilación/enlace, no un ejemplo funcional de firmware ni una prueba de radio. Sus llamadas protegidas a la API no se ejecutan. Para una aplicación BLE ejecutable, utiliza [`examples/ble`](../../examples/ble/README.md).

From the repository root, with ESP-IDF 5.4.2 loaded and the `esp32` target, select one of `ble_client.defaults`, `ble_both.defaults` or `ble_disabled.defaults`. Use separate build and sdkconfig paths for each variant so an earlier configuration does not mask the defaults:

Desde la raíz del repositorio, con ESP-IDF 5.4.2 cargado y el target `esp32`, selecciona uno de los archivos `ble_client.defaults`, `ble_both.defaults` o `ble_disabled.defaults`. Utiliza rutas de compilación y sdkconfig independientes para cada variante, de modo que una configuración anterior no oculte los valores predeterminados:

```sh
idf.py -C tests/idf/ble -B "$PWD/tests/idf/ble/build/client" -D SDKCONFIG="$PWD/tests/idf/ble/build/client/sdkconfig" -D SDKCONFIG_DEFAULTS="$PWD/tests/idf/ble_client.defaults" -D IDF_TARGET=esp32 build
```

The runnable example covers server-only compilation. These three fixtures cover client-only, both implementations enabled, and all BLE modes disabled with public stubs returning `ESP_ERR_NOT_SUPPORTED`. The client-only fixture keeps SDK GATT server support enabled to work around ESP-IDF 5.4.2's unconditional `ble_gatts_stop()` reference during teardown; the agBLE server implementation is still excluded. Host tests additionally exercise lifecycle, data and failure behavior against SDK fakes.

El ejemplo ejecutable cubre la compilación de solo servidor. Estas tres configuraciones cubren solo cliente, ambas implementaciones habilitadas y todos los modos BLE deshabilitados con funciones públicas que devuelven `ESP_ERR_NOT_SUPPORTED`. La configuración de solo cliente conserva el soporte de servidor GATT del SDK para resolver la referencia incondicional a `ble_gatts_stop()` durante la limpieza de ESP-IDF 5.4.2; la implementación del servidor agBLE sigue excluida. Las pruebas en el equipo de desarrollo comprueban además el ciclo de vida, los datos y los errores mediante simulaciones del SDK.
