#include "agexample.h"
#include "agexample.h"
#include "agwifi.h"
#include "agwifi.h"
#include "agHttpServer.h"
#include "agHttpServer.h"
#include "agBLE.h"
#include "agBLE.h"

int main()
{
    hello();
    (void)agwifi_connect_to_wifi_network();
    (void)agwifi_is_wifi_connected();
    (void)agwifi_disconnect_from_wifi_network();
    (void)agwifi_deinitialize();
    agwifi_print_wifi_network_information();
    (void)agHttpServer_get_http_server();
    (void)agHttpServer_initialize_http_server();
    (void)agHttpServer_stop_http_server();
    (void)agble_init(nullptr);
    (void)agble_scan_start(1000);
    (void)agble_scan_stop();
    (void)agble_connect(nullptr, 1000);
    (void)agble_disconnect();
    (void)agble_write(nullptr, 0);
    (void)agble_read();
    (void)agble_deinit();
    (void)agble_server_start(nullptr, nullptr, nullptr);
    (void)agble_server_publish(nullptr, 0);
    (void)agble_server_stop();
    return 0;
}
