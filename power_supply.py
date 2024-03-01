import logging
from typing import List, Optional

import pyvisa

logger = logging.getLogger('power_supply')

class DeviceNotFound(Exception):
    pass

class Device:
    BAUD_RATE: Optional[int] = None
    IDENTIFIERS: List[str]

    def __init__(self):
        resource_name = None

        rm = pyvisa.ResourceManager()
        resources = rm.list_resources()
        print(resources)

        for identifier in self.IDENTIFIERS:
            try:
                resource_name = next(
                    resource for resource in resources if identifier in resource)
                break
            except StopIteration:
                continue
        if not resource_name:
            raise DeviceNotFound

        self.inst = rm.open_resource(resource_name)

        if self.BAUD_RATE:
            self.inst.baud_rate = self.BAUD_RATE
        print(self._query('*IDN?'))

    def _write_command(self, command: str):
        logger.debug('Sending command %s', command)
        self.inst.write(command)

    def _query(self, query: str):
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

class BK9171B(Device):
    # Also uses SCPI? See "4.2 Remote Commands" of 9170B_9180B_Series_manual.pdf
    BAUD_RATE = 57600  # value from BK Precision's software
    IDENTIFIERS = ['ttyUSB', 'COM3']

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

    def get_voltage(self):
        return self._query('VOUT1?')

    def get_current(self):
        return self._query('IOUT1?')

class SCPIDevice(Device):
    # Implements SCPI (Standard Commands for Programmable Instruments)
    # Ref: https://www.ivifoundation.org/docs/scpi-99.pdf

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

class Keithley2280S(SCPIDevice):
    IDENTIFIERS = [
        '1510::8832',  # decimal of its USB ID 05e6:2280, used by pyvisa-py
        '0x05E6::0x2280',  # used by NI-VISA
    ]
