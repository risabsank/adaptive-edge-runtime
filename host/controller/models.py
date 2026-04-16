from __future__ import annotations

from enum import Enum
from time import time
from typing import Any
from uuid import uuid4

from pydantic import BaseModel, Field


class EventPriority(str, Enum):
    low = "low"
    normal = "normal"
    high = "high"
    critical = "critical"


class RuntimeAction(str, Enum):
    run_local = "run_local"
    offload = "offload"
    use_small_model = "use_small_model"
    use_large_model = "use_large_model"
    batch = "batch"
    retry = "retry"
    degraded_mode = "degraded_mode"


class EventState(BaseModel):
    event_id: str = Field(default_factory=lambda: f"evt_{uuid4().hex[:12]}")
    source: str = "simulator"
    timestamp_ms: int = Field(default_factory=lambda: int(time() * 1000))
    features: list[float] = Field(default_factory=list)
    local_prediction: str | None = None
    local_confidence: float = Field(default=1.0, ge=0.0, le=1.0)
    local_latency_ms: float = Field(default=0.0, ge=0.0)
    queue_depth: int = Field(default=0, ge=0)
    event_priority: EventPriority = EventPriority.normal
    host_reachable: bool = True
    recent_failures: int = Field(default=0, ge=0)


class RuntimeDecision(BaseModel):
    action: RuntimeAction
    reason: str
    timeout_ms: int = Field(default=250, ge=0)
    metadata: dict[str, Any] = Field(default_factory=dict)


class FallbackInferenceRequest(BaseModel):
    event_id: str = Field(default_factory=lambda: f"evt_{uuid4().hex[:12]}")
    features: list[float] = Field(default_factory=list)


class FallbackInferenceResponse(BaseModel):
    event_id: str
    prediction: str
    confidence: float
    model_id: str
