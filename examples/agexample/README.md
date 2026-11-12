# agexample example / Ejemplo de agexample

A minimal ESP-IDF application that calls `hello()` and prints `Hello, ESP32!` once. It needs no Wi-Fi configuration.

Aplicación mínima de ESP-IDF que llama a `hello()` e imprime el saludo `Hello, ESP32!` («¡Hola, ESP32!») una sola vez. No necesita configuración Wi-Fi.

## Build and run / Compilar y ejecutar

Activate the ESP-IDF environment and run these commands from the repository root. Replace `/dev/ttyUSB0` with your board's port; exit the monitor with `Ctrl+]`.

Activa el entorno ESP-IDF y ejecuta estos comandos desde la raíz del repositorio. Sustituye `/dev/ttyUSB0` por el puerto de tu placa; sal del monitor con `Ctrl+]`.

```sh
idf.py -C examples/agexample set-target esp32
idf.py -C examples/agexample build
idf.py -C examples/agexample -p /dev/ttyUSB0 flash monitor
```

The application uses only the `agexample` component. See [`main/main.c`](main/main.c) for the complete example and the [component README](../../agexample/README.md) for integration details.

La aplicación utiliza únicamente el componente `agexample`. Consulta [`main/main.c`](main/main.c) para ver el ejemplo completo y el [README del componente](../../agexample/README.md) para los detalles de integración.
