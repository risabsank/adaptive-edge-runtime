#include "host_client.h"

#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"

namespace {

constexpr const char* TAG = "host_client";
constexpr int kHttpTimeoutMs = 2000;
constexpr int kResponseBufferSize = 1024;

struct HttpResponseBuffer {
    char data[kResponseBufferSize];
    int length;
};

esp_err_t http_event_handler(esp_http_client_event_t* event) {
    auto* response = static_cast<HttpResponseBuffer*>(event->user_data);
    if (event->event_id == HTTP_EVENT_ON_DATA && response != nullptr) {
        const int copy_len = event->data_len < (kResponseBufferSize - response->length - 1)
            ? event->data_len
            : (kResponseBufferSize - response->length - 1);
        if (copy_len > 0) {
            memcpy(response->data + response->length, event->data, copy_len);
            response->length += copy_len;
            response->data[response->length] = '\0';
        }
    }
    return ESP_OK;
}

bool post_json(const char* url, const char* payload, HttpResponseBuffer* response) {
    response->length = 0;
    response->data[0] = '\0';

    esp_http_client_config_t config = {};
    config.url = url;
    config.timeout_ms = kHttpTimeoutMs;
    config.event_handler = http_event_handler;
    config.user_data = response;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == nullptr) {
        ESP_LOGE(TAG, "failed to initialize HTTP client");
        return false;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, payload, strlen(payload));

    const esp_err_t err = esp_http_client_perform(client);
    const int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    if (err != ESP_OK || status < 200 || status >= 300) {
        ESP_LOGW(TAG, "POST failed url=%s err=%s status=%d", url, esp_err_to_name(err), status);
        return false;
    }
    return true;
}

void copy_json_string(cJSON* root, const char* key, char* target, size_t target_size, const char* fallback) {
    const cJSON* item = cJSON_GetObjectItemCaseSensitive(root, key);
    const char* value = cJSON_IsString(item) ? item->valuestring : fallback;
    strlcpy(target, value, target_size);
}

}  // namespace

bool post_event_to_host(const EdgeEvent& event, const LocalInferenceResult& result, HostDecision* decision) {
    char payload[768];
    snprintf(
        payload,
        sizeof(payload),
        "{"
        "\"event_id\":\"%s\","
        "\"source\":\"esp32s3\","
        "\"features\":[%.4f,%.4f,%.4f],"
        "\"local_prediction\":\"%s\","
        "\"local_confidence\":%.4f,"
        "\"local_latency_ms\":%.4f,"
        "\"queue_depth\":%d,"
        "\"event_priority\":\"%s\","
        "\"host_reachable\":true,"
        "\"recent_failures\":0"
        "}",
        event.event_id,
        event.features[0],
        event.features[1],
        event.features[2],
        result.prediction,
        result.confidence,
        result.latency_ms,
        event.queue_depth,
        event.priority
    );

    HttpResponseBuffer response = {};
    if (!post_json(CONFIG_EDGE_HOST_EVENT_URL, payload, &response)) {
        return false;
    }

    cJSON* root = cJSON_Parse(response.data);
    if (root == nullptr) {
        ESP_LOGW(TAG, "host decision response was not valid JSON: %s", response.data);
        return false;
    }

    copy_json_string(root, "action", decision->action, sizeof(decision->action), "degraded_mode");
    copy_json_string(root, "reason", decision->reason, sizeof(decision->reason), "invalid_host_response");
    const cJSON* timeout = cJSON_GetObjectItemCaseSensitive(root, "timeout_ms");
    decision->timeout_ms = cJSON_IsNumber(timeout) ? timeout->valueint : 0;
    cJSON_Delete(root);
    return true;
}

bool post_fallback_to_host(const EdgeEvent& event) {
    char payload[256];
    snprintf(
        payload,
        sizeof(payload),
        "{\"event_id\":\"%s\",\"features\":[%.4f,%.4f,%.4f]}",
        event.event_id,
        event.features[0],
        event.features[1],
        event.features[2]
    );

    HttpResponseBuffer response = {};
    const bool ok = post_json(CONFIG_EDGE_HOST_FALLBACK_URL, payload, &response);
    if (ok) {
        ESP_LOGI(TAG, "fallback response event=%s body=%s", event.event_id, response.data);
    }
    return ok;
}
