# ESP32-S3 Firmware

Embedded inference firmware will live here.

Initial responsibilities:

- Receive events from the nRF52840
- Run local TensorFlow Lite Micro inference
- Measure latency and confidence
- Report runtime state to the host controller
- Execute approved runtime decisions

