#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pico/multicore.h"
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


//#define MULTICORE

void init_pins()
{
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
typedef uint8_t cmd_buffer[64];
static uint wr_buffer_number = 0;
static uint rd_buffer_number = 0; 
typedef struct buffer_info
{
    volatile uint count;
    volatile bool busy;
    cmd_buffer buffer;
} buffer_info;
buffer_info buffer_infos[2];

static cmd_buffer tx_buf;



void jtag_task()
{
#ifdef MULTICORE
    if (multicore_fifo_rvalid())
    {
        //some command processing has been done
        uint rx_num = multicore_fifo_pop_blocking();
        buffer_info* bi = &buffer_infos[rx_num];
        bi->busy = false;

    }
#endif
    if ((buffer_infos[wr_buffer_number].busy == false)) 
    {
        //If tud_task() is called and tud_vendor_read isn't called immediately (i.e before calling tud_task again)
        //after there is data available, there is a risk that data from 2 BULK OUT transaction will be (partially) combined into one
        //The DJTAG protocol does not tolerate this. 
        tud_task();// tinyusb device task
        if (tud_vendor_available())
        {
            uint bnum = wr_buffer_number;
            uint count = tud_vendor_read(buffer_infos[wr_buffer_number].buffer, 64);
            if (count != 0)
            {
                buffer_infos[bnum].count = count;
                buffer_infos[bnum].busy = true;
                wr_buffer_number = 1 - wr_buffer_number; //switch buffer
#ifdef MULTICORE
                multicore_fifo_push_blocking(bnum);
#endif
            }
        }

    }       

}


#ifdef MULTICORE
void core1_entry() {

    djtag_init();
    while (1)
    {
        uint rx_num = multicore_fifo_pop_blocking();
        buffer_info* bi = &buffer_infos[rx_num];
        assert (bi->busy);
        cmd_handle(&jtag, bi->buffer, bi->count, tx_buf);
        multicore_fifo_push_blocking(rx_num);
    }
 
}
#endif

void fetch_command()
{
#ifndef MULTICORE
    if (buffer_infos[rd_buffer_number].busy)
    {
        cmd_handle(&jtag, buffer_infos[rd_buffer_number].buffer, buffer_infos[rd_buffer_number].count, tx_buf);
        buffer_infos[rd_buffer_number].busy = false;
        rd_buffer_number = 1 - rd_buffer_number; //switch buffer
    }
#endif
}



int main()
{
    board_init();
    tusb_init();

#ifdef MULTICORE
    multicore_launch_core1(core1_entry);
#else 
    djtag_init();
#endif
    while (1) {
        jtag_task();
        fetch_command();//for unicore implementation
    }
}
