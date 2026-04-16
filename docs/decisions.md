# Design Decisions

## 001: Start With HTTP/JSON For Host Control

Status: accepted

Phase 1 uses HTTP/JSON for the host control API because it is easy to inspect, test, and simulate. This keeps early development focused on runtime semantics instead of transport complexity.

Future firmware may use MQTT, gRPC, BLE, or compact binary messages if needed.

## 002: Build Rule-Based Control Before RL

Status: accepted

The first controller is deterministic. It provides a baseline, a safety fallback, and a working runtime before reinforcement learning is introduced.

RL will be added after traces and failure scenarios exist, because learned control needs a realistic environment and measurable baseline.

## 003: Host Logging Starts As JSONL

Status: accepted

Phase 1 logs events and decisions to JSONL. JSONL is simple, inspectable, append-friendly, and easy to convert to Pandas or Parquet later.

SQLite, Prometheus, and Grafana can be added after the runtime event schema stabilizes.

