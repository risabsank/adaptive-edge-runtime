#include "ble_event_receiver.h"

#include <stdio.h>
#include <string.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace {

constexpr const char* TAG = "ble_event_rx";
constexpr uint16_t kNordicCompanyId = 0x0059;
constexpr uint8_t kFrameMagic0 = 0xAE;
constexpr uint8_t kFrameMagic1 = 0x10;

struct __attribute__((packed)) BleEventPayload {
    uint16_t sequence;
    uint8_t features[3];
    uint8_t priority;
    uint8_t queue_depth;
};

constexpr size_t kPayloadLength = 2 + sizeof(BleEventPayload);

QueueHandle_t ble_event_queue = nullptr;
bool scan_started = false;

bool parse_ble_payload(const uint8_t* payload, uint8_t payload_len, EdgeEvent* event) {
    if (payload_len < kPayloadLength || payload[0] != kFrameMagic0 || payload[1] != kFrameMagic1) {
        return false;
    }

    const BleEventPayload* frame = reinterpret_cast<const BleEventPayload*>(payload + 2);
    snprintf(event->event_id, sizeof(event->event_id), "nrf_%05u", static_cast<unsigned>(frame->sequence));
    event->feature_count = 3;
    event->features[0] = static_cast<float>(frame->features[0]) / 255.0f;
    event->features[1] = static_cast<float>(frame->features[1]) / 255.0f;
    event->features[2] = static_cast<float>(frame->features[2]) / 255.0f;
    event->priority = frame->priority > 0 ? "high" : "normal";
    event->queue_depth = static_cast<int>(frame->queue_depth);
    event->source = "nrf52840";
    return true;
}

void gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
    if (event == ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT) {
        esp_ble_gap_start_scanning(0);
        return;
    }

    if (event == ESP_GAP_BLE_SCAN_START_COMPLETE_EVT) {
        if (param->scan_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
            scan_started = true;
            ESP_LOGI(TAG, "BLE scan started");
        } else {
            ESP_LOGE(TAG, "BLE scan failed status=%d", param->scan_start_cmpl.status);
        }
        return;
    }

    if (event != ESP_GAP_BLE_SCAN_RESULT_EVT || param->scan_rst.search_evt != ESP_GAP_SEARCH_INQ_RES_EVT) {
        return;
    }

    uint8_t len = 0;
    const uint8_t* manufacturer = esp_ble_resolve_adv_data(
        param->scan_rst.ble_adv,
        ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE,
        &len
    );

    if (manufacturer == nullptr || len < 2 + kPayloadLength) {
        return;
    }

    const uint16_t company_id = static_cast<uint16_t>(manufacturer[1] << 8 | manufacturer[0]);
    if (company_id != kNordicCompanyId) {
        return;
    }

    EdgeEvent event_payload = {};
    if (!parse_ble_payload(manufacturer + 2, len - 2, &event_payload)) {
        return;
    }

    if (ble_event_queue != nullptr) {
        xQueueOverwrite(ble_event_queue, &event_payload);
    }
}

}  // namespace

bool init_ble_event_receiver() {
    ble_event_queue = xQueueCreate(1, sizeof(EdgeEvent));
    if (ble_event_queue == nullptr) {
        ESP_LOGE(TAG, "failed to allocate BLE event queue");
        return false;
    }

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_callback));

    esp_ble_scan_params_t scan_params = {};
    scan_params.scan_type = BLE_SCAN_TYPE_ACTIVE;
    scan_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
    scan_params.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL;
    scan_params.scan_interval = 0x50;
    scan_params.scan_window = 0x30;
    scan_params.scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE;

    ESP_ERROR_CHECK(esp_ble_gap_set_scan_params(&scan_params));
    return true;
}

bool wait_for_ble_event(EdgeEvent* event, uint32_t timeout_ms) {
    if (!scan_started || ble_event_queue == nullptr) {
        return false;
    }

    return xQueueReceive(ble_event_queue, event, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}
