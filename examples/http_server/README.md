# HTTP server example / Ejemplo de servidor HTTP

A minimal ESP-IDF application that uses `agwifi` for networking and `agHttpServer` to serve a greeting at `GET /` on port 80. The response is included directly in the C code; no HTML files or additional routes are needed.

Aplicación mínima de ESP-IDF que utiliza `agwifi` para la conexión de red y `agHttpServer` para servir un saludo en `GET /` en el puerto 80. La respuesta se incluye directamente en el código C; no necesita archivos HTML ni rutas adicionales.

## Configure, build and run / Configurar, compilar y ejecutar

Activate the ESP-IDF environment and run these commands from the repository root. In menuconfig, enter your SSID and password under **Component config → AG Wi-Fi configuration**. Replace `/dev/ttyUSB0` with your board's port; exit the monitor with `Ctrl+]`.

Activa el entorno ESP-IDF y ejecuta estos comandos desde la raíz del repositorio. En menuconfig, introduce tu SSID y contraseña en **Component config → AG Wi-Fi configuration**. Sustituye `/dev/ttyUSB0` por el puerto de tu placa; sal del monitor con `Ctrl+]`.

```sh
idf.py -C examples/http_server set-target esp32
idf.py -C examples/http_server menuconfig
idf.py -C examples/http_server build
idf.py -C examples/http_server -p /dev/ttyUSB0 flash monitor
```

Open `http://<IP>/` using the IP printed on the serial console, from a device that can reach the ESP32. The server stays running after `app_main()` returns. The default SSID is empty, and `ESP_ERROR_CHECK()` stops the example if connection, server startup or route registration fails.

Abre `http://<IP>/` utilizando la IP mostrada en la consola serie, desde un dispositivo que pueda acceder al ESP32. El servidor permanece activo después de terminar `app_main()`. El SSID predeterminado está vacío, y `ESP_ERROR_CHECK()` detiene el ejemplo si falla la conexión, el arranque del servidor o el registro de la ruta.

See [`main/main.c`](main/main.c) for the complete application. For shutdown, embedded files and lifecycle details, see the [HTTP component README](../../httpServer/agHttpServer/README.md); for network settings, see the [Wi-Fi component README](../../wifi/agwifi/README.md).

Consulta [`main/main.c`](main/main.c) para ver la aplicación completa. Para la parada, los archivos embebidos y los detalles del ciclo de vida, consulta el [README del componente HTTP](../../httpServer/agHttpServer/README.md); para la configuración de red, consulta el [README del componente Wi-Fi](../../wifi/agwifi/README.md).
