"""
Power-trace script parser and plotter helpers.

parse_power_script(script, ...):
  Reads a CSV power-trace file (step, period_ms, voltage_V, current_A) and
  returns a list of (voltage, current, period_ms) tuples.  Supports optional
  current normalization so that scripts recorded at one current level can be
  scaled to a different supply output.

trace_to_plot(trace):
  Converts the trace list into arrays suitable for matplotlib step-plot
  visualisation.

Used by plot-trace.py and measure-power-supply.py.
"""

import csv
import logging
from typing import Optional

logger = logging.getLogger("power_trace")


def parse_power_script(
    script: str,
    override_voltage: float = 0,
    normalized_average_current: Optional[float] = 0,
    normalized_max_current: Optional[float] = 0,
):
    with open(script, "r", newline="") as csvfile:
        reader = csv.reader(csvfile, delimiter=",")
        rows = list(reader)[1:]  # skip the header row

    if normalized_average_current:
        current_sum = 0
        for row in rows:
            step, period, voltage, current = row
            current_sum += float(current)
        average_current = current_sum / len(rows)
        logger.info("Average current=%f", average_current)
    elif normalized_max_current:
        max_current = 0
        for row in rows:
            step, period, voltage, current = row
            max_current = max(max_current, float(current))
        logger.info("Max current=%f", max_current)

    normalized_power_trace = []
    for row in rows:
        step, period, voltage, current = row
        if period.endswith("sec"):
            period = float(period[: -len("sec")])
        voltage = float(voltage)
        if override_voltage:
            voltage = override_voltage
        current = float(current)

        orig_current = current
        if normalized_average_current:
            current = current / average_current * normalized_average_current
        elif normalized_max_current:
            current = current / max_current * normalized_max_current
        # device_voltage = float(device.get_voltage())
        # current = voltage * current / device_voltage
        logger.debug("Normalize %f => %f", orig_current, current)
        normalized_power_trace.append((step, period, voltage, current))

    return normalized_power_trace


def trace_to_plot(power_trace):
    x = []
    y = []
    last_period = None

    for step, period, voltage, current in power_trace:
        if x:
            x.append(x[-1] + last_period)
        else:
            x.append(0)
        last_period = period
        y.append(voltage * current * 1000)

    return x, y
