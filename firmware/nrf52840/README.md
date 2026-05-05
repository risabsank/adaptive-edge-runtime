# nRF52840 Event Node

Phase 4 adds a wireless nRF52840 event node that advertises compact event packets over BLE so it can talk to the ESP32-S3 without a breadboard or jumper wires.

Current behavior:

- Generates synthetic event frames every two seconds
- Encodes them into BLE manufacturer advertisement data
- Broadcasts them wirelessly
- Works as the preferred event source for the ESP32-S3 in Phase 4

This is the first end-to-end wireless path:

```text
nRF52840 BLE advertisement
  -> ESP32-S3 BLE scanner
  -> local inference
  -> host /event decision
  -> host /infer/fallback when needed
```

The nRF52840 implementation uses Zephyr / nRF Connect SDK style project files:

- [CMakeLists.txt](/Users/risabsankarstuff/Downloads/adaptive-edge-runtime/firmware/nrf52840/CMakeLists.txt)
- [prj.conf](/Users/risabsankarstuff/Downloads/adaptive-edge-runtime/firmware/nrf52840/prj.conf)
- [src/main.c](/Users/risabsankarstuff/Downloads/adaptive-edge-runtime/firmware/nrf52840/src/main.c)
