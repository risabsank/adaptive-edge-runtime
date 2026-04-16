from fastapi.testclient import TestClient

from host.controller.app import app
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

