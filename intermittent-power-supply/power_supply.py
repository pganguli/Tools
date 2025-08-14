import logging
from typing import List, Optional

import pyvisa
from serial.tools.list_ports import comports

logger = logging.getLogger('power_supply')

class DeviceNotFound(Exception):
    pass

class Device:
    BAUD_RATE: Optional[int] = None
    IDENTIFIERS: List[str]

    def __init__(self, resource_name: Optional[str] = None):
        rm = pyvisa.ResourceManager()

        if resource_name is None:
            resource_name = self._find_resource(rm)

        self.inst: pyvisa.Resource = rm.open_resource(resource_name)

        if self.BAUD_RATE:
            self.inst.baud_rate = self.BAUD_RATE
        print(self._query('*IDN?'))

    def _find_resource(self, rm):
        resources = rm.list_resources()
        print(resources)

        for identifier in self.IDENTIFIERS:
            try:
                return next(
                    resource for resource in resources if identifier in resource)
                break
            except StopIteration:
                continue

        serial_ports_info = comports()
        for port_info in serial_ports_info:
            if not port_info.vid or not port_info.pid:
                # Not a USB device, ignoring
                continue
            if f'0x{port_info.vid:04X}::0x{port_info.pid:04X}' in self.IDENTIFIERS:
                return port_info.name

        raise DeviceNotFound

    def _write_command(self, command: str):
        logger.debug('Sending command %s', command)
        self.inst.write(command)

    def _query(self, query: str) -> str:
        logger.debug('Query %s', query)
        ret = self.inst.query(query)
        logger.debug('Result: %s', ret)
        return ret

    def output_on(self):
        raise NotImplementedError

    def output_off(self):
        raise NotImplementedError

    def set_voltage(self, voltage):
        raise NotImplementedError

    def set_current(self, voltage):
        raise NotImplementedError

    def get_voltage_and_current(self) -> tuple[float, float]:
        raise NotImplementedError

class BK9171B(Device):
    # Also uses SCPI? See "4.2 Remote Commands" of 9170B_9180B_Series_manual.pdf
    BAUD_RATE = 57600  # value from BK Precision's software
    IDENTIFIERS = [
        '0x10C4::0xEA60',
    ]

    def output_on(self):
        self._write_command('OUT 1')

    def output_off(self):
        self._write_command('OUT 0')

    def set_voltage(self, voltage):
        self._write_command(f'VSET {voltage}')

    def set_current(self, current):
        if current != 0 and current < 0.001:
            current = 0.001
        current = round(current * 1000) / 1000
        self._write_command(f'ISET {current:.4f}')

    def get_voltage_and_current(self) -> tuple[float, float]:
        voltage = float(self._query('VOUT1?').strip())
        current = float(self._query('IOUT1?').strip())
        return voltage, current

class SCPIDevice(Device):
    # Implements SCPI (Standard Commands for Programmable Instruments)
    # Ref: https://www.ivifoundation.org/downloads/SCPI/scpi-99.pdf

    def output_on(self):
        # 15.12
        self._write_command('OUTPut:STATe ON')

    def output_off(self):
        # 15.12
        self._write_command('OUTPut:STATe OFF')

    def set_voltage(self, voltage):
        # 19.23.4.1.1
        self._write_command(f'SOURce:VOLTage:LEVel:IMMediate:AMPLitude {voltage}')

    def set_current(self, current):
        # 19.5.4.1.1
        self._write_command(f'SOURce:CURRent:LEVel:IMMediate:AMPLitude {current}')

    def get_voltage_and_current(self) -> tuple[float, float]:
        # 3.7.2
        # 3.8.1.3
        current_str, voltage_str = self._query('MEASure:VOLTage:DC?').split(',', maxsplit=2)[:2]

        assert current_str.endswith('A') and voltage_str.endswith('V')

        current = float(current_str[:-1])
        voltage = float(voltage_str[:-1])

        return voltage, current


class Keithley2280S(SCPIDevice):
    IDENTIFIERS = [
        '1510::8832',  # decimal of its USB ID 05e6:2280, used by pyvisa-py
        '0x05E6::0x2280',  # used by NI-VISA
    ]
