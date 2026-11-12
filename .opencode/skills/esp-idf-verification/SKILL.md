---
name: esp-idf-verification
description: Verificar cambios de componentes ESP-IDF con compilación real de ejemplos, pruebas host de errores y ciclo de vida, variantes Kconfig y compatibilidad C++, separando simulación de hardware.
---

# Verificación de componentes

## Selección

Lee `AGENTS.md`, `tests/host/README.md`, `tests/host/run.py` y el README del ejemplo afectado. Determina qué parte cambió: documentación, API/implementación, configuración por modos, integración o hardware. Usa los comandos y requisitos existentes, no supongas que hay un build raíz ni scripts npm.

- Solo documentación: contrasta snippets, opciones, enlaces y comandos con los archivos reales y comprueba `git diff --check`. No repitas una compilación completa sin un cambio que lo justifique.
- Código/API: compila el ejemplo afectado y ejecuta regresiones pertinentes. Verifica el consumidor C++ si cambias encabezados o símbolos públicos.
- Configuración por modos: comprueba los modos habilitados y deshabilitados pertinentes, sus fuentes y dependencias. Compilar el ejemplo servidor no verifica por sí solo la implementación del cliente.
- Integración compartida, rutas o CMake: compila todas las aplicaciones de `examples/` afectadas por el cambio; actualmente existen `agexample`, `wifi`, `http_server` y `ble`. Reconsulta el árbol si se añaden otras.

## Compilación con SDK

ESP-IDF 5.4.2 y target `esp32` son la referencia actual. Con el entorno activado, desde la raíz:

```sh
idf.py -C examples/http_server set-target esp32
idf.py -C examples/http_server menuconfig
idf.py -C examples/http_server build
```

Sustituye la ruta por el ejemplo concreto. `agexample` no necesita Wi-Fi; Wi-Fi/HTTP requieren credenciales para ejecutarse, aunque compilan con SSID vacío. `examples/ble/sdkconfig.defaults` configura NimBLE para el ejemplo; revisa las variantes si cambias sus modos.

Sin SDK local, usa Docker con la fuente de solo lectura y todos los archivos generados en el contenedor:

```sh
docker run --rm -v "$PWD:/project:ro" -w /project/examples/http_server espressif/idf:v5.4.2 idf.py -B /tmp/ag-build -D SDKCONFIG=/tmp/ag-sdkconfig build
```

Adapta el directorio de trabajo a cada ejemplo. No publiques ni sobrescribas credenciales, el firmware de una placa ni la configuración de otra aplicación para verificar una compilación.

## Regresiones host

```sh
python3 tests/host/run.py
```

El runner requiere Python 3, GCC y G++, compila producción contra simulaciones del SDK con `-Werror` y UBSan y comprueba enlace C++. Revisa su implementación antes de atribuirle cobertura de un componente nuevo: si no lo compila ni lo ejercita, no lo está probando.

Añade pruebas significativas cuando modificas comportamiento de riesgo: argumentos y estado previo a init, reintentos, timeout, eventos tardíos, propiedad de recursos, llamadas repetidas, errores de asignación/inicialización/parada y limpieza parcial. No añadas pruebas que solo repliquen una asignación trivial o creen una segunda implementación de la lógica probada. Si los fakes no pueden representar un contrato del SDK, complementa con compilación o prueba de integración real y explica el límite.

## Resultado

Comprueba el diff final y los enlaces a ejemplos/documentación. Informa comandos realmente ejecutados, resultado y modos/SDK/target comprobados. No declares aprobada una prueba que no ejecutaste. Una compilación no prueba la radio, una simulación no prueba FreeRTOS real y un enlace C++ no significa que se haya ejecutado ese consumidor. Si faltan placa o servicios, especifica la validación de hardware pendiente sin inventar resultados. Cuando las comprobaciones pertinentes pasan, no repitas baterías sin nuevos cambios o motivos.
