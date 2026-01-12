#!/usr/bin/env python3

#
# Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# sudo pip3 install pyusb

import usb.core
import usb.util
import usb.backend.libusb1 as libusb1

# find our device
be = libusb1.get_backend()
dev = usb.core.find(idVendor=0x1209, idProduct=0xC0CA, backend=be)

# was it found?
if dev is None:
    raise ValueError('Device not found')

# get an endpoint instance
cfg = dev.get_active_configuration()
intf = cfg[(0, 0)]

outep = usb.util.find_descriptor(
    intf,
    # match the first OUT endpoint
    custom_match= \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_OUT)

inep = usb.util.find_descriptor(
    intf,
    # match the first IN endpoint
    custom_match= \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_IN)

assert inep is not None
assert outep is not None

outep.write(bytes([0x05,0x05,0])) #get TDO
#outep.write(bytes([0x05,0])) #get TDO

from_device1 = inep.read(32)
print (from_device1)
from_device2 = inep.read(32)
print (from_device1)
