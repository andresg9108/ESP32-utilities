# Repository context

- This is a collection of ESP-IDF C components, distributed as npm package `ag-util-esp32`. npm is only the distribution mechanism: there are no JavaScript entrypoints, dependencies, or npm scripts.
- There is no root ESP-IDF application or root `CMakeLists.txt`. Independent minimal apps live in `examples/agexample/`, `examples/wifi/`, `examples/http_server/`, and `examples/ble/`; HTTP uses `agwifi` for networking. The BLE example adapts `bot/main` to control GPIO 4. `agexample/` itself only exposes `hello()`.
- The component roots are `agexample/`, `wifi/agwifi/`, `httpServer/agHttpServer/`, and `BLE/agBLE/`. Each registers its source, public `include/` directory, and ESP-IDF dependencies in its own `CMakeLists.txt`; update that registration when adding sources or dependencies.

## Integration and verification

- With the ESP-IDF environment loaded, run `idf.py -C examples/http_server set-target esp32`, `idf.py -C examples/http_server menuconfig`, then `idf.py -C examples/http_server build`. Use the other example paths as needed; `agexample` needs no Wi-Fi configuration and `ble` supplies NimBLE defaults. Verify all four apps after integration changes. ESP-IDF 5.4.2 is the verified SDK; the root README also gives a read-only Docker build command.
- Run host regressions with `python3 tests/host/run.py` (Python 3, GCC, G++). They compile production C against SDK fakes with `-Werror` and UBSan, exercise lifecycle/error paths, and link a C++ consumer. They do not verify radio/network behavior.
- The root `README.md` shows `EXTRA_COMPONENT_DIRS` integration: add the component roots directly, or the repository root, `wifi/`, `httpServer/`, and `BLE/` (or their npm equivalents), before including `$ENV{IDF_PATH}/tools/cmake/project.cmake`.
- `wifi/agwifi/Kconfig` owns Wi-Fi settings under Component config → AG Wi-Fi configuration. Keep the existing `ESP_WIFI_*`/`ESP_MAXIMUM_RETRY` setting names for migration; consumers must remove duplicate definitions from `main/Kconfig.projbuild`. The default empty SSID makes connect return `ESP_ERR_INVALID_ARG` until configured.
- HTTP example assets such as `index.html` belong to the consuming app. Register them with `EMBED_FILES` in its component CMake file to provide the `_binary_*_start`/`_binary_*_end` linker symbols used in the HTTP README.

## Runtime contracts

- `agwifi_connect_to_wifi_network()` returns `esp_err_t` and blocks for IP, retry exhaustion or timeout. It owns the Wi-Fi driver but borrows any existing default station interface; deinit preserves shared NVS/netif/event-loop infrastructure. Never erase NVS automatically. Connection queries are safe before initialization.
- `agHttpServer_initialize_http_server()` is idempotent and uses `HTTPD_DEFAULT_CONFIG()`. Callers register URI handlers once using the borrowed handle; stop via `agHttpServer_stop_http_server()` before releasing Wi-Fi. Serialize component API calls, including queries during teardown; blocking lifecycle calls must not run in event/URI handlers. See the component READMEs for ownership and error contracts.
- `agBLE` owns NimBLE exclusively in client or server mode. Init NVS first; never deinit/stop from callbacks. Disabled modes return `ESP_ERR_NOT_SUPPORTED`. IDF 5.4.2 client teardown requires SDK GATT server support even with the agBLE server disabled; use `tests/idf/ble_client.defaults`. See `tests/idf/README.md` for real C++ linkage checks of all modes.

## Component workflow

- Every new component needs an independent minimal example and component/example READMEs. Functional updates must keep them aligned with API/configuration changes. Each English explanatory paragraph in a README must be immediately followed by its Spanish equivalent.
- The local `esp32-components` OpenCode agent and skills in `.opencode/skills/` cover implementation, examples/documentation and verification; see `.opencode/README.md` for invocation.
