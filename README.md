# Adaptive Edge AI Runtime

Adaptive Edge AI Runtime is an end-to-end systems project for adaptive AI inference across heterogeneous edge hardware. The runtime coordinates a low-power nRF52840 event node, an ESP32-S3 embedded inference node, and a host machine that provides fallback inference, control, logging, and observability.

The project focuses on a practical runtime problem: edge AI systems should not always run the same model, at the same rate, on the same device, with fixed thresholds. Real deployments face bursty workloads, limited resources, network instability, uncertain predictions, and partial failures. This runtime adapts execution behavior so inference remains responsive, efficient, and reliable under changing conditions.

## Repository Intent

This repository documents and implements an end-to-end adaptive edge AI runtime. It is intended to grow from a host-side executable skeleton into a real heterogeneous hardware system with embedded inference, runtime control, deterministic safety fallback, and observability.

## Current Phase

Phase 4 establishes the first wireless multi-device path:

- Host runtime with decision and fallback endpoints
- ESP32-S3 local inference and host communication
- nRF52840 BLE event broadcasting
- Wireless nRF52840 -> ESP32-S3 event flow
- Setup and reproducibility documentation for host and firmware bring-up

## Architecture

```text
nRF52840
  low-power sensing / event trigger
      |
      v
ESP32-S3
  local embedded inference
  latency and confidence reporting
      |
      v
Host Machine
  controller
  fallback inference
  logging
  monitoring
```

The host controller receives runtime state from the edge node and returns an execution decision such as `run_local`, `offload`, `use_small_model`, `batch`, `retry`, or `degraded_mode`.

## Repository Layout

```text
adaptive-edge-runtime/
  docs/                 Architecture, roadmap, protocol, and design decisions
  firmware/             Embedded firmware for nRF52840 and ESP32-S3
  host/                 Host-side controller, fallback inference, and monitoring services
  models/               Training code and exported embedded/host models
  experiments/          Runtime traces and experiment results
  scripts/              Developer scripts and local simulators
  tests/                Unit and integration tests
```

## Getting Started

The quickest way to start is to bring the system up in three layers:

1. Start the host runtime on your computer.
2. Flash and run the ESP32-S3 edge runtime.
3. Flash and run the nRF52840 BLE event node.

You can stop after step 1 or 2 if you want to test pieces incrementally.

### 1. Start the host runtime

Create a Python environment and install the host runtime dependencies:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r host/controller/requirements.txt
```

Run the host controller:

```bash
.venv/bin/python -m uvicorn host.controller.app:app --reload --host 0.0.0.0
```

Confirm it is running:

```bash
curl http://127.0.0.1:8000/health
curl http://127.0.0.1:8000/metrics
```

If you want to test the host by itself, in another terminal send synthetic events:

```bash
.venv/bin/python scripts/simulate_events.py
```

Run tests:

```bash
.venv/bin/python -m pytest
```

### 2. Start the ESP32-S3 runtime

The ESP32-S3 connects to your computer in two ways:

- USB for flashing, power, and serial logs
- Wi-Fi HTTP for calling the host runtime

Use ESP-IDF v5.x and run commands from:

```bash
cd firmware/esp32s3
```

Set your host machine IP address in the ESP32-S3 config. On macOS:

```bash
ipconfig getifaddr en0
```

Then configure and build:

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p /dev/cu.usbmodemXXXX flash monitor
```

In `menuconfig`, set:

- Wi-Fi SSID
- Wi-Fi password
- Host `/event` URL
- Host `/infer/fallback` URL

Example host URLs:

```text
http://192.168.1.25:8000/event
http://192.168.1.25:8000/infer/fallback
```

### 3. Start the nRF52840 event node

Phase 4 uses a wireless BLE path, so you do not need a breadboard or jumper wires between the nRF52840 and ESP32-S3.

The intended flow is:

```text
nRF52840 BLE advertisements
  -> ESP32-S3 BLE scanner
  -> local inference
  -> host runtime
```

The nRF52840 uses Zephyr / nRF Connect SDK style files in:

```bash
cd firmware/nrf52840
```

Build and flash with your Zephyr or nRF Connect SDK workflow for the connected board, then watch the serial log for advertisement output. The ESP32-S3 will prefer BLE events from the nRF52840 and fall back to synthetic events if no BLE event arrives before the configured timeout.

### 4. What success looks like

When the full Phase 4 path is up, you should see:

- nRF52840 serial logs showing BLE event advertisements
- ESP32-S3 serial logs showing `source=nrf52840` on received events
- host `/metrics` counters increasing
- host JSONL traces under `experiments/traces/events.jsonl`

## Initial API

- `GET /health` returns host runtime health.
- `GET /metrics` returns simple JSON runtime counters and latest-state values.
- `POST /event` records an event and returns a runtime decision.
- `POST /decision` evaluates runtime state without recording an event.
- `POST /infer/fallback` runs host fallback inference.

## Roadmap

See [docs/roadmap.md](docs/roadmap.md) for the full staged roadmap.

## ESP32-S3 Firmware

Phase 3 adds the ESP32-S3 edge inference node skeleton under `firmware/esp32s3`.

For setup and reproducibility, see [docs/esp32s3-setup.md](docs/esp32s3-setup.md).

## Wireless nRF52840 Setup

Phase 4 adds a wireless BLE event path from the nRF52840 to the ESP32-S3.

For the no-breadboard setup and bring-up steps, see [docs/wireless-phase4-setup.md](docs/wireless-phase4-setup.md).
