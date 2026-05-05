# Wireless Phase 4 Setup

## Goal

Bring up the first real wireless path:

```text
nRF52840 --BLE--> ESP32-S3 --Wi-Fi HTTP--> Host runtime
```

This setup is designed specifically for a no-breadboard, no-wired-interconnect workflow. Each board is only connected to your computer by USB for flashing and serial logs.

## Recommended Interfaces

Use:

- nRF52840 -> ESP32-S3: BLE advertisements
- ESP32-S3 -> host: Wi-Fi HTTP/JSON
- USB: flashing, power, serial monitoring

Do not use for this phase:

- UART between boards
- SPI or I2C between boards
- BLE directly from nRF52840 to the host
- MQTT or gRPC between ESP32-S3 and host

BLE advertisements are the simplest wireless board-to-board option here because they avoid breadboards, jumper wires, and a full GATT service design.

## Physical Setup

Connect both boards separately to your computer:

```text
Computer USB port 1 -> ESP32-S3
Computer USB port 2 -> nRF52840
```

There is no direct cable between the boards.

Runtime communication is:

```text
nRF52840 advertises event packets over BLE
ESP32-S3 scans for those BLE packets
ESP32-S3 posts runtime state to the host over Wi-Fi
```

## Step 1: Start the Host Runtime

From the repository root:

```bash
.venv/bin/python -m uvicorn host.controller.app:app --reload --host 0.0.0.0
```

Check:

```bash
curl http://127.0.0.1:8000/health
curl http://127.0.0.1:8000/metrics
```

Find your computer's Wi-Fi IP address:

```bash
ipconfig getifaddr en0
```

Use that IP in the ESP32-S3 host URL settings.

## Step 2: Configure and Flash the ESP32-S3

From:

```bash
cd firmware/esp32s3
```

Run:

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p /dev/cu.usbmodemXXXX flash monitor
```

Important settings in `menuconfig`:

- `EDGE_WIFI_SSID`
- `EDGE_WIFI_PASSWORD`
- `EDGE_HOST_EVENT_URL`
- `EDGE_HOST_FALLBACK_URL`
- `EDGE_USE_BLE_EVENT_SOURCE=y`
- `EDGE_BLE_EVENT_TIMEOUT_MS`

The ESP32-S3 should:

- connect to Wi-Fi
- start BLE scanning
- wait for nRF52840 BLE event packets
- fall back to synthetic events if none arrive in time

## Step 3: Configure and Flash the nRF52840

From:

```bash
cd firmware/nrf52840
```

This firmware uses Zephyr / nRF Connect SDK style files:

- `CMakeLists.txt`
- `prj.conf`
- `src/main.c`

Build and flash with your nRF Connect SDK workflow for the connected board.

At runtime, the nRF52840:

- generates synthetic event frames
- encodes them into BLE manufacturer data
- advertises them every two seconds

## Step 4: Verify the Full Path

Expected host behavior:

- `/metrics` event counters rise
- `experiments/traces/events.jsonl` grows

Expected ESP32-S3 behavior:

- logs show Wi-Fi ready
- logs show BLE event source ready
- logs show `source=nrf52840` when BLE events arrive
- logs show host decisions and fallback calls when needed

Expected nRF52840 behavior:

- serial logs show advertised event sequence and feature bytes

## Failure Modes

If the ESP32-S3 only shows synthetic events:

- confirm the nRF52840 firmware is actually advertising
- confirm `EDGE_USE_BLE_EVENT_SOURCE=y`
- move the boards physically closer
- power-cycle the ESP32-S3 after starting the nRF52840

If the ESP32-S3 sees BLE events but the host does not:

- confirm the host is running with `--host 0.0.0.0`
- confirm the host URL uses the computer's LAN IP, not `127.0.0.1`
- confirm the ESP32-S3 is on the same Wi-Fi network as the host machine

## Reproducibility Notes

Record these values for a repeatable demo:

- Git commit SHA
- ESP-IDF version
- nRF Connect SDK / Zephyr version
- host machine IP address
- ESP32-S3 host URLs
- BLE event timeout
- Wi-Fi SSID used
- serial log snippets from both boards
- host `/metrics` output during the run

