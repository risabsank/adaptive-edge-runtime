#pragma once

#include "runtime_types.h"

bool post_event_to_host(
    const EdgeEvent& event,
    const LocalInferenceResult& result,
    int recent_failures,
    HostDecision* decision
);
bool post_fallback_to_host(const EdgeEvent& event);
