idf_component_register(SRCS "agwifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "freertos" "lwip" "esp_wifi" "esp_system" "esp_event" "log" "nvs_flash")
