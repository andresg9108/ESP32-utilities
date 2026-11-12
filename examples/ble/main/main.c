#include "agBLE.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "ble_example";

static void on_ble_event(const agble_server_event_t *event, void *context)
{
    (void)context;
    switch (event->type) {
    case AGBLE_SERVER_COMMAND: {
        if (event->length != 1 || (event->data[0] != '0' && event->data[0] != '1' &&
                                   event->data[0] != 0 && event->data[0] != 1)) {
            ESP_LOGW(TAG, "Send one byte: 0 or 1 / Envía un byte: 0 o 1");
            break;
        }
        int level = event->data[0] == '1' || event->data[0] == 1;
        esp_err_t err = gpio_set_level(GPIO_NUM_4, level);
        if (err == ESP_OK) {
            const uint8_t status = level ? '1' : '0';
            err = agble_server_publish(&status, sizeof(status));
        }
        if (err != ESP_OK) ESP_LOGE(TAG, "GPIO/BLE: %s", esp_err_to_name(err));
        else ESP_LOGI(TAG, "GPIO 4: %d", level);
        break;
    }
    case AGBLE_SERVER_ADVERTISING:
        ESP_LOGI(TAG, "agBLE: 0xFFF0 / 0xFFF1 / 0xFFF2");
        break;
    case AGBLE_SERVER_ERROR:
        ESP_LOGE(TAG, "BLE: %d", event->status);
        break;
    default:
        break;
    }
}

void app_main(void)
{
    const gpio_config_t output = {
        .pin_bit_mask = 1ULL << GPIO_NUM_4,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&output));
    ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_4, 0));
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(agble_server_start("agBLE", on_ble_event, NULL));
    const uint8_t initial_status = '0';
    ESP_ERROR_CHECK(agble_server_publish(&initial_status, sizeof(initial_status)));
}
