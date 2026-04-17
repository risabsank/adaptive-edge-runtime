# ESP32-S3 Edge Inference Node

This directory contains the Phase 3 ESP32-S3 firmware skeleton for the adaptive edge runtime.

The firmware currently:

- Generates synthetic sensor events locally.
- Runs a local inference path and produces a prediction plus confidence.
- Measures local inference latency.
- Sends event state to the host runtime over Wi-Fi HTTP.
- Handles host decisions such as `offload`, `batch`, `retry`, and `degraded_mode`.
- Calls host fallback inference when the host returns `offload`.
- Uses deterministic local fallback behavior when the host is unavailable.

## Current Inference Path

The default build uses a deterministic embedded classifier so the ESP32-S3 can be useful before a real model is exported. The code is structured around `run_local_inference()` so TensorFlow Lite Micro can replace the deterministic score path once a valid model is added.

`CONFIG_EDGE_USE_TFLM` is included as an explicit integration flag, but it is disabled by default because this repository does not yet include a generated TFLite Micro model artifact.

## Build Location

Run ESP-IDF commands from this directory:

```bash
cd firmware/esp32s3
```

See [../../docs/esp32s3-setup.md](../../docs/esp32s3-setup.md) for setup, flashing, networking, and reproducibility instructions.

