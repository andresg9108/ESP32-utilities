```c
#include <string.h>
#include "esp_http_server.h"
#include "agwifi.h"

void app_main(void){
    agwifi_connect_to_wifi_network();

    if(!agwifi_is_wifi_connected()){
        printf("No hay ninguna conexión.\n");
    }else{
        agwifi_print_wifi_network_information();
    }
}
```