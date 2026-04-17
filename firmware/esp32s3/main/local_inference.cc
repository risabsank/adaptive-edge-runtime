#include "local_inference.h"

#include "esp_log.h"
#include "esp_timer.h"

namespace {

constexpr const char* TAG = "local_inference";

float mean_feature_score(const EdgeEvent& event) {
    if (event.feature_count == 0) {
        return 0.0f;
    }

    float total = 0.0f;
    for (int index = 0; index < event.feature_count; ++index) {
        total += event.features[index];
    }
    return total / static_cast<float>(event.feature_count);
}

float confidence_from_score(float score) {
    float distance = score > 0.5f ? score - 0.5f : 0.5f - score;
    float confidence = 0.5f + distance;
    if (confidence > 0.99f) {
        return 0.99f;
    }
    if (confidence < 0.5f) {
        return 0.5f;
    }
    return confidence;
}

}  // namespace

LocalInferenceResult run_local_inference(const EdgeEvent& event) {
    const int64_t start_us = esp_timer_get_time();

#if CONFIG_EDGE_USE_TFLM
    // TFLite Micro hook:
    // 1. Add a valid model_data.cc/model_data.h exported from TensorFlow Lite.
    // 2. Add the esp-tflite-micro component to this ESP-IDF project.
    // 3. Replace the deterministic score path below with MicroInterpreter invocation.
    ESP_LOGW(TAG, "CONFIG_EDGE_USE_TFLM is set, but no model runner has been linked yet; using deterministic classifier");
#endif

    const float score = mean_feature_score(event);
    LocalInferenceResult result = {};
    result.prediction = score >= 0.65f ? "anomaly" : "normal";
    result.confidence = confidence_from_score(score);
    result.latency_ms = static_cast<float>(esp_timer_get_time() - start_us) / 1000.0f;
    return result;
}

void apply_local_fallback(const EdgeEvent& event, const LocalInferenceResult& result) {
    ESP_LOGW(
        TAG,
        "local fallback event=%s prediction=%s confidence=%.3f latency_ms=%.3f",
        event.event_id,
        result.prediction,
        result.confidence,
        result.latency_ms
    );
}

