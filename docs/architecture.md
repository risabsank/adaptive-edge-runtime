# Architecture

## System Purpose

Adaptive Edge AI Runtime coordinates inference across a low-power event node, an embedded inference node, and a host fallback node. The runtime decides where and how inference should run based on live latency, confidence, queue, connectivity, priority, and failure signals.

## Hardware Roles

| Tier | Device | Responsibility |
|---|---|---|
| Sensing tier | nRF52840 | Low-power sensing, event triggering, compact event transmission |
| Edge inference tier | ESP32-S3 | Local inference, latency measurement, confidence reporting, host communication |
| Host tier | Host machine | Controller, fallback inference, logging, metrics, policy improvement |

## Service Boundaries

| Service | Location | Responsibility |
|---|---|---|
| Event source | nRF52840 | Produces sensor or anomaly events |
| Local inference client | ESP32-S3 | Runs embedded model and reports local results |
| Control Agent | Host | Chooses runtime action from current system state |
| Safety Agent | Host and edge | Validates decisions and applies deterministic overrides |
| Monitoring Agent | Host | Records metrics, events, decisions, and failures |
| Fallback inference | Host | Runs richer inference when local output is uncertain or overloaded |

## Data Flow

```text
1. nRF52840 detects or samples an event.
2. nRF52840 sends event data to ESP32-S3.
3. ESP32-S3 runs local inference and measures latency.
4. ESP32-S3 sends event state to the host controller.
5. Host controller evaluates rule-based or learned policy.
6. Safety layer validates the proposed action.
7. ESP32-S3 executes the approved action.
8. Host logs event, decision, reason, and observed outcome.
```

## Phase 1 Runtime

Phase 1 starts with the host-side runtime and simulator:

```text
scripts/simulate_events.py
      |
      v
POST /event
      |
      v
host.controller.decision.RuleBasedController
      |
      v
JSONL event log + decision response
```

This lets the project validate runtime state, decision semantics, and logging before embedded firmware is complete.

## Safety Principle

Adaptive control is advisory until it passes safety validation. The rule-based fallback remains available whenever learned control is unavailable, slow, disconnected, or unsafe.

