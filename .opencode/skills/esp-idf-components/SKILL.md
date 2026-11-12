---
name: esp-idf-components
description: Crear, importar, actualizar o revisar componentes ESP-IDF C de ag-util-esp32, sus APIs, CMake, Kconfig, propiedad de recursos y ciclo de vida.
---

# Implementación de componentes ESP-IDF

## Fuentes de verdad

Lee `AGENTS.md` y la implementación, encabezados, CMake, Kconfig, README y ejemplo del componente afectado. Los README de `wifi/agwifi` y `httpServer/agHttpServer` muestran cómo este proyecto documenta retornos, handles prestados, tareas y limpieza; `agexample` muestra el mínimo necesario. Los contratos de BLE se consultan en `BLE/agBLE` cuando corresponda. Resuelve discrepancias a favor del código y del SDK, y corrige la documentación afectada.

## Diseño e integración

1. Delimita la API y el comportamiento solicitado antes de elegir recursos o callbacks. Conserva nombres existentes salvo una razón concreta de incompatibilidad y documenta la migración.
2. Ubica el componente en la familia adecuada: `wifi/`, `httpServer/`, `BLE/` o el lugar que determine la estructura vigente. No conviertas la raíz del paquete npm en una aplicación ni añadas scripts JavaScript para compilar firmware.
3. Registra todas las fuentes en su `idf_component_register()`. Expón `include/`; utiliza `REQUIRES` para dependencias visibles en encabezados y `PRIV_REQUIRES` para las internas. Declara las dependencias directas, no dependas por accidente de includes transitivos.
4. Encabezados C/C++: guardas de inclusión, tipos autocontenidos, `extern "C"` y comentarios del contrato. Estado interno privado con `static`; evita símbolos globales genéricos.
5. Si necesita opciones, suministra Kconfig dentro del componente, con nombres propios para opciones nuevas, valores y rangos comprobados. Conserva los nombres históricos de Wi-Fi para migración. No copies las mismas definiciones en `main/Kconfig.projbuild` de los ejemplos.
6. Habilita solo lo necesario. Las opciones de modo y dependencias deben permitir que las aplicaciones existentes compilen sin activar protocolos que no usan. Comprueba las declaraciones de ESP-IDF/NimBLE antes de asumir firmas, retorno, contexto o disponibilidad.

## Ciclo de vida y errores

- Identifica recursos propios, prestados y compartidos: NVS, event loop, interfaces, driver Wi-Fi, NimBLE, tareas, buffers y handles HTTP.
- Valida argumentos antes de reservar recursos. Define consultas antes de inicializar, llamadas repetidas, espera de disponibilidad y secuencia de parada. No prometas idempotencia donde no existe.
- Propaga errores recuperables con `esp_err_t`; los estados asíncronos pueden notificarse mediante eventos con su fase y código del SDK. Explica la diferencia entre petición aceptada y operación terminada.
- En fallos parciales, conserva información suficiente para no liberar dos veces, usar handles inválidos ni transferir propiedad indebidamente. Solo reintenta una operación si el SDK permite repetirla con seguridad.
- Detén usuarios de un recurso antes de liberarlo; elimina callbacks antes de sus buffers/grupos de eventos y confirma la terminación de tareas antes de destruir sus recursos.
- Declara la política de concurrencia y ejecútala de forma consistente. No mezcles un contrato de llamadas serializadas con acceso concurrente no protegido desde callbacks. Explica cuánto viven los datos prestados y cuándo el consumidor debe copiarlos.
- No borres NVS ni liberes infraestructura compartida como recuperación automática. No imprimas credenciales.

## Importación

Comprueba las instrucciones y la licencia del origen. Adapta a los contratos y SDK de este repositorio sin modificar la aplicación de origen. Mantén el comportamiento específico de GPIO, robot o contenido web en el ejemplo. Conserva la atribución y comprueba tanto el modo utilizado por el ejemplo como los demás modos públicos que se importan.

## Entrega asociada

Carga `component-examples-docs`: toda creación o actualización funcional exige crear o actualizar el ejemplo correspondiente y su documentación. Carga `esp-idf-verification` para seleccionar y ejecutar comprobaciones reales. No cierres el trabajo dejando esos pasos como una sugerencia para después.
