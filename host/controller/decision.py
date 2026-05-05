from host.controller.models import EventPriority, EventState, RuntimeAction, RuntimeDecision


class RuleBasedController:
    """Deterministic runtime controller used before RL is introduced."""

    def __init__(
        self,
        confidence_threshold: float = 0.70,
        degraded_confidence_threshold: float = 0.85,
        latency_budget_ms: float = 50.0,
        max_queue_depth: int = 8,
        overload_queue_depth: int = 12,
        max_recent_failures: int = 2,
        degraded_sampling_interval_ms: int = 4000,
    ) -> None:
        self.confidence_threshold = confidence_threshold
        self.degraded_confidence_threshold = degraded_confidence_threshold
        self.latency_budget_ms = latency_budget_ms
        self.max_queue_depth = max_queue_depth
        self.overload_queue_depth = overload_queue_depth
        self.max_recent_failures = max_recent_failures
        self.degraded_sampling_interval_ms = degraded_sampling_interval_ms

    def decide(self, state: EventState) -> RuntimeDecision:
        metadata = {
            "confidence_threshold": self.confidence_threshold,
            "degraded_confidence_threshold": self.degraded_confidence_threshold,
            "latency_budget_ms": self.latency_budget_ms,
            "max_queue_depth": self.max_queue_depth,
            "overload_queue_depth": self.overload_queue_depth,
            "max_recent_failures": self.max_recent_failures,
            "degraded_sampling_interval_ms": self.degraded_sampling_interval_ms,
        }

        if state.recent_failures > self.max_recent_failures:
            return RuntimeDecision(
                action=RuntimeAction.degraded_mode,
                reason="repeated_failures_degraded_mode",
                timeout_ms=0,
                metadata={
                    **metadata,
                    "active_confidence_threshold": self.degraded_confidence_threshold,
                    "fallback_mode": "stay_local",
                    "sampling_interval_ms": self.degraded_sampling_interval_ms,
                },
            )

        if not state.host_reachable:
            return RuntimeDecision(
                action=RuntimeAction.degraded_mode,
                reason="host_unreachable_stay_local",
                timeout_ms=0,
                metadata={
                    **metadata,
                    "active_confidence_threshold": self.degraded_confidence_threshold,
                    "fallback_mode": "stay_local",
                    "sampling_interval_ms": self.degraded_sampling_interval_ms,
                },
            )

        if state.event_priority in {EventPriority.high, EventPriority.critical}:
            return RuntimeDecision(
                action=RuntimeAction.offload,
                reason="priority_escalation",
                timeout_ms=750 if state.event_priority == EventPriority.critical else 500,
                metadata=metadata,
            )

        if state.queue_depth >= self.overload_queue_depth:
            return RuntimeDecision(
                action=RuntimeAction.batch,
                reason="queue_depth_overload_reduce_sampling",
                timeout_ms=500,
                metadata={
                    **metadata,
                    "sampling_interval_ms": self.degraded_sampling_interval_ms,
                },
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
                timeout_ms=250,
                metadata={
                    **metadata,
                    "sampling_interval_ms": 2000,
                },
            )

        if state.local_latency_ms > self.latency_budget_ms:
            return RuntimeDecision(
                action=RuntimeAction.use_small_model,
                reason="local_latency_over_budget",
                timeout_ms=250,
                metadata={
                    **metadata,
                    "target_model": "small",
                },
            )

        return RuntimeDecision(
            action=RuntimeAction.run_local,
            reason="local_inference_acceptable",
            timeout_ms=250,
            metadata=metadata,
        )
