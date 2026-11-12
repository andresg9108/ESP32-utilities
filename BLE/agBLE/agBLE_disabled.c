/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Andrés González */
#include "agBLE.h"
#include "sdkconfig.h"

#if !CONFIG_AGBLE_CLIENT_ENABLED
esp_err_t agble_init(const agble_config_t *config)
{ (void)config; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t agble_scan_start(uint32_t duration_ms)
{ (void)duration_ms; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t agble_scan_stop(void) { return ESP_ERR_NOT_SUPPORTED; }
esp_err_t agble_connect(const agble_address_t *address, uint32_t timeout_ms)
{ (void)address; (void)timeout_ms; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t agble_disconnect(void) { return ESP_ERR_NOT_SUPPORTED; }
esp_err_t agble_write(const void *data, size_t length)
{ (void)data; (void)length; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t agble_read(void) { return ESP_ERR_NOT_SUPPORTED; }
esp_err_t agble_deinit(void) { return ESP_ERR_NOT_SUPPORTED; }
#endif

#if !CONFIG_AGBLE_SERVER_ENABLED
esp_err_t agble_server_start(const char *name, agble_server_callback_t callback, void *context)
{ (void)name; (void)callback; (void)context; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t agble_server_publish(const void *data, size_t length)
{ (void)data; (void)length; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t agble_server_stop(void) { return ESP_ERR_NOT_SUPPORTED; }
#endif
