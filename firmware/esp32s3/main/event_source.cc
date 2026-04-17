#include "event_source.h"

#include <stdio.h>
#include <stdint.h>

#include "esp_random.h"

namespace {

float random_unit_float() {
    return static_cast<float>(esp_random() % 1000) / 1000.0f;
}

}  // namespace

EdgeEvent next_synthetic_event() {
    static uint32_t sequence = 0;

    EdgeEvent event = {};
    snprintf(event.event_id, sizeof(event.event_id), "esp32s3_%06lu", static_cast<unsigned long>(sequence++));
    event.feature_count = 3;
    event.features[0] = random_unit_float();
    event.features[1] = random_unit_float();
    event.features[2] = random_unit_float();
    event.priority = event.features[2] > 0.92f ? "high" : "normal";
    event.queue_depth = static_cast<int>(esp_random() % 10);
    return event;
}
