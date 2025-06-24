/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2025 Patrick Dussud
 * Copyright (c) 2021 jeanthom 
 * Copyright (c) 2023 David Williams (davidthings)
 * Copyright (c) 2023 Chandler Klüser
 * Copyright (c) 2024 DangerousPrototypes
 * Copyright (c) 2024 DESKTOP-M9CCUTI\ian
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "pico/multicore.h"
#include "pio_jtag.h"
#include "cdc_uart.h"
#include "led.h"
#include "bsp/board.h"
#include "tusb.h"
#include "cmd.h"
#include "get_serial.h"
#include "util.h"

#include "dirtyJtagConfig.h"

#define MULTICORE

void init_pins()
{
    bi_decl(bi_4pins_with_names(PIN_TCK, "TCK", PIN_TDI, "TDI", PIN_TDO, "TDO", PIN_TMS, "TMS"));
    #if !( BOARD_TYPE == BOARD_QMTECH_RP2040_DAUGHTERBOARD )
    bi_decl(bi_2pins_with_names(PIN_RST, "RST", PIN_TRST, "TRST"));
    #endif
}

pio_jtag_inst_t jtag = {
            .pio = pio0,
            .sm = 0
};

void djtag_init()
{
    init_pins();
    #if !( BOARD_TYPE == BOARD_QMTECH_RP2040_DAUGHTERBOARD )
    init_jtag(&jtag, 1000, PIN_TCK, PIN_TDI, PIN_TDO, PIN_TMS, PIN_RST, PIN_TRST);
    #else
    init_jtag(&jtag, 1000, PIN_TCK, PIN_TDI, PIN_TDO, PIN_TMS, 255, 255);
    #endif
}
typedef uint8_t cmd_buffer[64];
static uint wr_buffer_number = 0;
static uint rd_buffer_number = 0; 
typedef struct buffer_info
{
    volatile uint8_t count;
    volatile uint8_t busy;
    cmd_buffer buffer;
} buffer_info;

#define n_buffers (4)

buffer_info buffer_infos[n_buffers];

static cmd_buffer tx_buf;

void jtag_main_task()
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
        
        // Get the number of available bytes and only transfer that many,
        // instead of setting an arbitrary value.
        uint32_t bytes_available = 0;
        if (bytes_available = tud_vendor_available())
        {
            led_rx( 1 );
            uint bnum = wr_buffer_number;
            uint count = tud_vendor_read(buffer_infos[wr_buffer_number].buffer, 64);
            if (count != 0)
            {
/*
#if ( PDJ_DEBUG_LEVEL > 0 )
                printf("jtag_main_task: bytes_available: %d\n", bytes_available);
                printf("jtag_main_task: Printing bytes read:\n");
                print_buffer(buffer_infos[wr_buffer_number].buffer, bytes_available);
#endif
*/
                buffer_infos[bnum].count = count;
                buffer_infos[bnum].busy = true;
                wr_buffer_number = wr_buffer_number + 1; //switch buffer
                if (wr_buffer_number == n_buffers)
                {
                    wr_buffer_number = 0; 
                }
#ifdef MULTICORE
                multicore_fifo_push_blocking(bnum);
#endif
            }
            led_rx( 0 );
        } else {
#if ( CDC_UART_INTF_COUNT > 0 )           
            cdc_uart_task();
#endif
        }
    }
}

void jtag_task()
{
#ifndef MULTICORE
    jtag_main_task();
#endif
}

#ifdef MULTICORE
void core1_entry() {

    djtag_init();

/*
#if ( PDJ_DEBUG_LEVEL > 0 )
    char pf_a[15] = "core1 rx_buf ";
    char pf_b[15] = "core1 tx_buf ";
#endif
*/

    while (1)
    {
        uint rx_num = multicore_fifo_pop_blocking();
        buffer_info* bi = &buffer_infos[rx_num];
        assert (bi->busy);
        cmd_handle(&jtag, bi->buffer, bi->count, tx_buf);
/*
#if ( PDJ_DEBUG_LEVEL > 0 )
        printf("core1_entry: count: %d\n", bi->count);
        printf("core1_entry: Printing rx_buf:\n");
        print_buffer_pf(bi->buffer, bi->count, pf_a);
        printf("core1_entry: Printing tx_buf:\n");
        print_buffer_pf(tx_buf, bi->count, pf_b);
#endif
*/
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
        rd_buffer_number++; //switch buffer
        if (rd_buffer_number == n_buffers)
        {
            rd_buffer_number = 0; 
        }
    }
#endif
}

//this is to work around the fact that tinyUSB does not handle setup request automatically
//Hence this boiler plate code
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
    if (stage != CONTROL_STAGE_SETUP) return true;
    return false;
}

int main()
{
    board_init();
    usb_serial_init();
    tusb_init();

#if ( PDJ_DEBUG_LEVEL > 0 )
    stdio_init_all();
#endif

    led_init( LED_INVERTED, PIN_LED_TX, PIN_LED_RX, PIN_LED_ERROR );
#if ( CDC_UART_INTF_COUNT > 0 )
    cdc_uart_init( 0, PIN_UART0, PIN_UART0_RX, PIN_UART0_TX );
#endif
#if ( CDC_UART_INTF_COUNT > 1)
    cdc_uart_init( 1, PIN_UART1, PIN_UART1_RX, PIN_UART1_TX );
#endif


#ifdef MULTICORE
    multicore_launch_core1(core1_entry);
#else 
    djtag_init();
#endif
    while (1) {
        jtag_main_task();
        fetch_command();//for unicore implementation
    }
}
