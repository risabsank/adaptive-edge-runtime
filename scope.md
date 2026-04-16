# Adaptive Edge AI Runtime

## Project Summary

Adaptive Edge AI Runtime is an end-to-end systems project for running AI inference across heterogeneous edge hardware under changing runtime conditions.

The runtime coordinates a low-power sensing node, an embedded inference node, and a host fallback node to dynamically decide how inference should be executed. Its goal is to maintain low latency, efficient resource use, and reliable behavior when workloads, device health, network conditions, and prediction confidence change over time.

This repository documents the design and implementation of a real adaptive edge AI runtime, not just a model experiment or research simulator.

## Problem Statement

Most edge AI pipelines are static. They typically run one model, at one frequency, on one device, using fixed thresholds and fixed execution logic.

That approach breaks down in real deployments. Edge systems must handle:

- Bursty event streams
- Limited compute, memory, and energy budgets
- Network instability between devices
- Uncertain or low-confidence predictions
- Device failures and degraded operating modes
- Competing latency and accuracy requirements

A static pipeline cannot reliably balance responsiveness, efficiency, and correctness across these conditions. This project addresses that gap by building a runtime that continuously adapts inference execution in response to live system state.

## Project Goals

The primary goal is to build a deployable adaptive runtime for edge AI inference across heterogeneous hardware.

The runtime should:

- Execute inference locally when possible
- Offload inference when local execution is overloaded, uncertain, or unsafe
- Select between smaller/faster and larger/more accurate models
- Adjust sampling or event processing frequency based on system load
- Escalate uncertain or high-priority cases to a stronger fallback node
- Maintain deterministic safe behavior when adaptive control is unavailable
- Collect runtime metrics for observability and future policy improvement
- Demonstrate real execution across embedded hardware and host-side services

## Non-Goals

This project is not intended to be:

- A benchmark-only model comparison project
- A purely simulated reinforcement learning environment
- A cloud-first inference platform
- A general-purpose MLOps framework
- A replacement for safety-critical certification workflows
- A project focused only on maximizing model accuracy

The focus is runtime control: how an edge AI system decides where, when, and how inference should run under real-world constraints.

## Target Users

This project is designed for:

- Engineers interested in edge AI systems
- Embedded systems developers
- ML engineers working on deployment and inference reliability
- Researchers exploring adaptive runtime control
- Recruiters evaluating systems, embedded AI, and applied ML capability
- Open-source collaborators interested in heterogeneous edge computing

## System Architecture Overview

The runtime is built around three hardware tiers:

| Component | Role |
|---|---|
| nRF52840 | Low-power sensing and event-trigger node |
| ESP32-S3 | Primary embedded inference node |
| Host machine | Higher-capacity fallback, controller, logging, and observability node |

The system is framed as an adaptive event detection and anomaly triage pipeline.

Sensor or event data originates on the nRF52840. The ESP32-S3 performs local inference when conditions allow. The host machine handles controller logic, richer fallback inference, logging, and observability for uncertain, high-load, or high-priority cases.

## Why Heterogeneous Hardware Matters

Real edge systems are not uniform. They often combine ultra-low-power microcontrollers, more capable embedded processors, and nearby host or gateway machines.

Each tier has different strengths:

- The nRF52840 is suitable for low-power sensing and event triggering.
- The ESP32-S3 provides local embedded inference close to the data source.
- The host machine offers more compute, storage, observability, and fallback capacity.

Using heterogeneous hardware makes the runtime problem meaningful. The system must decide how to use each device based on latency, power, confidence, availability, and reliability rather than assuming one fixed execution path.

## Hardware and Software Components

### Hardware

- nRF52840 development board
- ESP32-S3 development board
- Host machine for controller, fallback inference, logging, and analysis
- Communication links between sensor, edge, and host nodes

### Software

- Embedded firmware for sensing and event transmission
- Embedded ML inference on ESP32-S3
- Host-side runtime controller
- Rule-based safety fallback
- Logging and observability pipeline
- Metrics collection for latency, confidence, queue depth, failures, and system health
- Policy improvement workflow for future adaptive behavior

## Runtime Decision Model

The runtime adapts how inference is executed based on live system state.

It can adjust:

- Local vs. offloaded inference
- Model selection
- Sampling rate or event frequency
- Confidence thresholds for escalation
- Batching vs. immediate execution
- Retry, timeout, and degraded mode behavior

Example state signals include:

- Inference latency
- Prediction confidence
- Queue depth
- Device availability
- Communication failures
- Recent error rate
- Event priority
- Host connectivity
- Resource pressure

The control objective is not simply to maximize accuracy. The runtime must balance latency, efficiency, reliability, and confidence under changing conditions.

## Control and Safety Design

The system uses a layered control architecture.

