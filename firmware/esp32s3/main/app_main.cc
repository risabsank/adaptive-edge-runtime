#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "event_source.h"
#include "host_client.h"
#include "local_inference.h"

namespace {

constexpr const char* TAG = "adaptive_edge";
constexpr EventBits_t kWifiConnectedBit = BIT0;

EventGroupHandle_t wifi_event_group;

void wifi_event_handler(void*, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi disconnected; retrying");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, kWifiConnectedBit);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        auto* event = static_cast<ip_event_got_ip_t*>(event_data);
        ESP_LOGI(TAG, "Wi-Fi connected ip=" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, kWifiConnectedBit);
    }
}

bool configure_wifi() {
    if (strlen(CONFIG_EDGE_WIFI_SSID) == 0) {
        ESP_LOGW(TAG, "CONFIG_EDGE_WIFI_SSID is empty; host communication will use local fallback");
        return false;
    }

    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_config));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, nullptr));

    wifi_config_t wifi_config = {};
    strlcpy(reinterpret_cast<char*>(wifi_config.sta.ssid), CONFIG_EDGE_WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strlcpy(reinterpret_cast<char*>(wifi_config.sta.password), CONFIG_EDGE_WIFI_PASSWORD, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    const EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        kWifiConnectedBit,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(10000)
    );
    return (bits & kWifiConnectedBit) != 0;
}

void handle_host_decision(const EdgeEvent& event, const LocalInferenceResult& result, const HostDecision& decision) {
    ESP_LOGI(
        TAG,
        "host decision event=%s action=%s reason=%s timeout_ms=%d",
        event.event_id,
        decision.action,
        decision.reason,
        decision.timeout_ms
    );

    if (strcmp(decision.action, "offload") == 0) {
        if (!post_fallback_to_host(event)) {
            apply_local_fallback(event, result);
        }
    } else if (strcmp(decision.action, "degraded_mode") == 0 || strcmp(decision.action, "retry") == 0) {
        apply_local_fallback(event, result);
    } else if (strcmp(decision.action, "batch") == 0) {
        vTaskDelay(pdMS_TO_TICKS(decision.timeout_ms));
    }
}

}  // namespace

extern "C" void app_main() {
    const esp_err_t nvs_status = nvs_flash_init();
    if (nvs_status == ESP_ERR_NVS_NO_FREE_PAGES || nvs_status == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    } else {
        ESP_ERROR_CHECK(nvs_status);
    }

    const bool wifi_ready = configure_wifi();
    ESP_LOGI(TAG, "ESP32-S3 edge runtime started wifi_ready=%s", wifi_ready ? "true" : "false");

    while (true) {
        const EdgeEvent event = next_synthetic_event();
        const LocalInferenceResult result = run_local_inference(event);
        ESP_LOGI(
            TAG,
            "local inference event=%s prediction=%s confidence=%.3f latency_ms=%.3f queue_depth=%d priority=%s",
            event.event_id,
            result.prediction,
            result.confidence,
            result.latency_ms,
            event.queue_depth,
            event.priority
        );

        HostDecision decision = {};
        if (wifi_ready && post_event_to_host(event, result, &decision)) {
            handle_host_decision(event, result, decision);
        } else {
            apply_local_fallback(event, result);
        }

        vTaskDelay(pdMS_TO_TICKS(CONFIG_EDGE_SYNTHETIC_EVENT_PERIOD_MS));
    }
}
