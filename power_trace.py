import csv
import logging
from typing import Optional

logger = logging.getLogger('power_trace')

def parse_power_script(
    script: str,
    normalized_average_current: Optional[float] = 0,
    normalized_max_current: Optional[float] = 0,
):
    with open(script, 'r', newline='') as csvfile:
        reader = csv.reader(csvfile, delimiter=',')
        rows = list(reader)[1:]  # skip the header row

    if normalized_average_current:
        current_sum = 0
        for row in rows:
            step, period, voltage, current = row
            current_sum += float(current)
        average_current = current_sum / len(rows)
        logger.info('Average current=%f', average_current)
    elif normalized_max_current:
        max_current = 0
        for row in rows:
            step, period, voltage, current = row
            max_current = max(max_current, float(current))
        logger.info('Max current=%f', max_current)

    normalized_power_trace = []
    for row in rows:
        step, period, voltage, current = row
        if period.endswith('sec'):
            period = float(period[:-len('sec')])
        voltage = float(voltage)
        current = float(current)

        orig_current = current
        if normalized_average_current:
            current = current / average_current * normalized_average_current
        elif normalized_max_current:
            current = current / max_current * normalized_max_current
        # device_voltage = float(device.get_voltage())
        # current = voltage * current / device_voltage
        logger.debug('Normalize %f => %f', orig_current, current)
        normalized_power_trace.append((step, period, voltage, current))

    return normalized_power_trace
