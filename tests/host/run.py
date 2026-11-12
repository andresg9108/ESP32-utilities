#!/usr/bin/env python3
"""Compile production C against deterministic SDK fakes and check C++ linkage.
Compila el código C de producción con simulaciones deterministas del SDK y verifica el enlace con C++.
"""

import os
from pathlib import Path
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
CC = os.environ.get("CC", "gcc")
CXX = os.environ.get("CXX", "g++")


def run(*args):
    subprocess.run([str(arg) for arg in args], check=True)


with tempfile.TemporaryDirectory(prefix="ag-utilities-test-") as directory:
    build = Path(directory)
    for name in ("sdkconfig.h", "esp_err.h", "esp_event.h", "esp_log.h",
                 "esp_netif.h", "esp_wifi.h", "nvs_flash.h", "esp_http_server.h",
                 "freertos/FreeRTOS.h", "freertos/event_groups.h"):
        header = build / name
        header.parent.mkdir(parents=True, exist_ok=True)
        header.write_text('#include "fake_idf.h"\n')

    includes = [f"-I{path}" for path in (
        build, HERE, ROOT / "agexample/include", ROOT / "wifi/agwifi/include",
        ROOT / "httpServer/agHttpServer/include", ROOT / "BLE/agBLE/include")]
    flags = ["-Wall", "-Wextra", "-Werror", "-g", "-fsanitize=undefined",
             "-fno-sanitize-recover=undefined"]
    objects = []
    for source in (ROOT / "agexample/agexample.c", ROOT / "wifi/agwifi/agwifi.c",
                   ROOT / "httpServer/agHttpServer/agHttpServer.c",
                   ROOT / "BLE/agBLE/agBLE_disabled.c"):
        target = build / (source.stem + ".o")
        run(CC, "-std=c11", *flags, *includes, "-c", source, "-o", target)
        objects.append(target)

    test = build / "test-components"
    run(CC, "-std=c11", *flags, *includes, HERE / "test_components.c", *objects, "-o", test)
    run(test)

    empty = build / "test-empty-ssid"
    run(CC, "-std=c11", *flags, *includes, '-DCONFIG_ESP_WIFI_SSID=""',
        HERE / "test_components.c", ROOT / "wifi/agwifi/agwifi.c",
        ROOT / "httpServer/agHttpServer/agHttpServer.c", "-o", empty)
    run(empty)

    # Reuse SDK fakes without the C test entrypoint for a C++ consumer link check.
    # Reutiliza las simulaciones del SDK sin el punto de entrada de la prueba C para verificar el enlace de un consumidor C++.
    fakes = build / "fakes.o"
    run(CC, "-std=c11", *flags, *includes, "-Dmain=fake_test_main", "-c",
        HERE / "test_components.c", "-o", fakes)
    run(CXX, "-std=c++11", *flags, *includes, HERE / "headers.cpp", *objects,
        fakes, "-o", build / "cpp-consumer")
    print("C++ public-header inclusion and C linkage passed", flush=True)

    ble_build = build / "ble"
    for name in ("sdkconfig.h", "freertos/FreeRTOS.h", "freertos/semphr.h",
                 "freertos/task.h", "host/ble_hs.h", "host/ble_gatt.h",
                 "host/ble_gap.h", "host/ble_att.h", "host/ble_uuid.h",
                 "host/util/util.h", "nimble/nimble_port.h", "os/os_mbuf.h",
                 "services/gap/ble_svc_gap.h", "services/gatt/ble_svc_gatt.h"):
        header = ble_build / name
        header.parent.mkdir(parents=True, exist_ok=True)
        header.write_text('#include "fake_nimble.h"\n')
    ble_includes = [f"-I{ble_build}", f"-I{HERE / 'ble'}", *includes]
    ble_test = ble_build / "test-ble"
    run(CC, "-std=gnu11", *flags, *ble_includes,
        HERE / "ble/test_ble.c", ROOT / "BLE/agBLE/agBLE_runtime.c",
        ROOT / "BLE/agBLE/agBLE.c", ROOT / "BLE/agBLE/agBLE_server.c", "-o", ble_test)
    run(ble_test)
    for case in ("client", "server", "startup-client", "startup-server"):
        run(ble_test, case)
    disabled_test = ble_build / "test-disabled"
    run(CC, "-std=c11", *flags, *includes, HERE / "ble/test_disabled.c",
        ROOT / "BLE/agBLE/agBLE_disabled.c", "-o", disabled_test)
    run(disabled_test)
