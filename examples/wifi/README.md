# Wi-Fi example / Ejemplo de Wi-Fi

A minimal ESP-IDF application that connects with `agwifi` and prints the IP address, gateway and netmask. Wi-Fi stays active after `app_main()` returns.

Aplicación mínima de ESP-IDF que conecta mediante `agwifi` y muestra la dirección IP, la puerta de enlace y la máscara de red. Wi-Fi permanece activo después de terminar `app_main()`.

## Configure, build and run / Configurar, compilar y ejecutar

Activate the ESP-IDF environment and run these commands from the repository root. In menuconfig, enter your SSID and password under **Component config → AG Wi-Fi configuration**. Replace `/dev/ttyUSB0` with your board's port; exit the monitor with `Ctrl+]`.

Activa el entorno ESP-IDF y ejecuta estos comandos desde la raíz del repositorio. En menuconfig, introduce tu SSID y contraseña en **Component config → AG Wi-Fi configuration**. Sustituye `/dev/ttyUSB0` por el puerto de tu placa; sal del monitor con `Ctrl+]`.

```sh
idf.py -C examples/wifi set-target esp32
idf.py -C examples/wifi menuconfig
idf.py -C examples/wifi build
idf.py -C examples/wifi -p /dev/ttyUSB0 flash monitor
```

The default SSID is empty. This example uses `ESP_ERROR_CHECK()` to stop on an error; it does not add application-level retries or cleanup logic. For recoverable errors and resource ownership, see the [component README](../../wifi/agwifi/README.md). The complete application is in [`main/main.c`](main/main.c).

El SSID predeterminado está vacío. Este ejemplo utiliza `ESP_ERROR_CHECK()` para detenerse ante un error; no añade reintentos a nivel de aplicación ni lógica de liberación de recursos. Para errores recuperables y propiedad de los recursos, consulta el [README del componente](../../wifi/agwifi/README.md). La aplicación completa está en [`main/main.c`](main/main.c).
