# Runtime Protocol

## Overview

The runtime protocol defines how edge nodes report inference state and how the host returns execution decisions. Phase 1 uses HTTP/JSON because it is easy to inspect and simulate. The protocol can later be mapped to MQTT, gRPC, BLE payloads, or compact binary messages.

## Event State

`POST /event`

```json
{
  "event_id": "evt_0001",
  "source": "esp32s3",
  "timestamp_ms": 1710000000000,
  "features": [0.12, 0.4, 0.88],
  "local_prediction": "normal",
  "local_confidence": 0.82,
  "local_latency_ms": 18.5,
  "queue_depth": 2,
  "event_priority": "normal",
  "host_reachable": true,
  "recent_failures": 0
}
```

## Decision Response

```json
{
  "action": "run_local",
  "reason": "local_confidence_sufficient",
  "timeout_ms": 250,
  "metadata": {
    "confidence_threshold": 0.7,
    "latency_budget_ms": 50
  }
}
```

## Metrics Snapshot

`GET /metrics`

```json
{
  "total_events_received": 1,
  "total_decisions_made": 1,
  "total_fallback_inference_requests": 0,
  "latest_local_confidence": 0.82,
  "latest_local_latency_ms": 18.5,
  "latest_queue_depth": 2,
  "latest_decision_action": "run_local"
}
```

## Supported Actions

| Action | Meaning |
|---|---|
| `run_local` | Accept local ESP32-S3 inference result |
| `offload` | Send payload to host fallback inference |
| `use_small_model` | Prefer a faster local model under pressure |
| `use_large_model` | Prefer a more accurate model when resources allow |
| `batch` | Delay briefly and process events together |
| `retry` | Retry after a transient failure |
| `degraded_mode` | Continue with conservative local behavior |

## Priority Levels

| Priority | Meaning |
|---|---|
| `low` | Can tolerate delay or batching |
| `normal` | Default event handling |
| `high` | Should be escalated when confidence or health is uncertain |
| `critical` | Escalate or use safest available path immediately |

## Versioning

Protocol changes should be documented here before firmware depends on them. Breaking changes should increment a protocol version field once embedded clients are implemented.
