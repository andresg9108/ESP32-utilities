set(agble_sources "agBLE_disabled.c")
if(CONFIG_AGBLE_CLIENT_ENABLED OR CONFIG_AGBLE_SERVER_ENABLED)
    list(APPEND agble_sources "agBLE_runtime.c")
endif()
if(CONFIG_AGBLE_CLIENT_ENABLED)
    list(APPEND agble_sources "agBLE.c")
endif()
if(CONFIG_AGBLE_SERVER_ENABLED)
    list(APPEND agble_sources "agBLE_server.c")
endif()

idf_component_register(SRCS ${agble_sources}
                       INCLUDE_DIRS "include"
                       REQUIRES esp_common
                       PRIV_REQUIRES bt freertos)
