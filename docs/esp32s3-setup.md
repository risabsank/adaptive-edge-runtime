# ESP32-S3 Setup And Reproducibility

## Recommended Hardware Setup

Use this Phase 3 setup:

```text
Computer
  | USB: flashing, power, serial logs
  |
ESP32-S3
  | Wi-Fi HTTP
  |
Host runtime on the same computer
```

The ESP32-S3 should connect to the computer in two ways:

- USB for flashing, power, and serial monitoring.
- Wi-Fi for runtime API calls to the host FastAPI service.

Do not add the nRF52840 yet. Phase 3 uses synthetic events generated on the ESP32-S3 so the edge inference node can be debugged by itself.

## Interface Choice

Use Wi-Fi HTTP between the ESP32-S3 and host for this phase.

Recommended:

- USB for flashing and logs.
- Wi-Fi station mode for runtime communication.
- HTTP/JSON for `/event` and `/infer/fallback`.
- Synthetic events before real sensors or nRF52840 input.

Avoid for now:

- BLE between ESP32-S3 and host.
- MQTT.
- gRPC.
- Custom binary protocols.
- nRF52840 integration.
- Real-time dashboards.

HTTP/JSON is not the most efficient final transport, but it is the best first interface because it is easy to inspect, debug, and reproduce.

## Host Setup

From the repository root, run the host runtime:

```bash
.venv/bin/python -m uvicorn host.controller.app:app --reload --host 0.0.0.0
```

Find your computer's local IP address on the Wi-Fi network.

On macOS:

```bash
ipconfig getifaddr en0
```

Example host URLs:

```text
http://192.168.1.25:8000/event
http://192.168.1.25:8000/infer/fallback
```

Use your actual IP address when configuring the ESP32-S3.

## ESP-IDF Setup

Install ESP-IDF using Espressif's official installer or command-line setup. Use ESP-IDF v5.x for this project.

After ESP-IDF is installed and activated:

```bash
cd firmware/esp32s3
idf.py set-target esp32s3
idf.py menuconfig
```

In `menuconfig`, open:

```text
Adaptive Edge Runtime
```

Set:

- Wi-Fi SSID
- Wi-Fi password
- Host `/event` URL
- Host `/infer/fallback` URL
- Synthetic event period if needed

Then build:

```bash
idf.py build
```

Flash and monitor:

```bash
idf.py -p /dev/cu.usbmodem11101 flash monitor
```

Replace `/dev/cu.usbmodemXXXX` with your ESP32-S3 serial port.

To list likely serial ports on macOS:

```bash
ls /dev/cu.*
```

## Expected Runtime Behavior

After flashing, the ESP32-S3 should:

1. Connect to Wi-Fi.
2. Generate a synthetic event.
3. Run local inference.
4. Measure local inference latency.
5. POST the event state to the host `/event` endpoint.
6. Log the host decision.
7. If the decision is `offload`, call `/infer/fallback`.
8. If the host is unreachable, use deterministic local fallback.

Example serial logs should include lines like:

```text
local inference event=esp32s3_000001 prediction=normal confidence=0.742 latency_ms=0.021
host decision event=esp32s3_000001 action=run_local reason=local_inference_acceptable
```

On the host, check:

```bash
curl http://127.0.0.1:8000/metrics
```

The event and decision counters should increase as the ESP32-S3 runs.

## TensorFlow Lite Micro Integration

The firmware is prepared for TensorFlow Lite Micro, but the repository does not yet include a generated model.

Use this progression:

1. Keep `CONFIG_EDGE_USE_TFLM=n` while validating Wi-Fi, HTTP, timing, and decision handling.
2. Train or choose a small model with a fixed input shape matching the event features.
3. Convert the model to TensorFlow Lite.
4. Convert the `.tflite` file to a C array, commonly named `model_data.cc` and `model_data.h`.
5. Add the ESP-IDF TensorFlow Lite Micro component.
6. Replace the deterministic score path in `local_inference.cc` with a `MicroInterpreter` invocation.
7. Enable `CONFIG_EDGE_USE_TFLM=y`.

Do not enable the TFLM flag until a valid model and component are linked.

## Reproducibility Checklist

Record these values for any demo or experiment:

- ESP32-S3 board model.
- ESP-IDF version.
- Git commit SHA.
- Wi-Fi network used.
- Host machine IP address.
- Host runtime command.
- Configured `/event` and `/infer/fallback` URLs.
- Synthetic event period.
- Whether `CONFIG_EDGE_USE_TFLM` is enabled.
- Serial log excerpt showing local inference and host decision.
- Host `/metrics` output after the run.

## Troubleshooting

If the ESP32-S3 cannot reach the host:

- Confirm the host server uses `--host 0.0.0.0`.
- Confirm ESP32-S3 and computer are on the same Wi-Fi network.
- Confirm the configured host URL uses the computer's LAN IP, not `127.0.0.1`.
- Check local firewall prompts on the computer.
- Open `http://<host-ip>:8000/health` from another device on the same network if possible.

If flashing fails:

- Confirm the USB cable supports data, not only charging.
- Hold the board boot button while starting the flash command if required by your board.
- Try another serial port from `ls /dev/cu.*`.
- Close any other serial monitor using the same port.

