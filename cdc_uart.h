/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
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

#ifndef CDC_UART_H
#define CDC_UART_H

#include "dirtyJtagConfig.h"

#define USBUSART_BAUDRATE 115200

/* For speed this is set to the USB transfer size */
#define FULL_SWO_PACKET (64)

#define TX_BUFFER_SIZE (4096) //needs to be a power of 2
#define RX_BUFFER_SIZE (4096)


struct uart_device {
    uint index;
    uart_inst_t *inst;
    uint8_t tx_buf[TX_BUFFER_SIZE];
    volatile uint8_t rx_buf[RX_BUFFER_SIZE];
    uint rx_dma_channel;
    uint tx_dma_channel;
    volatile uint8_t *tx_write_address;
    uint8_t *rx_read_address;
    uint n_checks;
    uint is_connected;
} uart_devices[2];



#ifdef USB_CDC_UART_BRIDGE
void cdc_uart_init( uart_inst_t *const uart, int uart_rx_pin, int uart_tx_pin );
void cdc_uart_task(void);
#endif

#endif
