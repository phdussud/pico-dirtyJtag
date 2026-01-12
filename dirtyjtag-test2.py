import usb1
import time

VID = 0x1209
PID = 0xC0CA
EP_OUT = 0x01
EP_IN  = 0x82

def send_cmd(handle, data):
    handle.bulkWrite(EP_OUT, data)
    try:
        resp = handle.bulkRead(EP_IN, 64, timeout=100)
        print("IN:",resp)
    except usb1.USBErrorTimeout:
        print("No IN response")

def main():
    ctx = usb1.USBContext()
    handle = ctx.openByVendorIDAndProductID(VID, PID,
                                            skip_on_error=True)
    if handle is None:
        print("Device not found")
        return
    with handle.claimInterface(0):
        # CMD_XFER with 11-bit TDI pattern 10110111011
        bit_len = 11
        payload = bytes([0xB7, 0x60])
        cmd_val = 0x03  # CMD_XFER, no modifiers
        send_cmd(handle, bytes([cmd_val, bit_len]) + payload)

if __name__ == "__main__":
    main()