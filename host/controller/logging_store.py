from __future__ import annotations

import json
from pathlib import Path
from typing import Any


class JsonlEventStore:
    def __init__(self, path: str | Path = "experiments/traces/events.jsonl") -> None:
        self.path = Path(path)
        self.path.parent.mkdir(parents=True, exist_ok=True)

    def append(self, record: dict[str, Any]) -> None:
        with self.path.open("a", encoding="utf-8") as handle:
            handle.write(json.dumps(record, sort_keys=True) + "\n")
