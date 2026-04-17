#pragma once

#include <stdint.h>

struct EdgeEvent {
    char event_id[32];
    float features[3];
    int feature_count;
    const char* priority;
    int queue_depth;
};

struct LocalInferenceResult {
    const char* prediction;
    float confidence;
    float latency_ms;
};

struct HostDecision {
    char action[32];
    char reason[64];
    int timeout_ms;
};