### RL-Based Controller

A reinforcement learning based controller selects runtime actions from observed system state. Its role is to learn adaptive policies that improve execution behavior over time.

Possible actions include:

- Run inference locally
- Offload inference to the host
- Use a smaller or larger model
- Increase or decrease event frequency
- Escalate uncertain predictions
- Enter degraded mode
- Retry or timeout a request

### Safety Layer

Before an action is executed, it is validated by a safety layer. This layer prevents unsafe or invalid decisions, such as actions that exceed device capability, violate latency requirements, or create excessive retry loops.

### Rule-Based Fallback

RL alone is insufficient for this system.

A learned controller may be uncertain, slow, disconnected, undertrained, or exposed to runtime conditions it has not seen before. Edge systems need deterministic behavior when adaptive control is unavailable or unsafe.

The rule-based fallback provides predictable behavior for conditions such as:

- Controller timeout
- Missing state data
- Host disconnection
- Repeated inference failures
- Unsafe action proposals
- Low-confidence RL decisions
- Resource exhaustion

This hybrid approach allows the runtime to benefit from adaptive control while preserving reliable behavior under failure modes.

## Multi-Agent and Service Structure

The runtime is organized as a set of cooperating agents or services.

| Agent / Service | Responsibility |
|---|---|
| Control Agent | Chooses adaptive runtime actions based on system state |
| Safety Agent | Validates actions and applies guardrails or overrides |
| Monitoring Agent | Collects metrics, health signals, logs, and device state |
| Adaptation Agent | Analyzes execution history and supports policy or rule improvement |

This decomposition keeps decision-making, safety, observability, and improvement workflows separate while allowing them to operate as one runtime system.

## End-to-End Execution Flow

1. The nRF52840 senses data or detects an event.
2. The event is sent to the ESP32-S3.
3. The ESP32-S3 performs local inference when runtime conditions permit.
4. The Monitoring Agent collects latency, confidence, queue, and health metrics.
5. The Control Agent evaluates the current system state.
6. The Safety Agent validates the proposed runtime action.
7. The runtime either continues locally, changes model behavior, adjusts event frequency, batches work, retries, enters degraded mode, or offloads inference.
8. The host machine handles fallback inference for uncertain, high-load, or high-priority cases.
9. Logs and metrics are stored for observability and future adaptation.
10. The Adaptation Agent uses runtime history to improve policies, thresholds, and fallback rules over time.

## Key Deliverables

The project aims to deliver:

- Working firmware for the nRF52840 sensing/event node
- Working ESP32-S3 embedded inference pipeline
- Host-side controller and fallback inference service
- Runtime state representation and action model
- RL-based adaptive controller prototype
- Deterministic rule-based fallback system
- Safety validation layer for runtime decisions
- Metrics and logging pipeline
- End-to-end adaptive event detection demo
- Documentation explaining architecture, runtime behavior, and design decisions

## Success Criteria

The runtime is successful if it can demonstrate measurable improvements in reliability, efficiency, and responsiveness compared with a static edge inference pipeline.

Concrete success criteria include:

- Maintains bounded inference latency under changing event load
- Reduces unnecessary host offloads when local inference is sufficient
- Escalates uncertain or high-priority cases to stronger inference reliably
- Continues operating in degraded mode during host or communication failure
- Avoids unsafe adaptive actions through deterministic validation
- Adjusts sampling or execution frequency when resource pressure increases
- Logs enough runtime state to explain why decisions were made
- Recovers from transient failures without manual intervention
- Demonstrates real execution across nRF52840, ESP32-S3, and host machine

## Why This Project Is Valuable

Adaptive Edge AI Runtime focuses on the systems problem behind practical edge AI deployment.

A model that performs well in isolation is not enough. Real edge systems must make runtime decisions under constraints: where inference should run, how often it should run, when to trust a local result, when to escalate, and how to keep operating during partial failure.

This project is valuable because it combines:

- Real heterogeneous hardware
- Embedded inference
- Adaptive systems control
- Reinforcement learning for runtime decisions
- Deterministic safety fallback
- Observability and health tracking
- Execution under changing runtime conditions

The result is a practical runtime architecture for keeping edge inference responsive, efficient, and reliable without relying on brittle static pipelines.

## Future Extension Ideas

Potential future extensions include:

- Multi-model inference with dynamic accuracy and latency tradeoffs
- Energy-aware scheduling using battery or power measurements
- More advanced RL training environments based on real runtime traces
- Distributed multi-node edge coordination
- Secure communication between device tiers
- OTA policy and rule updates
- Anomaly-specific escalation policies
- Visualization dashboard for runtime decisions and system health
- Support for additional microcontrollers or edge accelerators
- Formal verification of safety rules for constrained deployment scenarios