# Component specialist / Especialista en componentes

`esp32-components` is a project-local OpenCode agent for creating, updating and maintaining ESP-IDF components. It inherits your configured model and permissions and can be used as a primary agent or mentioned as `@esp32-components`. Restart OpenCode after adding or changing this configuration.

`esp32-components` es un agente local de OpenCode para crear, actualizar y mantener componentes ESP-IDF. Hereda tu modelo y permisos configurados y puede utilizarse como agente principal o mencionarse como `@esp32-components`. Reinicia OpenCode después de añadir o modificar esta configuración.

```sh
opencode --agent esp32-components
```

The agent uses three project skills: `esp-idf-components` for implementation and lifecycle, `component-examples-docs` for minimal examples and bilingual documentation, and `esp-idf-verification` for builds and meaningful regressions. Creating a component requires its own example and README; functional updates must keep them consistent with the current API and configuration.

El agente utiliza tres skills del proyecto: `esp-idf-components` para implementación y ciclo de vida, `component-examples-docs` para ejemplos mínimos y documentación bilingüe, y `esp-idf-verification` para compilación y regresiones significativas. Crear un componente exige su propio ejemplo y README; las actualizaciones funcionales deben mantenerlos coherentes con la API y configuración actuales.

Example request: “Update agwifi and adapt its minimal example, component README, example README and relevant tests.”

Ejemplo de petición: «Actualiza agwifi y adapta su ejemplo mínimo, el README del componente, el README del ejemplo y las pruebas pertinentes».
