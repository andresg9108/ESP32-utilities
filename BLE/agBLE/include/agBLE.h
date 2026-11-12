/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Andrés González */
#ifndef AGBLE_H
#define AGBLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* UUIDs and addresses use Bluetooth little-endian byte order.
 * Los UUID y las direcciones utilizan el orden little-endian de Bluetooth. */
typedef struct {
    uint8_t length; /* 2 or 16 / 2 o 16 */
    uint8_t bytes[16];
} agble_uuid_t;

typedef struct {
    uint8_t type; /* 0 public, 1 random / 0 pública, 1 aleatoria */
    uint8_t bytes[6];
} agble_address_t;

typedef enum {
    AGBLE_STACK_READY,
    AGBLE_SCAN_RESULT,
    AGBLE_SCAN_DONE,
    AGBLE_CONNECTED,
    AGBLE_READY,
    AGBLE_DATA,
    AGBLE_WRITE_DONE,
    AGBLE_DISCONNECTED,
    AGBLE_ERROR
} agble_event_type_t;

typedef enum {
    AGBLE_PHASE_STACK,
    AGBLE_PHASE_SCAN,
    AGBLE_PHASE_CONNECT,
    AGBLE_PHASE_DISCOVERY,
    AGBLE_PHASE_SUBSCRIBE,
    AGBLE_PHASE_READ,
    AGBLE_PHASE_WRITE
} agble_phase_t;

/* Events/data are borrowed until callback return. status is a NimBLE/ATT code.
 * Los eventos/datos son prestados hasta terminar el callback. status es un código NimBLE/ATT. */
typedef struct {
    agble_event_type_t type;
    agble_phase_t phase;
    int status;
    agble_address_t address;
    int8_t rssi;
    const uint8_t *data;
    size_t length;
    bool from_read;
} agble_event_t;

typedef void (*agble_callback_t)(const agble_event_t *event, void *context);

typedef struct {
    agble_uuid_t service;
    agble_uuid_t write_char;
    agble_uuid_t receive_char;
    agble_callback_t callback;
    void *context;
} agble_config_t;

/* Initialize NVS first. Single NimBLE owner; serialize public API calls.
 * Inicializa NVS primero. Un único propietario de NimBLE; serializa las llamadas públicas.
 * Callbacks must not block or call deinit/stop. Copy data to retain it.
 * Los callbacks no deben bloquear ni llamar a deinit/stop. Copia los datos para conservarlos.
 * Disabled modes return ESP_ERR_NOT_SUPPORTED. Configuration is copied.
 * Los modos deshabilitados devuelven ESP_ERR_NOT_SUPPORTED. Se copia la configuración. */
esp_err_t agble_init(const agble_config_t *config);
esp_err_t agble_scan_start(uint32_t duration_ms);
esp_err_t agble_scan_stop(void);
esp_err_t agble_connect(const agble_address_t *address, uint32_t timeout_ms);
esp_err_t agble_disconnect(void);
esp_err_t agble_write(const void *data, size_t length);
esp_err_t agble_read(void);
esp_err_t agble_deinit(void);

#define AGBLE_SERVER_SERVICE_UUID 0xFFF0
#define AGBLE_SERVER_COMMAND_UUID 0xFFF1
#define AGBLE_SERVER_STATUS_UUID  0xFFF2
#define AGBLE_SERVER_STATUS_MAX_LENGTH 20

typedef enum {
    AGBLE_SERVER_ADVERTISING,
    AGBLE_SERVER_CONNECTED,
    AGBLE_SERVER_DISCONNECTED,
    AGBLE_SERVER_COMMAND,
    AGBLE_SERVER_ERROR
} agble_server_event_type_t;

typedef struct {
    agble_server_event_type_t type;
    int status;
    const uint8_t *data;
    size_t length;
} agble_server_event_t;

typedef void (*agble_server_callback_t)(const agble_server_event_t *event, void *context);

/* Name: 1..20 bytes. Commands: 1..512 bytes. Status: 1..20 bytes, copied.
 * Nombre: 1..20 bytes. Comandos: 1..512 bytes. Estado: 1..20 bytes, copiados.
 * Server/client are exclusive. No pairing or encryption is initiated.
 * Servidor/cliente son excluyentes. No se inicia emparejamiento ni cifrado. */
esp_err_t agble_server_start(const char *name, agble_server_callback_t callback, void *context);
esp_err_t agble_server_publish(const void *data, size_t length);
esp_err_t agble_server_stop(void);

#ifdef __cplusplus
}
#endif
#endif
