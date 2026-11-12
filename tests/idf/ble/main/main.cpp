#include "agBLE.h"
#include "agBLE.h"

/* Link every API against the real SDK; this is not a radio test.
 * Enlaza toda la API contra el SDK real; no es una prueba de radio. */
static volatile bool exercise_api = false;

extern "C" void app_main(void)
{
    if (!exercise_api) return;
    agble_config_t config = {};
    agble_address_t address = {};
    (void)agble_init(&config);
    (void)agble_scan_start(1000);
    (void)agble_scan_stop();
    (void)agble_connect(&address, 1000);
    (void)agble_write("0", 1);
    (void)agble_read();
    (void)agble_disconnect();
    (void)agble_deinit();
    (void)agble_server_start("test", nullptr, nullptr);
    (void)agble_server_publish("0", 1);
    (void)agble_server_stop();
}
