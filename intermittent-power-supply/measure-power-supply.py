import argparse
import csv
import datetime
import time

from power_supply import (
    BK9171B,
    Keithley2280S,
    DeviceNotFound,
)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--ip', type=str, default='')
    parser.add_argument('--power-trace-log-csv', required=True)
    args = parser.parse_args()

    resource_name = None
    if args.ip:
        # https://pyvisa.readthedocs.io/en/1.8/names.html
        resource_name = f'TCPIP::{args.ip}::INSTR'

    try:
        device = Keithley2280S(resource_name)
    except DeviceNotFound:
        device = BK9171B(resource_name)

    with open(args.power_trace_log_csv, 'w', newline='', encoding='utf-8') as log_csv_file:
        writer = csv.writer(log_csv_file)
        writer.writerow(['Timestamp', 'Voltage', 'Current'])

        while True:
            voltage, current = device.get_voltage_and_current()
            now = datetime.datetime.now()
            writer.writerow([now, voltage, current])
            time.sleep(0.1)

if __name__ == '__main__':
    main()
