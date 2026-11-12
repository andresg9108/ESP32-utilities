# BLE example / Ejemplo BLE

A small adaptation of `bot/main`: advertise as `agBLE`, receive a one-byte command, set GPIO 4, and publish its state. Wi-Fi is not used. The reusable component contains no GPIO or robot-specific logic.

Adaptación sencilla de `bot/main`: anuncia el dispositivo como `agBLE`, recibe un comando de un byte, configura GPIO 4 y publica su estado. No utiliza Wi-Fi. El componente reutilizable no contiene lógica de GPIO ni del robot.

## Build and run / Compilar y ejecutar

Activate ESP-IDF and run these commands from the repository root. The supplied `sdkconfig.defaults` enables NimBLE peripheral/server mode. Replace the serial port with your board's port; exit the monitor with `Ctrl+]`.

Activa ESP-IDF y ejecuta estos comandos desde la raíz del repositorio. El archivo `sdkconfig.defaults` incluido habilita el modo periférico/servidor NimBLE. Sustituye el puerto serie por el de tu placa; sal del monitor con `Ctrl+]`.

```sh
idf.py -C examples/ble set-target esp32
idf.py -C examples/ble build
idf.py -C examples/ble -p /dev/ttyUSB0 flash monitor
```

GPIO 4 starts low. Use a board where that pin is available, or change `GPIO_NUM_4` in [`main/main.c`](main/main.c) to the output you want to control. No external device is required to inspect the serial log or read the published value.

GPIO 4 comienza a nivel bajo. Utiliza una placa donde ese pin esté disponible, o cambia `GPIO_NUM_4` en [`main/main.c`](main/main.c) por la salida que quieras controlar. No necesitas un dispositivo externo conectado al GPIO para consultar el registro serie o leer el valor publicado.

## Try it from a phone or PC / Probar desde un teléfono o PC

Use a BLE GATT client, such as nRF Connect, to scan and connect to `agBLE`. Open service `0xFFF0`. Enable notifications on status characteristic `0xFFF2` and read it once to get the initial ASCII value `0`.

Utiliza un cliente BLE GATT, como nRF Connect, para buscar y conectar con `agBLE`. Abre el servicio `0xFFF0`. Habilita las notificaciones de la característica de estado `0xFFF2` y léela una vez para obtener el valor ASCII inicial `0`.

Write **with response** to command characteristic `0xFFF1`: send ASCII `1` (`0x31`) to set GPIO 4 high, or ASCII `0` (`0x30`) to set it low. Binary `0x01` and `0x00` are also accepted. Send exactly one byte, without a newline. Other commands are ignored. The status is always published as ASCII `0` or `1`.

Escribe **con respuesta** en la característica de comandos `0xFFF1`: envía ASCII `1` (`0x31`) para poner GPIO 4 a nivel alto, o ASCII `0` (`0x30`) para ponerlo a nivel bajo. También se aceptan los bytes binarios `0x01` y `0x00`. Envía exactamente un byte, sin salto de línea. Los demás comandos se ignoran. El estado siempre se publica como ASCII `0` o `1`.

The server keeps running after `app_main()` returns and advertises again after disconnection. No pairing is initiated. Initialization errors stop this minimal example through `ESP_ERROR_CHECK()`; command-time errors are logged. See the [component README](../../BLE/agBLE/README.md) for the full API, central mode and shutdown rules.

El servidor permanece activo después de terminar `app_main()` y vuelve a anunciarse tras una desconexión. No se inicia emparejamiento. Los errores de inicialización detienen este ejemplo mínimo mediante `ESP_ERROR_CHECK()`; los errores al procesar comandos se registran. Consulta el [README del componente](../../BLE/agBLE/README.md) para la API completa, el modo central y las reglas de parada.
