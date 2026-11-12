/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Andrés González */
#ifndef AGBLE_INTERNAL_H
#define AGBLE_INTERNAL_H

#include <stdbool.h>
#include "esp_err.h"

typedef enum { AGBLE_OWNER_NONE, AGBLE_OWNER_CLIENT, AGBLE_OWNER_SERVER } agble_owner_t;

bool agble_runtime_owned(agble_owner_t owner);
bool agble_runtime_in_host(void);
esp_err_t agble_runtime_init(agble_owner_t owner);
esp_err_t agble_runtime_start(void);
esp_err_t agble_runtime_stop(agble_owner_t owner);

#endif
