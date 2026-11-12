---
name: component-examples-docs
description: Crear o actualizar ejemplos ESP-IDF mínimos y README bilingües inglés-español junto con cambios en componentes, configuración, APIs o integración de este proyecto.
---

# Ejemplos y documentación de componentes

## Obligación de acompañamiento

- Componente nuevo: crea su README y `examples/<nombre>/` con `CMakeLists.txt`, `main/CMakeLists.txt`, `main/<archivo>.c` y README propios. Añade `sdkconfig.defaults` únicamente si necesita opciones para compilar o funcionar, como NimBLE.
- Componente actualizado: revisa el ejemplo y el README del componente y del ejemplo. Adapta las llamadas, dependencias, configuración, instrucciones, respuesta esperada y manejo de errores cuando el cambio los afecte. No mantengas snippets de una API eliminada.
- Refactor sin cambios observables: confirma que siguen vigentes; evita cambios cosméticos forzados. Nunca uses esta excepción para omitir la documentación de una API o requisito nuevo.

## Ejemplos mínimos

Usa `examples/agexample`, `examples/wifi` y `examples/http_server` como referencia de sencillez. Consulta `examples/ble` si el uso requiere BLE/GPIO. El usuario ha pedido expresamente ejemplos lo más simples posible.

1. Cada ejemplo es una aplicación independiente con `app_main()`, sin depender de otro ejemplo ni de un proyecto externo.
2. Registra solo los componentes que usa en `EXTRA_COMPONENT_DIRS` antes de incluir `$ENV{IDF_PATH}/tools/cmake/project.cmake`; declara también sus dependencias directas en el CMake de `main`.
3. Muestra un caso principal de uso. Evita frameworks de ejemplo, menús, temporizadores, tareas auxiliares, rutas extra o HTML embebido cuando una respuesta directa sea suficiente.
4. `ESP_ERROR_CHECK()` está permitido para mantener una demostración mínima, con su comportamiento de fallo explicado. No lo uses para ocultar una operación asíncrona cuya disponibilidad todavía deba esperarse.
5. Si el usuario aporta una aplicación de referencia, conserva sus comandos y resultados relevantes, simplifica lo accesorio y explica los pines, UUID, formato de bytes o requisitos que el usuario debe conocer. Esa lógica pertenece al ejemplo.
6. Configura datos necesarios mediante menuconfig/defaults apropiados. No incluyas credenciales reales ni fuerces configuración del usuario. No dupliques símbolos Kconfig de componentes en el ejemplo.
7. Comprueba las exclusiones de archivos generados de `examples/.gitignore` y `.gitignore`. No subas `build/`, `sdkconfig` con credenciales ni binarios.

## Regla lingüística

En **todos los README creados o modificados**, cada párrafo o elemento explicativo en inglés debe ir inmediatamente seguido de un párrafo equivalente en español. Mantén el mismo detalle y las mismas advertencias en ambos idiomas, en lugar de resumir o añadir información solo en uno. Usa títulos como `## Configuration / Configuración`.

Los bloques de código se comparten entre ambos idiomas. No traduzcas nombres de funciones, rutas, flags, símbolos Kconfig ni salida literal del programa. Si añades comentarios explicativos en inglés dentro de snippets, acompáñalos de español o evita comentarios innecesarios. Las instrucciones internas de agentes/skills pueden escribirse íntegramente en español.

## README del componente

Incluye solo secciones pertinentes, siguiendo los README actuales:

- Propósito, alcance y dependencias reales.
- Integración CMake: ubicación, nombre del componente y encabezado público.
- Opciones y requisitos: valores predeterminados, límites y significado; SDK/chip realmente comprobados.
- Snippet mínimo compatible con la API actual.
- Funciones públicas, retornos, eventos y diferencia entre inicio y disponibilidad.
- Propiedad de recursos y datos, contexto de callbacks, llamadas bloqueantes, concurrencia y secuencia de parada.
- Casos de fallo, limpieza parcial, restricciones y migración si cambia el contrato.
- Enlace relativo al ejemplo y a las verificaciones existentes.

No conviertas un README sencillo en un manual exhaustivo si el componente solo necesita unas líneas. No afirmes compatibilidad o garantías que no estén verificadas.

## README del ejemplo

Explica qué hace, qué placa/configuración necesita, desde qué directorio se ejecutan los comandos, cómo compilar/grabar/abrir el monitor y qué resultado observar. Para red: credenciales/ruta de menuconfig e IP/puerto o nombre BLE/UUID y formato del comando. Para GPIO: pin y valor inicial. Explica si termina, sigue activo o aborta ante error. Enlaza al README del componente para los contratos avanzados.

## Referencias compartidas

Actualiza el índice y las rutas locales/npm de integración del README raíz al añadir componentes o ejemplos. Si renombras o eliminas un ejemplo, busca sus referencias en todo el proyecto, incluido `AGENTS.md` y los README de pruebas; retira las rutas antiguas. Verifica todos los enlaces relativos contra archivos existentes y mantén los comandos de build coherentes con la aplicación concreta.
