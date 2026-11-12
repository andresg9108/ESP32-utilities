# agBLE

An ESP-IDF NimBLE component adapted from the `bot` project's BLE implementation. It provides a central/GATT client and a peripheral/GATT server, with a single owner of the NimBLE host. The modes can be compiled independently, but cannot run at the same time. Public headers support C and C++.

Componente NimBLE de ESP-IDF adaptado de la implementación BLE del proyecto `bot`. Proporciona un central/cliente GATT y un periférico/servidor GATT, con un único propietario del host NimBLE. Los modos pueden compilarse de forma independiente, pero no ejecutarse simultáneamente. Los encabezados públicos son compatibles con C y C++.

## Integration and configuration / Integración y configuración

Add `BLE/agBLE` to `EXTRA_COMPONENT_DIRS` before including ESP-IDF's `project.cmake`, declare `agBLE` in the consuming component's dependencies and include `agBLE.h`. See the [root README](../../README.md) for npm/local integration. The reference SDK is ESP-IDF 5.4.2 on `esp32`; other SDKs and BLE-capable chips require their own validation.

Añade `BLE/agBLE` a `EXTRA_COMPONENT_DIRS` antes de incluir `project.cmake` de ESP-IDF, declara `agBLE` en las dependencias del componente consumidor e incluye `agBLE.h`. Consulta el [README raíz](../../README.md) para la integración mediante npm o una copia local. El SDK de referencia es ESP-IDF 5.4.2 en `esp32`; los demás SDK y chips con BLE necesitan su propia validación.

The application initializes NVS with `nvs_flash_init()` before starting either mode. NVS errors are left to the application; this component never erases or deinitializes NVS. Do not initialize another NimBLE instance or share its host/controller with another library while agBLE owns it.

La aplicación inicializa NVS mediante `nvs_flash_init()` antes de iniciar cualquiera de los modos. Los errores de NVS se dejan a la aplicación; este componente nunca borra ni desinicializa NVS. No inicialices otra instancia NimBLE ni compartas su host/controlador con otra biblioteca mientras agBLE sea su propietario.

Enable Bluetooth and the NimBLE host in menuconfig. Under **Component config → AG BLE configuration**, `CONFIG_AGBLE_CLIENT_ENABLED` requires the central, observer and GATT client roles, plus SDK GATT server support for teardown; `CONFIG_AGBLE_SERVER_ENABLED` requires peripheral, broadcaster and GATT server support. Each mode defaults to enabled when its prerequisites are satisfied. Disabled modes retain their public symbols and return `ESP_ERR_NOT_SUPPORTED`; merely discovering this component does not enable Bluetooth.

Habilita Bluetooth y el host NimBLE en menuconfig. En **Component config → AG BLE configuration**, `CONFIG_AGBLE_CLIENT_ENABLED` requiere los roles central, observador y cliente GATT, además del soporte de servidor GATT del SDK para la limpieza; `CONFIG_AGBLE_SERVER_ENABLED` requiere periférico, emisor de anuncios y servidor GATT. Cada modo se habilita de forma predeterminada cuando se cumplen sus requisitos. Los modos deshabilitados conservan sus símbolos públicos y devuelven `ESP_ERR_NOT_SUPPORTED`; descubrir este componente no habilita Bluetooth por sí solo.

The [BLE example defaults](../../examples/ble/sdkconfig.defaults) enable only server mode and one connection. For central mode, also enable `CONFIG_BT_NIMBLE_ROLE_CENTRAL`, `CONFIG_BT_NIMBLE_ROLE_OBSERVER` and `CONFIG_BT_NIMBLE_GATT_CLIENT`. ESP-IDF 5.4.2 calls `ble_gatts_stop()` unconditionally during connected-host teardown, so retain `CONFIG_BT_NIMBLE_ROLE_PERIPHERAL=y` and `CONFIG_BT_NIMBLE_GATT_SERVER=y` even for client-only use; `CONFIG_AGBLE_SERVER_ENABLED` can remain disabled and no agBLE service is advertised. The [SDK link checks](../../tests/idf/README.md) contain complete configurations for client-only, both modes and Bluetooth disabled.

Los [valores predeterminados del ejemplo BLE](../../examples/ble/sdkconfig.defaults) habilitan únicamente el modo servidor y una conexión. Para el modo central, habilita además `CONFIG_BT_NIMBLE_ROLE_CENTRAL`, `CONFIG_BT_NIMBLE_ROLE_OBSERVER` y `CONFIG_BT_NIMBLE_GATT_CLIENT`. ESP-IDF 5.4.2 llama a `ble_gatts_stop()` incondicionalmente al liberar un host con soporte de conexión, por lo que debes conservar `CONFIG_BT_NIMBLE_ROLE_PERIPHERAL=y` y `CONFIG_BT_NIMBLE_GATT_SERVER=y` incluso para uso de solo cliente; `CONFIG_AGBLE_SERVER_ENABLED` puede permanecer deshabilitado y no se anuncia ningún servicio agBLE. Las [comprobaciones de enlace con el SDK](../../tests/idf/README.md) contienen configuraciones completas de solo cliente, ambos modos y Bluetooth deshabilitado.

