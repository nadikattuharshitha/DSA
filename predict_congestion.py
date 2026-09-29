"""
AI traffic prediction module for the Intelligent Network Packet Routing Simulator.

The C program sends each router's congestion history (oldest -> newest) on stdin,
one router per line:

    <name> <queue_size> <reading1> <reading2> ... <readingN>

For every router this script fits a linear regression (least squares) to the
history, forecasts the load for the next few checks, and prints a prediction.
Pure Python, no third-party packages needed.
"""
import sys

FORECAST_STEPS = 3        # how many checks ahead to predict
CONGESTION_RATIO = 0.8    # same threshold as the C program (80% of queue)
MODERATE_RATIO = 0.5


def linear_regression(y):
    """Least-squares fit y = slope * x + intercept for x = 0..n-1."""
    n = len(y)
    xs = range(n)
    mean_x = (n - 1) / 2.0
    mean_y = sum(y) / n
    denom = sum((x - mean_x) ** 2 for x in xs)
    if denom == 0:
        return 0.0, mean_y
    slope = sum((x - mean_x) * (v - mean_y) for x, v in zip(xs, y)) / denom
    intercept = mean_y - slope * mean_x
    return slope, intercept


def predict(readings, queue_size):
    n = len(readings)
    slope, intercept = linear_regression(readings)
    forecasts = []
    for step in range(1, FORECAST_STEPS + 1):
        value = slope * (n - 1 + step) + intercept
        forecasts.append(max(0.0, min(float(queue_size), value)))  # clamp to [0, queue]
    return slope, forecasts


def classify(slope, latest, forecasts, queue_size):
    peak = max(forecasts)
    if peak >= queue_size * CONGESTION_RATIO and slope > 0:
        return "LIKELY TO CONGEST SOON"
    if latest >= queue_size * CONGESTION_RATIO and slope >= 0:
        return "STAYING CONGESTED"
    if slope > 0.05:
        return "RISING, but not urgent"
    return "STABLE / IMPROVING"


def main():
    print("\n------- CONGESTION PREDICTION (Python AI) -------")
    any_router = False
    for line in sys.stdin:
        parts = line.split()
        if len(parts) < 3:
            continue
        any_router = True
        name = parts[0]
        try:
            queue_size = int(parts[1])
            readings = [int(v) for v in parts[2:]]
        except ValueError:
            print(f"Router {name} : invalid data received.")
            continue

        if len(readings) < 2:
            print(f"Router {name} : not enough data yet "
                  f"(run 'Check Congestion' a few more times).")
            continue

        slope, forecasts = predict(readings, queue_size)
        latest = readings[-1]
        verdict = classify(slope, latest, forecasts, queue_size)
        fc = ", ".join(f"{f:.1f}" for f in forecasts)
        print(f"Router {name} : trend = {slope:+.2f} packets/check, "
              f"current = {latest}/{queue_size}, "
              f"next {FORECAST_STEPS} checks -> [{fc}] -> {verdict}")

    if not any_router:
        print("No router data received.")


if __name__ == "__main__":
    main()
