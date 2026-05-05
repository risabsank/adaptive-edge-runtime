#pragma once

#include <stdint.h>

#include "runtime_types.h"

bool init_ble_event_receiver();
bool wait_for_ble_event(EdgeEvent* event, uint32_t timeout_ms);