## Peripheral/server API / API de periférico/servidor

`agble_server_start(name, callback, context)` copies the name and callback settings and launches the host task. The name must contain 1–20 bytes. Successful return means startup was accepted; wait for `AGBLE_SERVER_ADVERTISING` before connecting. Repeated startup while either mode owns NimBLE returns `ESP_ERR_INVALID_STATE`.

`agble_server_start(name, callback, context)` copia el nombre y la configuración del callback y lanza la tarea del host. El nombre debe contener de 1 a 20 bytes. Un retorno correcto indica que el arranque fue aceptado; espera `AGBLE_SERVER_ADVERTISING` antes de conectar. Repetir el arranque mientras cualquiera de los modos posee NimBLE devuelve `ESP_ERR_INVALID_STATE`.

- Service `0xFFF0` contains command characteristic `0xFFF1` (**Write with response**, 1–512 bytes) and status characteristic `0xFFF2` (**Read + Notify**, 1–20 bytes when published). The actual write transfer must also fit the peer's ATT procedure and negotiated MTU.

  El servicio `0xFFF0` contiene la característica de comandos `0xFFF1` (**escritura con respuesta**, de 1 a 512 bytes) y la de estado `0xFFF2` (**lectura y notificación**, de 1 a 20 bytes al publicar). La transferencia real de escritura también debe ajustarse al procedimiento ATT del cliente y al MTU negociado.
- The callback receives advertising, connected, disconnected, command and error events. `AGBLE_SERVER_COMMAND` provides the complete binary command; it is not guaranteed to be a null-terminated string. The server advertises again after disconnection unless shutdown is in progress.

  El callback recibe eventos de anuncio, conexión, desconexión, comando y error. `AGBLE_SERVER_COMMAND` proporciona el comando binario completo; no se garantiza que sea una cadena terminada en nulo. El servidor vuelve a anunciarse tras desconectar, salvo que esté en proceso de parada.
- `agble_server_publish(data, length)` copies the current status and requests notification of subscribed peers. Read `0xFFF2` once when connecting to obtain the current value, then subscribe for updates. `ESP_OK` means the local update was accepted, not that a remote application received it. Before the first publish, reads return an empty value.

  `agble_server_publish(data, length)` copia el estado actual y solicita la notificación a los clientes suscritos. Lee `0xFFF2` una vez al conectar para obtener el valor actual y después suscríbete a las actualizaciones. `ESP_OK` indica que la actualización local fue aceptada, no que una aplicación remota la haya recibido. Antes de la primera publicación, las lecturas devuelven un valor vacío.
- `agble_server_stop()` stops the host, including advertising and active connections, then releases the task and NimBLE resources. It must run in an application task. It is idempotent after successful cleanup; calling it while client mode owns NimBLE returns `ESP_ERR_INVALID_STATE`.

  `agble_server_stop()` detiene el host, incluidos los anuncios y las conexiones activas, y después libera la tarea y los recursos NimBLE. Debe ejecutarse en una tarea de la aplicación. Es idempotente después de una limpieza correcta; llamarlo mientras el modo cliente posee NimBLE devuelve `ESP_ERR_INVALID_STATE`.

See [`examples/ble`](../../examples/ble/README.md) for the minimal GPIO 4 command/status example adapted from `bot/main`. Application-specific commands and GPIO handling stay in that example, not in the component.

Consulta [`examples/ble`](../../examples/ble/README.md) para ver el ejemplo mínimo de comando/estado de GPIO 4 adaptado de `bot/main`. Los comandos específicos de la aplicación y el control de GPIO permanecen en ese ejemplo, no en el componente.

## Central/client API / API de central/cliente

`agble_init(config)` copies three UUIDs and callback/context settings. All UUIDs must have length 2 or 16 and use Bluetooth little-endian byte order: service `0xFFF0` is `{ .length = 2, .bytes = {0xf0, 0xff} }`. Reverse the usual human-readable byte order for a 128-bit UUID. Initialize one client connection at a time and wait for `AGBLE_STACK_READY` before scanning or connecting.

`agble_init(config)` copia tres UUID y la configuración del callback/contexto. Todos los UUID deben tener longitud 2 o 16 y utilizar el orden little-endian de Bluetooth: el servicio `0xFFF0` es `{ .length = 2, .bytes = {0xf0, 0xff} }`. Invierte el orden habitual de bytes de la representación legible de un UUID de 128 bits. Inicializa una única conexión cliente y espera `AGBLE_STACK_READY` antes de buscar o conectar.

