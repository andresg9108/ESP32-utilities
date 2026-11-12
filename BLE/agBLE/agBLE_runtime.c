/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Andrés González */
#include "agBLE_internal.h"

#include <stdatomic.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"

static atomic_int owner;
static TaskHandle_t host_task_handle;
static SemaphoreHandle_t exited;
static bool stop_requested;
static esp_err_t cleanup_error;

bool agble_runtime_owned(agble_owner_t wanted)
{
    return atomic_load(&owner) == (int)wanted;
}

bool agble_runtime_in_host(void)
{
    return host_task_handle && xTaskGetCurrentTaskHandle() == host_task_handle;
}

static void host_task(void *arg)
{
    (void)arg;
    nimble_port_run();
    xSemaphoreGive(exited);
    vTaskSuspend(NULL);
}

esp_err_t agble_runtime_init(agble_owner_t wanted)
{
    int expected = AGBLE_OWNER_NONE;
    if (!atomic_compare_exchange_strong(&owner, &expected, wanted)) {
        return ESP_ERR_INVALID_STATE;
    }
    exited = xSemaphoreCreateBinary();
    if (!exited) {
        atomic_store(&owner, AGBLE_OWNER_NONE);
        return ESP_ERR_NO_MEM;
    }
    esp_err_t err = nimble_port_init();
    if (err != ESP_OK) {
        vSemaphoreDelete(exited);
        exited = NULL;
        atomic_store(&owner, AGBLE_OWNER_NONE);
        return err;
    }
    stop_requested = false;
    cleanup_error = ESP_OK;
    return ESP_OK;
}

esp_err_t agble_runtime_start(void)
{
    /* Own the task directly: the IDF 5.4 NimBLE wrapper discards allocation errors.
     * Gestiona la tarea directamente: el wrapper NimBLE de IDF 5.4 descarta errores de asignación. */
    BaseType_t rc = xTaskCreatePinnedToCore(host_task, "agble_host",
        CONFIG_BT_NIMBLE_HOST_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 4,
        &host_task_handle, CONFIG_BT_NIMBLE_PINNED_TO_CORE);
    return rc == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

esp_err_t agble_runtime_stop(agble_owner_t wanted)
{
    if (!agble_runtime_owned(wanted) || agble_runtime_in_host()) {
        return ESP_ERR_INVALID_STATE;
    }
    if (cleanup_error != ESP_OK) {
        return cleanup_error;
    }
    if (host_task_handle) {
        if (!stop_requested) {
            if (nimble_port_stop() != 0) {
                return ESP_FAIL;
            }
            stop_requested = true;
        }
        if (xSemaphoreTake(exited, pdMS_TO_TICKS(CONFIG_AGBLE_STOP_TIMEOUT_MS)) != pdTRUE) {
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelete(host_task_handle);
        host_task_handle = NULL;
    }
    ble_hs_cfg.sync_cb = NULL;
    ble_hs_cfg.reset_cb = NULL;
    cleanup_error = nimble_port_deinit();
    if (cleanup_error != ESP_OK) {
        /* SDK teardown may be partial; do not repeat it or transfer ownership.
         * La limpieza del SDK puede ser parcial; no la repitas ni transfieras la propiedad. */
        return cleanup_error;
    }
    vSemaphoreDelete(exited);
    exited = NULL;
    atomic_store(&owner, AGBLE_OWNER_NONE);
    return ESP_OK;
}
