from __future__ import annotations

from threading import Lock
from typing import Any

from host.controller.models import EventState, RuntimeDecision


class RuntimeMetrics:
    """In-memory host metrics for the minimal runtime skeleton."""

    def __init__(self) -> None:
        self._lock = Lock()
        self.reset()

    def reset(self) -> None:
        with self._lock:
            self.total_events_received = 0
            self.total_decisions_made = 0
            self.total_fallback_inference_requests = 0
            self.latest_local_confidence: float | None = None
            self.latest_local_latency_ms: float | None = None
            self.latest_queue_depth: int | None = None
            self.latest_decision_action: str | None = None

    def record_event(self, state: EventState) -> None:
        with self._lock:
            self.total_events_received += 1
            self.latest_local_confidence = state.local_confidence
            self.latest_local_latency_ms = state.local_latency_ms
            self.latest_queue_depth = state.queue_depth

    def record_decision(self, decision: RuntimeDecision) -> None:
        with self._lock:
            self.total_decisions_made += 1
            self.latest_decision_action = decision.action.value

    def record_fallback_request(self) -> None:
        with self._lock:
            self.total_fallback_inference_requests += 1

    def snapshot(self) -> dict[str, Any]:
        with self._lock:
            return {
                "total_events_received": self.total_events_received,
                "total_decisions_made": self.total_decisions_made,
                "total_fallback_inference_requests": self.total_fallback_inference_requests,
                "latest_local_confidence": self.latest_local_confidence,
                "latest_local_latency_ms": self.latest_local_latency_ms,
                "latest_queue_depth": self.latest_queue_depth,
                "latest_decision_action": self.latest_decision_action,
            }

