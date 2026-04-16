from fastapi.testclient import TestClient

from host.controller.app import app, event_store, runtime_metrics
from host.controller.decision import RuleBasedController
from host.controller.models import EventPriority, EventState, RuntimeAction


def test_low_confidence_events_are_offloaded() -> None:
    decision = RuleBasedController().decide(EventState(local_confidence=0.4))
    assert decision.action == RuntimeAction.offload
    assert decision.reason == "low_local_confidence"


def test_host_unreachable_enters_degraded_mode() -> None:
    decision = RuleBasedController().decide(EventState(host_reachable=False))
    assert decision.action == RuntimeAction.degraded_mode
    assert decision.reason == "host_unreachable"


def test_high_priority_events_are_escalated() -> None:
    decision = RuleBasedController().decide(
        EventState(local_confidence=0.95, event_priority=EventPriority.high)
    )
    assert decision.action == RuntimeAction.offload
    assert decision.reason == "priority_escalation"


def test_health_endpoint() -> None:
    client = TestClient(app)
    response = client.get("/health")
    assert response.status_code == 200
    assert response.json()["status"] == "ok"


def test_decision_endpoint_returns_runtime_action() -> None:
    runtime_metrics.reset()
    client = TestClient(app)
    response = client.post("/decision", json={"local_confidence": 0.95})
    assert response.status_code == 200
    assert response.json()["action"] == RuntimeAction.run_local.value


def test_event_endpoint_records_decision_and_updates_metrics(tmp_path) -> None:
    runtime_metrics.reset()
    event_store.path = tmp_path / "events.jsonl"
    client = TestClient(app)
    response = client.post(
        "/event",
        json={
            "event_id": "test_event_001",
            "local_confidence": 0.45,
            "local_latency_ms": 22.5,
            "queue_depth": 3,
        },
    )
    assert response.status_code == 200
    assert response.json()["action"] == RuntimeAction.offload.value

    metrics_response = client.get("/metrics")
    assert metrics_response.status_code == 200
    metrics = metrics_response.json()
    assert metrics["total_events_received"] == 1
    assert metrics["total_decisions_made"] == 1
    assert metrics["latest_local_confidence"] == 0.45
    assert metrics["latest_local_latency_ms"] == 22.5
    assert metrics["latest_queue_depth"] == 3
    assert metrics["latest_decision_action"] == RuntimeAction.offload.value


def test_fallback_inference_returns_placeholder_result_and_updates_metrics() -> None:
    runtime_metrics.reset()
    client = TestClient(app)
    response = client.post(
        "/infer/fallback",
        json={"event_id": "fallback_001", "features": [0.8, 0.7, 0.9]},
    )
    assert response.status_code == 200
    body = response.json()
    assert body["event_id"] == "fallback_001"
    assert body["prediction"] == "anomaly"
    assert body["confidence"] > 0
    assert body["model_id"] == "host-fallback-baseline-v0"

    metrics = client.get("/metrics").json()
    assert metrics["total_fallback_inference_requests"] == 1


def test_metrics_endpoint_returns_baseline_counters() -> None:
    runtime_metrics.reset()
    client = TestClient(app)
    response = client.get("/metrics")
    assert response.status_code == 200
    assert response.json() == {
        "total_events_received": 0,
        "total_decisions_made": 0,
        "total_fallback_inference_requests": 0,
        "latest_local_confidence": None,
        "latest_local_latency_ms": None,
        "latest_queue_depth": None,
        "latest_decision_action": None,
    }
