# agexample

A minimal ESP-IDF component exposing `hello()`. It demonstrates source registration, a public header and linkage from C or C++; it is not a standalone firmware application and has no `app_main()` of its own.

Componente mínimo de ESP-IDF que expone `hello()`. Demuestra el registro de código fuente, un encabezado público y el enlace desde C o C++; no es una aplicación de firmware independiente ni tiene un `app_main()` propio.

## Integration / Integración

Add `agexample/` to the application's `EXTRA_COMPONENT_DIRS` before including ESP-IDF's `project.cmake`. Then declare `agexample` in the consuming component's `REQUIRES`. The [root README](../README.md) shows complete integration through a local checkout or npm.

Añade `agexample/` a `EXTRA_COMPONENT_DIRS` de la aplicación antes de incluir `project.cmake` de ESP-IDF. Después declara `agexample` en `REQUIRES` del componente consumidor. El [README raíz](../README.md) muestra la integración completa mediante una copia local o npm.

```cmake
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES agexample)
```

## Usage / Uso

Call `hello()` to print `Hello, ESP32!` followed by a newline to standard output. It returns `void`, has no configuration or initialization step, and does not access GPIO or networking.

Llama a `hello()` para imprimir el saludo `Hello, ESP32!` («¡Hola, ESP32!») seguido de un salto de línea en la salida estándar. Devuelve `void`, no tiene configuración ni necesita inicialización, y no accede a GPIO ni a la red.

```c
#include "agexample.h"

void app_main(void)
{
    hello();
}
```

For C++, include the same header; its `extern "C"` declarations allow calls into the compiled C component. An application's C++ entrypoint must also use C linkage: `extern "C" void app_main(void)`.

Para C++, incluye el mismo encabezado; sus declaraciones `extern "C"` permiten llamar al componente compilado en C. El punto de entrada de una aplicación en C++ también debe utilizar enlace C: `extern "C" void app_main(void)`.

## Verification / Verificación

The [minimal application](../examples/agexample/README.md) calls `hello()` without networking or other reusable components. The [host runner](../tests/host/README.md) compiles this component and links it into a C++ consumer to verify header/link compatibility; that C++ consumer is not executed.

La [aplicación mínima](../examples/agexample/README.md) llama a `hello()` sin conexión de red ni otros componentes reutilizables. El [ejecutor de pruebas](../tests/host/README.md) compila este componente y lo enlaza con un consumidor C++ para verificar la compatibilidad de encabezados y enlace; ese consumidor C++ no se ejecuta.
