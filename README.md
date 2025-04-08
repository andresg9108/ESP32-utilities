# ESP32-utilities

~~~
idf.py create-component agexample
~~~

~~~
set(EXTRA_COMPONENT_DIRS ../ESP32-utilities)
~~~

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

```c
printf("Test #1: %s\n", CONFIG_USER_NAME);
printf("Test #2: %s\n", CONFIG_MY_EXAMPLE);
printf("Test #3: %s\n", CONFIG_MENU2_TEST_1);
printf("Test #4: %s\n", CONFIG_MENU2_TEST_2);
```