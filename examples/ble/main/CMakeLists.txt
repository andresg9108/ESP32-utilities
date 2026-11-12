idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES agBLE esp_driver_gpio nvs_flash log)
