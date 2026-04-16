from host.controller.models import EventPriority, EventState, RuntimeAction, RuntimeDecision


class RuleBasedController:
    """Deterministic baseline controller for Phase 1."""

    def __init__(
        self,
        confidence_threshold: float = 0.70,
        latency_budget_ms: float = 50.0,
        max_queue_depth: int = 8,
        max_recent_failures: int = 2,
    ) -> None:
        self.confidence_threshold = confidence_threshold
        self.latency_budget_ms = latency_budget_ms
        self.max_queue_depth = max_queue_depth
        self.max_recent_failures = max_recent_failures

    def decide(self, state: EventState) -> RuntimeDecision:
        metadata = {
            "confidence_threshold": self.confidence_threshold,
            "latency_budget_ms": self.latency_budget_ms,
            "max_queue_depth": self.max_queue_depth,
            "max_recent_failures": self.max_recent_failures,
        }

        if not state.host_reachable:
            return RuntimeDecision(
                action=RuntimeAction.degraded_mode,
                reason="host_unreachable",
                timeout_ms=0,
                metadata=metadata,
            )

        if state.recent_failures > self.max_recent_failures:
            return RuntimeDecision(
                action=RuntimeAction.retry,
                reason="recent_failures_exceeded",
                timeout_ms=500,
                metadata=metadata,
            )

        if state.event_priority in {EventPriority.high, EventPriority.critical}:
            return RuntimeDecision(
                action=RuntimeAction.offload,
                reason="priority_escalation",
                timeout_ms=750 if state.event_priority == EventPriority.critical else 500,
                metadata=metadata,
            )

        if state.local_confidence < self.confidence_threshold:
            return RuntimeDecision(
                action=RuntimeAction.offload,
                reason="low_local_confidence",
                timeout_ms=500,
                metadata=metadata,
            )

        if state.queue_depth >= self.max_queue_depth:
            return RuntimeDecision(
                action=RuntimeAction.batch,
                reason="queue_depth_pressure",
                timeout_ms=100,
                metadata=metadata,
            )

        if state.local_latency_ms > self.latency_budget_ms:
            return RuntimeDecision(
                action=RuntimeAction.use_small_model,
                reason="local_latency_over_budget",
                timeout_ms=250,
                metadata=metadata,
            )

        return RuntimeDecision(
            action=RuntimeAction.run_local,
            reason="local_inference_acceptable",
            timeout_ms=250,
            metadata=metadata,
        )

