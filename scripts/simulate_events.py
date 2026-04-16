import random
from time import sleep, time

import requests


CONTROLLER_URL = "http://127.0.0.1:8000/event"


def make_event(index: int) -> dict:
    features = [round(random.random(), 4) for _ in range(3)]
    confidence = round(random.uniform(0.45, 0.95), 4)
    latency = round(random.uniform(8, 75), 2)
    return {
        "event_id": f"sim_{index:04d}",
        "source": "simulator",
        "timestamp_ms": int(time() * 1000),
        "features": features,
        "local_prediction": "anomaly" if sum(features) / len(features) >= 0.65 else "normal",
        "local_confidence": confidence,
        "local_latency_ms": latency,
        "queue_depth": random.randint(0, 12),
        "event_priority": random.choice(["low", "normal", "normal", "high"]),
        "host_reachable": True,
        "recent_failures": random.choice([0, 0, 0, 1, 2, 3]),
    }


def main() -> None:
    for index in range(20):
        event = make_event(index)
        response = requests.post(CONTROLLER_URL, json=event, timeout=2)
        response.raise_for_status()
        decision = response.json()
        print(f"{event['event_id']} -> {decision['action']} ({decision['reason']})")
        sleep(0.25)


if __name__ == "__main__":
    main()

