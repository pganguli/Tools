# Common steps

1. `python -m pip install -r requirements.txt`

# Preparation for Linux

1. `sudo cp 50-cp210x.rules 50-Keithley2280S.rules /etc/udev/rules.d/`
2. `sudo udevadm control --reload`
3. Unplug and replug the USB cable


```
$ pyvisa-info
Machine Details:
   Platform ID:    Linux-6.1.27-1-lts-x86_64-with-glibc2.37
   Processor:

Python:
   Implementation: CPython
   Executable:     /4TB/chyen/.cache/virtualenvwrapper/usage-span/bin/python
   Version:        3.9.16
   Compiler:       GCC 12.2.1 20230201
   Bits:           64bit
   Build:          May  2 2023 12:16:03 (#main)
   Unicode:        UCS4

PyVISA Version: 1.13.0

Backends:
   ivi:
      Version: 1.13.0 (bundled with PyVISA)
      Binary library: Not found
   py:
      Version: 0.6.3
      ASRL INSTR:
         Please install PySerial (>=3.0) to use this resource type.
         No module named 'serial'
      USB INSTR: Available via PyUSB (1.2.1). Backend: libusb1
      USB RAW: Available via PyUSB (1.2.1). Backend: libusb1
      TCPIP INSTR: Available
         Resource discovery:
         - VXI-11: ok
         - hislip: disabled (zeroconf not installed)
      VICP INSTR:
         Please install PyVICP to use this resource type.
      TCPIP SOCKET: Available
      GPIB INSTR:
         Please install linux-gpib (Linux) or gpib-ctypes (Windows, Linux) to use this resource type. Note that installing gpib-ctypes will give you access to a broader range of funcionality.
         No module named 'gpib'
```

# Preparation for Windows

1. Download NI-VISA package manager from https://www.ni.com/zh-tw/support/downloads/drivers/download.ni-visa.html
2. Install NI-VISA using the package manager. Note that only NI-VISA is necessary. All additional items can be deselected.
3. For BK9171B, download and install CP2102 driver from https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads

After installing NI-VISA, the USB device, which represents the power supply, in Windows Device Manager should get a driver.

After installation:
```
C:\> pyvisa-info.exe
Machine Details:
   Platform ID:    Windows-10-10.0.19045-SP0
   Processor:      Intel64 Family 6 Model 140 Stepping 1, GenuineIntel

Python:
   Implementation: CPython
   Executable:     D:\venv\Scripts\python.exe
   Version:        3.9.13
   Compiler:       MSC v.1929 64 bit (AMD64)
   Bits:           64bit
   Build:          May 17 2022 16:36:42 (#tags/v3.9.13:6de2ca5)
   Unicode:        UCS4

PyVISA Version: 1.13.0

Backends:
   ivi:
      Version: 1.13.0 (bundled with PyVISA)
      #1: C:\WINDOWS\system32\visa32.dll:
         found by: auto
         bitness: 64
         Vendor: National Instruments
         Impl. Version: 24118016
         Spec. Version: 7340032
      #2: C:\WINDOWS\system32\visa64.dll:
         found by: auto
         bitness: 64
         Vendor: National Instruments
         Impl. Version: 24118016
         Spec. Version: 7340032
```

Known errors:

* `VI_ERROR_INV_OBJECT`: The VISA library is problematic. TekVISA (part of OpenChoice Desktop) has such an issue and NI-VISA does not.
* `VI_ERROR_LIBRARY_NFOUND`: Some dependencies are missing. Please reinstall the VISA library.
