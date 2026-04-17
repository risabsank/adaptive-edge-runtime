#pragma once

#include "runtime_types.h"

LocalInferenceResult run_local_inference(const EdgeEvent& event);
void apply_local_fallback(const EdgeEvent& event, const LocalInferenceResult& result);

