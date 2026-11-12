#include "agBLE.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    assert(agble_init(NULL) == ESP_ERR_NOT_SUPPORTED);
    assert(agble_scan_start(1) == ESP_ERR_NOT_SUPPORTED);
    assert(agble_scan_stop() == ESP_ERR_NOT_SUPPORTED);
    assert(agble_connect(NULL, 1) == ESP_ERR_NOT_SUPPORTED);
    assert(agble_disconnect() == ESP_ERR_NOT_SUPPORTED);
    assert(agble_write(NULL, 0) == ESP_ERR_NOT_SUPPORTED);
    assert(agble_read() == ESP_ERR_NOT_SUPPORTED);
    assert(agble_deinit() == ESP_ERR_NOT_SUPPORTED);
    assert(agble_server_start(NULL, NULL, NULL) == ESP_ERR_NOT_SUPPORTED);
    assert(agble_server_publish(NULL, 0) == ESP_ERR_NOT_SUPPORTED);
    assert(agble_server_stop() == ESP_ERR_NOT_SUPPORTED);
    puts("BLE disabled modes: all public functions return ESP_ERR_NOT_SUPPORTED");
    return 0;
}
