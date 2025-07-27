import argparse
import csv
import datetime
import logging
import socket
import threading
import time

from power_supply import (
    BK9171B,
    Keithley2280S,
    DeviceNotFound,
)
from power_trace import parse_power_script

logger = logging.getLogger('control-power-supply')

stop_flag = threading.Event()

def server_thread():
    host = '127.0.0.1'
    port = 65432

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, port))
        s.listen()
        print(f"Listening for commands on {host}:{port}")

        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")
            data = conn.recv(1024)
            if data.decode() == "STOP":
                print("Received 'STOP' command. Setting stop flag.")
                stop_flag.set()

class StopCommand(BaseException):
    pass

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--script', required=True)
    parser.add_argument('--voltage', type=float, default=0)
    parser.add_argument('--normalized_average_current', type=float, default=0)
    parser.add_argument('--normalized_max_current', type=float, default=0)
    parser.add_argument('--ip', type=str, default='')
    parser.add_argument('--power-trace-log-csv', required=True)
    parser.add_argument('--debug', action='store_true', default=False)
    args = parser.parse_args()

    logging_kwargs = {
        'format': '%(asctime)s.%(msecs)03d %(levelname)-8s %(message)s',
        'datefmt': '%Y-%m-%d %H:%M:%S',
    }
    if args.debug:
        logging_kwargs['level'] = logging.DEBUG
    else:
        logging_kwargs['level'] = logging.INFO
    logging.basicConfig(**logging_kwargs)

    normalized_power_trace = parse_power_script(args.script, args.voltage, args.normalized_average_current, args.normalized_max_current)

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
        run_power_trace(device, normalized_power_trace, writer)

def run_power_trace(device, normalized_power_trace, csv_log_writer):
    try:
        device.output_on()

        start_time = time.time()

        # Start the server thread
        server = threading.Thread(target=server_thread)
        server.start()

        while True:  # repeat the script infinitely
            for step, period, voltage, current in normalized_power_trace:
                device.set_voltage(voltage)
                device.set_current(current)
                logger.info('Set voltage=%f, current=%f', voltage, current)

                device_voltage, device_current = device.get_voltage_and_current()
                now = datetime.datetime.now()
                csv_log_writer.writerow([now, device_voltage, device_current])

                time.sleep(period)
                logger.info('Elapsed time: %f', time.time() - start_time)
                if stop_flag.is_set():
                    raise StopCommand()
    except (KeyboardInterrupt, StopCommand):
        pass
    finally:
        device.output_off()

if __name__ == '__main__':
    main()
