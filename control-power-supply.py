import argparse
import logging
import time

from power_supply import (
    BK9171B,
    Keithley2280S,
    DeviceNotFound,
)
from power_trace import parse_power_script

logger = logging.getLogger('control-power-supply')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--script', required=True)
    parser.add_argument('--normalized_average_current', type=float, default=0)
    parser.add_argument('--normalized_max_current', type=float, default=0)
    parser.add_argument('--debug', action='store_true', default=False)
    args = parser.parse_args()

    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    normalized_power_trace = parse_power_script(args.script, args.normalized_average_current, args.normalized_max_current)

    try:
        device = Keithley2280S()
    except DeviceNotFound:
        device = BK9171B()

    try:
        device.output_on()

        start_time = time.time()

        while True:  # repeat the script infinitely
            for step, period, voltage, current in normalized_power_trace:
                device.set_voltage(voltage)
                device.set_current(current)
                time.sleep(period)
                logger.info('Elapsed time: %f', time.time() - start_time)
    except KeyboardInterrupt:
        device.output_off()

if __name__ == '__main__':
    main()
