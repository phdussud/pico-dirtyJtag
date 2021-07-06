# pico-dirtyJtag

This is a port of the excellent [DirtyJtag project](https://github.com/jeanthom/DirtyJTAG)   
It uses the PIO unit to produce and capture the JTAG signals. 

The pinout is as follows:

| Pin name | GPIO   |
|:---------|:-------|
| TDI      | GPIO16 |
| TDO      | GPIO17 |
| TCK      | GPIO18 |
| TMS      | GPIO19 |
| RST      | GPIO20 |
| TRST     | GPIO21 |

![Pinout image](doc/pinout.png)

## Building pico-dirtyJtag

Follow [the instructions](https://github.com/raspberrypi/pico-sdk) for installing the official Raspberry Pi Pico SDK, then clone this repository and use the following commands:

```
mkdir -p build
cd build
cmake ..
make
```

If everything succeeds you should have a `dirtyJtag.uf2` file that you can directly upload to the Pi Pico.
