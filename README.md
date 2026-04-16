# Adaptive Edge AI Runtime

Adaptive Edge AI Runtime is an end-to-end systems project for adaptive AI inference across heterogeneous edge hardware. The runtime coordinates a low-power nRF52840 event node, an ESP32-S3 embedded inference node, and a host machine that provides fallback inference, control, logging, and observability.

The project focuses on a practical runtime problem: edge AI systems should not always run the same model, at the same rate, on the same device, with fixed thresholds. Real deployments face bursty workloads, limited resources, network instability, uncertain predictions, and partial failures. This runtime adapts execution behavior so inference remains responsive, efficient, and reliable under changing conditions.

## Repository Intent

This repository documents and implements an end-to-end adaptive edge AI runtime. It is intended to grow from a host-side executable skeleton into a real heterogeneous hardware system with embedded inference, runtime control, deterministic safety fallback, and observability.

## Current Phase

Phase 1 establishes the project foundation:

- Public-facing project documentation
- Architecture and protocol docs
- Host controller skeleton
- Rule-based decision baseline
- Local event simulator
- Initial test structure

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

## Quick Start

Create a Python environment and install the host runtime dependencies:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r host/controller/requirements.txt
```

Run the host controller:

```bash
uvicorn host.controller.app:app --reload
```

In another terminal, send synthetic events:

```bash
python scripts/simulate_events.py
```

Run tests:

```bash
pytest
```

## Initial API

- `GET /health` returns host runtime health.
- `GET /metrics` returns simple JSON runtime counters and latest-state values.
- `POST /event` records an event and returns a runtime decision.
- `POST /decision` evaluates runtime state without recording an event.
- `POST /infer/fallback` runs host fallback inference.

## Roadmap

See [docs/roadmap.md](docs/roadmap.md) for the full staged roadmap.
