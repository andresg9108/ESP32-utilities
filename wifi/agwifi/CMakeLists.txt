idf_component_register(SRCS "agwifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "esp_common"
                    PRIV_REQUIRES "freertos" "esp_wifi" "esp_netif" "esp_event" "log" "nvs_flash")
