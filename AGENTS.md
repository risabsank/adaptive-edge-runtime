# AGENTS.md

## Project Overview
- **Project:** Adaptive Edge AI Runtime — An edge AI runtime that intelligently adapts execution decisions across devices to maintain performance under real-world variability, while guaranteeing reliability through deterministic fallback.
- **Target user:** Edge AI / IoT Engineers
- **My skill level:** Intermediate
- **Stack:** 
Embedded: C/C++, ESP-IDF, nRF SDK
ML/Inference: TensorFlow Lite Micro, ONNX Runtime (host), PyTorch
Backend/Control: Python, FastAPI, gRPC
RL: PyTorch / Stable-Baselines3
Communication: BLE, WiFi (HTTP/gRPC/MQTT)
Infra/Runtime: Docker
Observability: Prometheus, Grafana
Data/Logging: SQLite / Parquet / Pandas

## Commands
- **Install:** `python3 -m venv .venv && .venv/bin/python -m pip install -r host/controller/requirements.txt`
- **Dev:** `.venv/bin/python -m uvicorn host.controller.app:app --reload`
- **Simulate:** `.venv/bin/python scripts/simulate_events.py`
- **Test:** `.venv/bin/python -m pytest`
- **Lint:** Not configured yet

## Do
- Read existing code before modifying anything
- Match existing patterns, naming, and style
- Handle errors gracefully — no silent failures
- Keep changes small and scoped to what was asked
- Run dev/build after changes to verify nothing broke
- Ask clarifying questions before guessing

## Don't
- Install new dependencies without asking
- Delete or overwrite files without confirming
- Hardcode secrets, API keys, or credentials
- Rewrite working code unless explicitly asked
- Push, deploy, or force-push without permission
- Make changes outside the scope of the request

## When Stuck
- If a task is large, break it into steps and confirm the plan first
- If you can't fix an error in 2 attempts, stop and explain the issue

## Testing
- Run existing tests after any change
- Add at least one test for new features
- Never skip or delete tests to make things pass

## Git
- Small, focused commits with descriptive messages
- Never force push

## Response Style
- always respond with clear & concise messages
- use plain English when explaining to the User
- avoid long sentences, complex words, or long paragraphs
