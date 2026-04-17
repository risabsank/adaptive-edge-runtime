idf_component_register(
    SRCS
        "app_main.cc"
        "event_source.cc"
        "host_client.cc"
        "local_inference.cc"
    INCLUDE_DIRS "."
    REQUIRES
        esp_http_client
        esp_timer
        esp_wifi
        json
        nvs_flash
)