```c
agble_config_t config = {
    .service = { .length = 2, .bytes = {0xf0, 0xff} },
    .write_char = { .length = 2, .bytes = {0xf1, 0xff} },
    .receive_char = { .length = 2, .bytes = {0xf2, 0xff} },
    .callback = on_ble_event,
};
```

This configuration targets the bundled server. Supply your own `on_ble_event(const agble_event_t *, void *)` callback and initialize NVS before calling `agble_init(&config)`. The workflow is asynchronous:

Esta configuración corresponde al servidor incluido. Proporciona tu propio callback `on_ble_event(const agble_event_t *, void *)` e inicializa NVS antes de llamar a `agble_init(&config)`. El flujo es asíncrono:

1. `agble_scan_start(duration_ms)` performs legacy passive scanning: 1–`INT32_MAX` milliseconds, or 0 for continuous scanning. `AGBLE_SCAN_RESULT` contains address, RSSI and raw advertising data; duplicate reports are possible. Save the chosen address, stop scanning and wait for `AGBLE_SCAN_DONE` before connecting. Manual `agble_scan_stop()` reports completion in the calling task.

   `agble_scan_start(duration_ms)` realiza búsqueda pasiva de anuncios legacy: de 1 a `INT32_MAX` milisegundos, o 0 para búsqueda continua. `AGBLE_SCAN_RESULT` contiene dirección, RSSI y datos de anuncio sin procesar; puede haber informes duplicados. Guarda la dirección elegida, detén la búsqueda y espera `AGBLE_SCAN_DONE` antes de conectar. La llamada manual a `agble_scan_stop()` informa de la finalización en la tarea que la ejecuta.
2. `agble_connect(address, timeout_ms)` accepts a copied scan address, type 0 (public) or 1 (random), and a timeout from 1 to `INT32_MAX` milliseconds. `AGBLE_CONNECTED` means only that the link exists. Wait for `AGBLE_READY`: service/characteristic discovery and notification/indication subscription must finish first. A rejected MTU exchange falls back to the default MTU.

   `agble_connect(address, timeout_ms)` acepta una dirección copiada de la búsqueda, tipo 0 (pública) o 1 (aleatoria), y un timeout de 1 a `INT32_MAX` milisegundos. `AGBLE_CONNECTED` solo indica que existe el enlace. Espera `AGBLE_READY`: antes deben terminar el descubrimiento del servicio y las características y la suscripción a notificaciones/indicaciones. Si se rechaza el intercambio de MTU, se utiliza el MTU predeterminado.
3. `agble_write(data, length)` writes with response, copying 1–`min(512, MTU - 3)` bytes before return. The write characteristic must support Write with response. `agble_read()` performs a single ATT read on a readable receive characteristic, at most `MTU - 1` bytes. Only one read/write may be pending; completion arrives as `AGBLE_WRITE_DONE`, `AGBLE_DATA` with `from_read=true`, or an error event.

   `agble_write(data, length)` escribe con respuesta y copia de 1 a `min(512, MTU - 3)` bytes antes de retornar. La característica de escritura debe admitir escritura con respuesta. `agble_read()` realiza una única lectura ATT sobre una característica de recepción legible, de hasta `MTU - 1` bytes. Solo puede haber una lectura/escritura pendiente; su finalización llega como `AGBLE_WRITE_DONE`, `AGBLE_DATA` con `from_read=true`, o un evento de error.
4. Notifications and indications arrive as `AGBLE_DATA` with `from_read=false`. Discovery/subscription errors report their phase and attempt to disconnect. `agble_disconnect()` is asynchronous: await `AGBLE_DISCONNECTED`, or a connect-phase error when cancelling a connection attempt. Disconnection supersedes pending I/O completion.

   Las notificaciones e indicaciones llegan como `AGBLE_DATA` con `from_read=false`. Los errores de descubrimiento/suscripción indican su fase e intentan desconectar. `agble_disconnect()` es asíncrono: espera `AGBLE_DISCONNECTED`, o un error de fase de conexión al cancelar un intento. La desconexión sustituye a cualquier finalización de E/S pendiente.
5. Call `agble_deinit()` from an application task only after scanning and connection activity have ended. Active scanning, connecting, discovery or a live link returns `ESP_ERR_INVALID_STATE`. Successful cleanup permits reinitialization or switching to server mode. Repeated cleanup without an owner returns `ESP_OK`.

   Llama a `agble_deinit()` desde una tarea de la aplicación solo después de terminar la búsqueda y la actividad de conexión. Una búsqueda, conexión, descubrimiento o enlace activo devuelve `ESP_ERR_INVALID_STATE`. La limpieza correcta permite reinicializar o cambiar a modo servidor. Repetir la limpieza cuando no hay propietario devuelve `ESP_OK`.

