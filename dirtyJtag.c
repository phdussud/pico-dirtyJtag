#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pio_jtag.h"
#include "bsp/board.h"
#include "get_serial.h"
#include "tusb.h"
#include "cmd.h"

#define PIN_TDI 16 
#define PIN_TDO 17
#define PIN_TCK 18
#define PIN_TMS 19
#define PIN_RST 20
#define PIN_TRST 21
#define PIN_SYNC 22

void init_pins()
{
    gpio_init(PIN_SYNC);
    gpio_set_dir(PIN_SYNC, GPIO_OUT);
}

pio_jtag_inst_t jtag = {
            .pio = pio0,
            .sm = 0
};

void djtag_init()
{
    init_pins();
    init_jtag(&jtag, 1000, PIN_TCK, PIN_TDI, PIN_TDO, PIN_TMS, PIN_RST, PIN_TRST);
}

static uint8_t rx_buf[64];
static uint8_t tx_buf[64];

void jtag_task(pio_jtag_inst_t* jtag)
{
    if ( tud_vendor_available() ) {
        uint count = tud_vendor_read(rx_buf, 64);
        if (count == 0) {
            return;
        }
        cmd_handle(jtag, rx_buf, count, tx_buf);
    }       

}


int main()
{
//  stdio_init_all();
//  usb_serial_init();
    board_init();
    tusb_init();
    djtag_init();
    while (1) {
        tud_task(); // tinyusb device task
        jtag_task(&jtag);
    }
}
