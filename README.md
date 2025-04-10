# AG utilities ESP32


Example of creating a component.
Ejemplo de creación de un componente.
~~~
idf.py create-component agexample
~~~

Example of including components in the CMakeLists.txt file at the root of an ESP32 project.
Ejemplo de inclusión de componentes en el archivo CMakeLists.txt en la raíz de un proyecto ESP32.
~~~
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS 
    ../ESP32-utilities
    ../ESP32-utilities/wifi
    ../ESP32-utilities/httpServer
    ${CMAKE_SOURCE_DIR}/node_modules/ag-util-esp32
    ${CMAKE_SOURCE_DIR}/node_modules/ag-util-esp32/wifi
    ${CMAKE_SOURCE_DIR}/node_modules/ag-util-esp32/httpServer
    ${CMAKE_SOURCE_DIR}/components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(example)
~~~

Example of creating variables for the menuconfig of an ESP32 project.
Ejemplo de creación de variables para el menuconfig de un proyecto ESP32
~~~
# main/Kconfig.projbuild

menu "Mi primer menu"

config USER_NAME
    string "Nombre de usuario"
    default "Andrés"
    help
        Nombre de usuario de pruebas para el código

config MY_EXAMPLE
    string "Opción de ejemplo #2"
    default "Otra"
    help
        Esta es otra opción de ejemplo para el código

endmenu

menu "Otro menu"

config MENU2_TEST_1
    string "Test #1"
    default "uno"
    help
        Esta es la primera del ejemplo #2

config MENU2_TEST_2
    string "Test #2"
    default "Dos"
    help
        Esta es la segunda del ejemplo #2

endmenu
~~~

Example of using variables created in the menuconfig.
Ejemplo de uso de variables creadas en el menuconfig.
```c
printf("Test #1: %s\n", CONFIG_USER_NAME);
printf("Test #2: %s\n", CONFIG_MY_EXAMPLE);
printf("Test #3: %s\n", CONFIG_MENU2_TEST_1);
printf("Test #4: %s\n", CONFIG_MENU2_TEST_2);
```