## Errors, ownership and shutdown / Errores, propiedad y parada

Public calls return `esp_err_t`: argument errors use `ESP_ERR_INVALID_ARG`, invalid lifecycle or ownership uses `ESP_ERR_INVALID_STATE`, oversized MTU writes use `ESP_ERR_INVALID_SIZE`, and unsupported modes/read properties use `ESP_ERR_NOT_SUPPORTED`. Immediate NimBLE scan/connect/read/write failures return `ESP_FAIL`; asynchronous events carry the actual NimBLE/ATT status and phase. Do not pass event status codes to `esp_err_to_name()` as if they were ESP-IDF errors.

Las llamadas públicas devuelven `esp_err_t`: los errores de argumentos utilizan `ESP_ERR_INVALID_ARG`, el ciclo de vida o propiedad inválidos utilizan `ESP_ERR_INVALID_STATE`, las escrituras que superan el MTU utilizan `ESP_ERR_INVALID_SIZE`, y los modos o propiedades de lectura no admitidos utilizan `ESP_ERR_NOT_SUPPORTED`. Los fallos inmediatos NimBLE de búsqueda/conexión/lectura/escritura devuelven `ESP_FAIL`; los eventos asíncronos incluyen el estado real NimBLE/ATT y su fase. No pases los códigos de estado de los eventos a `esp_err_to_name()` como si fueran errores ESP-IDF.

Serialize public API calls and coordinate application-task activity with callbacks. Callbacks normally run in the NimBLE host task; manual scan-stop completion is the exception described above. Never block a callback or call client deinit/server stop from one. Event/data pointers are borrowed until callback return: copy them before queuing or retaining them. The server's status buffer is copied under a lock.

Serializa las llamadas a la API pública y coordina la actividad de la tarea de aplicación con los callbacks. Los callbacks normalmente se ejecutan en la tarea del host NimBLE; la finalización de la parada manual de búsqueda es la excepción indicada antes. Nunca bloquees un callback ni llames desde él a la desinicialización del cliente o la parada del servidor. Los punteros de eventos/datos son prestados hasta terminar el callback: cópialos antes de encolarlos o conservarlos. El buffer de estado del servidor se copia bajo un bloqueo.

Shutdown first asks NimBLE to stop, then waits up to `CONFIG_AGBLE_STOP_TIMEOUT_MS` (default 5000 ms) for the host task to exit. This limit covers the task-exit wait, **not** a hard deadline for the SDK's own blocking `nimble_port_stop()`. A failed SDK stop or task-exit timeout keeps ownership and allows a later stop/deinit retry; operations remain disabled during cleanup.

La parada primero solicita a NimBLE que se detenga y después espera hasta `CONFIG_AGBLE_STOP_TIMEOUT_MS` (5000 ms de forma predeterminada) para que termine la tarea del host. Este límite cubre la espera de finalización de la tarea, **no** un plazo máximo estricto para la propia llamada bloqueante `nimble_port_stop()` del SDK. Un fallo de parada del SDK o un timeout al esperar la tarea conserva la propiedad y permite reintentar después stop/deinit; las operaciones permanecen deshabilitadas durante la limpieza.

If `nimble_port_deinit()` fails, SDK cleanup may already be partial. agBLE retains ownership and returns that cached error on subsequent cleanup calls instead of repeating an unsafe SDK deinit or allowing a new owner. Reboot to recover from that terminal cleanup failure. Initialization failures otherwise release resources when cleanup succeeds.

Si falla `nimble_port_deinit()`, la limpieza del SDK ya puede ser parcial. agBLE conserva la propiedad y devuelve ese error almacenado en las siguientes llamadas de limpieza, en lugar de repetir una desinicialización insegura del SDK o permitir un nuevo propietario. Reinicia para recuperarte de ese fallo terminal de limpieza. Los demás fallos de inicialización liberan los recursos cuando la limpieza tiene éxito.

No pairing or encryption is initiated. Access to protected remote characteristics can fail. This is BLE GATT, not Bluetooth Classic serial. See [host regressions](../../tests/host/README.md) and [SDK link checks](../../tests/idf/README.md) for verified behavior and build variants; actual radio/GPIO operation still requires a board and peer.

No se inicia emparejamiento ni cifrado. El acceso a características remotas protegidas puede fallar. Esto es BLE GATT, no un puerto serie Bluetooth Classic. Consulta las [regresiones en el equipo de desarrollo](../../tests/host/README.md) y las [comprobaciones de enlace con el SDK](../../tests/idf/README.md) para el comportamiento verificado y las variantes de compilación; el funcionamiento real de radio/GPIO sigue necesitando una placa y un cliente.
