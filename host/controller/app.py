from __future__ import annotations

from time import time

from fastapi import FastAPI

from host.controller.decision import RuleBasedController
from host.controller.logging_store import JsonlEventStore
from host.controller.metrics import RuntimeMetrics
from host.controller.models import (
    EventState,
    FallbackInferenceRequest,
    FallbackInferenceResponse,
    RuntimeDecision,
)

app = FastAPI(
    title="Adaptive Edge AI Runtime Controller",
    version="0.1.0",
    description="Host-side controller for adaptive edge inference decisions.",
)

controller = RuleBasedController()
event_store = JsonlEventStore()
runtime_metrics = RuntimeMetrics()


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok", "service": "adaptive-edge-runtime-controller"}


@app.get("/metrics")
def metrics() -> dict:
    return runtime_metrics.snapshot()


@app.post("/decision", response_model=RuntimeDecision)
def decide(state: EventState) -> RuntimeDecision:
    decision = controller.decide(state)
    runtime_metrics.record_decision(decision)
    return decision


@app.post("/event", response_model=RuntimeDecision)
def record_event(state: EventState) -> RuntimeDecision:
    decision = controller.decide(state)
    runtime_metrics.record_event(state)
    runtime_metrics.record_decision(decision)
    event_store.append(
        {
            "received_at_ms": int(time() * 1000),
            "event": state.model_dump(mode="json"),
            "decision": decision.model_dump(mode="json"),
        }
    )
    return decision


@app.post("/infer/fallback", response_model=FallbackInferenceResponse)
def fallback_inference(request: FallbackInferenceRequest) -> FallbackInferenceResponse:
    runtime_metrics.record_fallback_request()
    score = sum(request.features) / len(request.features) if request.features else 0.0
    prediction = "anomaly" if score >= 0.65 else "normal"
    confidence = min(0.99, max(0.50, abs(score - 0.5) + 0.5))
    return FallbackInferenceResponse(
        event_id=request.event_id,
        prediction=prediction,
        confidence=round(confidence, 4),
        model_id="host-fallback-baseline-v0",
    )
