#include "agwifi.h"

void app_main(void)
{
    ESP_ERROR_CHECK(agwifi_connect_to_wifi_network());
    agwifi_print_wifi_network_information();
}
