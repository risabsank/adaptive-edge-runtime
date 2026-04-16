# Roadmap

## Phase 1: Repository And Host Foundation

Status: complete

Deliverables:

- Public README
- Architecture, protocol, roadmap, and decision docs
- Host controller API
- Rule-based decision baseline
- Local simulator
- Initial tests

Exit criteria:

- A developer can run the host controller locally.
- Synthetic events receive deterministic runtime decisions.
- Events and decisions are logged for later analysis.

## Phase 2: Minimal Host Runtime Skeleton

Status: complete

Deliverables:

- FastAPI host service
- `/health`, `/event`, `/decision`, `/infer/fallback`, and `/metrics`
- Deterministic rule-based decision baseline
- Local JSONL event logging
- Basic in-memory runtime metrics for latency, confidence, queue depth, decisions, and fallback requests

Exit criteria:

- The host runtime can receive synthetic events, return deterministic decisions, log event records, expose fallback inference, and report basic JSON metrics.

## Phase 3: ESP32-S3 Local Inference

Deliverables:

- ESP32-S3 firmware skeleton
- Local inference loop with TensorFlow Lite Micro
- Latency and confidence measurement
- Host communication client
- Deterministic offline fallback behavior

Exit criteria:

- ESP32-S3 can run local inference and report runtime state to the host.

## Phase 4: nRF52840 Event Node

Deliverables:

- nRF52840 firmware skeleton
- Event trigger using button, timer, IMU, or synthetic signal
- Compact event transmission to ESP32-S3

Exit criteria:

- nRF52840 can trigger events that flow into ESP32-S3 inference.

## Phase 5: End-To-End Hardware Demo

Deliverables:

- nRF52840 to ESP32-S3 to host event path
- Local inference plus host fallback
- Logged decisions and metrics

Exit criteria:

- The complete heterogeneous pipeline runs on real hardware.

## Phase 6: Observability And Experiments

Deliverables:

- Structured runtime traces
- Static versus adaptive comparison
- Failure scenario tests
- Latency, offload, confidence, and recovery metrics

Exit criteria:

- The runtime can demonstrate concrete improvements over a static pipeline.

## Phase 7: RL Controller Prototype

Deliverables:

- Simulation environment based on real traces
- RL state and action space
- Reward function for latency, reliability, confidence, and offload cost
- Policy evaluation against rule baseline

Exit criteria:

- Learned policy beats or complements the rule baseline in controlled scenarios.

## Phase 8: Hybrid Adaptive Runtime

Deliverables:

- RL controller integration
- Safety validation layer
- Rule fallback override
- Public demo and results

Exit criteria:

- The runtime adapts under changing conditions while preserving deterministic safe behavior.
