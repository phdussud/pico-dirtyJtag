/*
 * This file is part of the Black Magic Debug project.
 *
 * Based on work that is Copyright (C) 2017 Black Sphere Technologies Ltd.
 * Copyright (C) 2017 Dave Marples <dave@marples.net>
 * Copyright (c) 2020-2025 Patrick Dussud
 * Copyright (c) 2023 David Williams (davidthings)
 * Copyright (c) 2023 luyi1888
 * Copyright (c) 2024 DangerousPrototypes
 * Copyright (c) 2024 DESKTOP-M9CCUTI\ian
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.	 If not, see <http://www.gnu.org/licenses/>.
 */

#include "dirtyJtagConfig.h"

#if ( CDC_UART_INTF_COUNT > 0 )

#include <pico/stdlib.h>
#include <hardware/dma.h>
#include <hardware/irq.h>
#include "led.h"
#include "tusb.h"
#include "cdc_uart.h"

static struct uart_device
{
	uart_inst_t *inst;
	volatile uint8_t tx_buf[TX_BUFFER_SIZE];
	volatile uint8_t rx_buf[RX_BUFFER_SIZE];
	uint rx_dma_channel;
	uint tx_dma_channel;
	volatile uint8_t *tx_write_address;
	uint8_t *rx_read_address;
	uint n_checks;
	uint is_connected;
} uart_devices[2];

static void dma_handler();

static void set_tx_dma(volatile uint8_t *l_tx_write_address, struct uart_device *uart);

static uint n_bits(uint n)
{
	int i;
	for (i = 0; i < 31; i++)
	{
		n >>= 1;
		if (n == 0)
		break;
	}
	return i+1;
}

uint setup_usart_tx_dma(uart_inst_t *uart, volatile uint8_t *tx_address, uint buffer_size)
{
	uint dma_chan = dma_claim_unused_channel(true);
	// Tell the DMA to raise IRQ line 1 when the channel finishes a block
	dma_channel_set_irq1_enabled(dma_chan, true);
	// enable DMA TX
	hw_write_masked(&uart_get_hw(uart)->dmacr, 1 << UART_UARTDMACR_TXDMAE_LSB, UART_UARTDMACR_TXDMAE_BITS);

	dma_channel_config c = dma_channel_get_default_config(dma_chan);
	channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
	channel_config_set_read_increment(&c, true);
	channel_config_set_write_increment(&c, false);
	channel_config_set_dreq(&c, uart_get_dreq(uart, true));
	dma_channel_configure(
		dma_chan,
		&c,
		&uart_get_hw(uart)->dr, // Write Ad
		tx_address,             // Read Address
		0,                      // transfer count
		false                   // start
	);
	return dma_chan;
}

uint setup_usart_rx_dma(uart_inst_t *uart, volatile void *rx_address, irq_handler_t handler, uint buffer_size)
{
	uint dma_chan = dma_claim_unused_channel(true);
	// Tell the DMA to raise IRQ line 1 when the channel finishes a block
	dma_channel_set_irq1_enabled(dma_chan, true);

	// Configure the processor to run dma_handler() when DMA IRQ 1 is asserted
	irq_add_shared_handler(DMA_IRQ_1, handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
	irq_set_enabled(DMA_IRQ_1, true);
	// enable DMA RX
	hw_write_masked(&uart_get_hw(uart)->dmacr, 1 << UART_UARTDMACR_RXDMAE_LSB, UART_UARTDMACR_RXDMAE_BITS);

	dma_channel_config c = dma_channel_get_default_config(dma_chan);
	channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
	channel_config_set_read_increment(&c, false);
	channel_config_set_write_increment(&c, true);
	channel_config_set_dreq(&c, uart_get_dreq(uart, false));
	hw_clear_bits(&uart_get_hw(uart)->rsr, UART_UARTRSR_BITS); // clear
	dma_channel_configure(
		dma_chan,
		&c,
		rx_address,				// Write Address
		&uart_get_hw(uart)->dr, // Read Address
		buffer_size,			// transfer count
		true					// start
	);
	return dma_chan;
}

void cdc_uart_init( int index, uart_inst_t *const uart_, int uart_rx_pin, int uart_tx_pin )  {
	uint uart_index;
	uart_index = uart_get_index(uart_);
    struct uart_device *uart;
	uart = &uart_devices[index];

	gpio_set_function(uart_tx_pin, GPIO_FUNC_UART);
	gpio_set_function(uart_rx_pin, GPIO_FUNC_UART);
	gpio_set_pulls(uart_tx_pin, 1, 0);
	gpio_set_pulls(uart_rx_pin, 1, 0);

	uart->inst = uart_;
	uart_init(uart->inst, USBUSART_BAUDRATE);
	uart_set_hw_flow(uart->inst, false, false);
	uart_set_format(uart->inst, 8, 1, UART_PARITY_NONE);
	uart_set_fifo_enabled(uart->inst, true);
	uart->tx_dma_channel = setup_usart_tx_dma(uart->inst, &uart->tx_buf[0], TX_BUFFER_SIZE);
	uart->rx_dma_channel = setup_usart_rx_dma(uart->inst, &uart->rx_buf[0], dma_handler, RX_BUFFER_SIZE);
	uart->tx_write_address = &uart->tx_buf[0];
	uart->rx_read_address = (uint8_t *)&uart->rx_buf[0];
	uart->n_checks = 0; 
}

void set_tx_dma(volatile uint8_t *l_tx_write_address, struct uart_device *uart)
{
	uint8_t *ra = (uint8_t *)(dma_channel_hw_addr(uart->tx_dma_channel)->read_addr);
	if (ra >= (&uart->tx_buf[0] + TX_BUFFER_SIZE))
	{
		assert(ra == (&uart->tx_buf[0] + TX_BUFFER_SIZE));
		dma_channel_set_read_addr(uart->tx_dma_channel, &uart->tx_buf[0], false);
		ra = (uint8_t*)&uart->tx_buf[0];
	}
	if (ra != l_tx_write_address)
	{
		size_t length = (l_tx_write_address >= ra) ? (l_tx_write_address - ra) : (TX_BUFFER_SIZE - (ra - &uart->tx_buf[0]));
		dma_channel_set_trans_count(uart->tx_dma_channel, length, true);
	}
}

// shared between rx_dma_channel and tx_dma_channel
static void dma_handler()
{
	volatile uint32_t ints = dma_hw->ints1;
	struct uart_device *uart;

	for (size_t i = 0; i < CDC_UART_INTF_COUNT; i++)
	{
		uart = &uart_devices[i];
		
		if (dma_channel_get_irq1_status(uart->rx_dma_channel))
		{
			dma_channel_set_write_addr(uart->rx_dma_channel, &uart->rx_buf[0], true);
		}
		if (dma_channel_get_irq1_status(uart->tx_dma_channel))
		{
			// cdc_uart_task can modify uart->tx_write_address. cache it locally
			volatile uint8_t *l_tx_write_address = uart->tx_write_address;
            set_tx_dma(l_tx_write_address, uart);
        }
	}
	
	dma_hw->ints1 = ints;
}



bool cdc_stopped = false;
void cdc_uart_task(void)
{
	if (cdc_stopped)
	  	return;

	struct uart_device *uart;

	for (size_t i = 0; i < CDC_UART_INTF_COUNT; i++)
	{
		uart = &uart_devices[i];
		if (tud_cdc_n_connected(i))
		{
			uart->is_connected = 1;
			int written = 0;
			volatile uint8_t *wa = (uint8_t*)(dma_channel_hw_addr(uart->rx_dma_channel)->write_addr);
			if (wa == &uart->rx_buf[RX_BUFFER_SIZE])
			{
				wa = &uart->rx_buf[0];
			}
			uint32_t rx_used_space = (wa >= uart->rx_read_address) ? (wa - uart->rx_read_address) : (wa + RX_BUFFER_SIZE - uart->rx_read_address);
			uart->n_checks++;
			if ((rx_used_space >= FULL_SWO_PACKET) || ((rx_used_space != 0) && (uart->n_checks > 4)))
			{
				led_tx(1);
				uart->n_checks = 0;
				uint32_t capacity = tud_cdc_n_write_available(i);
				uint32_t size_out = MIN(rx_used_space, capacity);
				if (capacity >= FULL_SWO_PACKET)
				{
					uint32_t written = tud_cdc_n_write(i, uart->rx_read_address, size_out);
					if (rx_used_space < FULL_SWO_PACKET)
						tud_cdc_n_write_flush(i);
					tud_task();
					uart->rx_read_address += written;
					if (uart->rx_read_address >= &uart->rx_buf[RX_BUFFER_SIZE])
						uart->rx_read_address -= RX_BUFFER_SIZE;
				}
				led_tx(0);
			}
			uint usb_available = tud_cdc_n_available(i);
			uint8_t *ra = (uint8_t *)(dma_channel_hw_addr(uart->tx_dma_channel)->read_addr);
			uint32_t tx_free_space = (uart->tx_write_address >= ra) ? (&uart->tx_buf[TX_BUFFER_SIZE] - uart->tx_write_address) : (ra - uart->tx_write_address);
			size_t watermark = MIN(usb_available, tx_free_space);
			if (watermark > 0)
			{
				led_rx(1);
				size_t tx_len;
				tx_len = tud_cdc_n_read(i, (void*)uart->tx_write_address, watermark);
				//be careful about modifying tx_write_address as it is used in the IRQ handler
				volatile uint8_t *l_tx_write_address = uart->tx_write_address + tx_len;
				if (l_tx_write_address >= &uart->tx_buf[TX_BUFFER_SIZE])
					uart->tx_write_address = l_tx_write_address - TX_BUFFER_SIZE;
				else
					uart->tx_write_address = l_tx_write_address;
				// restart dma if not active
				if (!dma_channel_is_busy(uart->tx_dma_channel))
				{
					set_tx_dma(l_tx_write_address, uart);
				}
				led_rx(0);
			}
		}
		else if (uart->is_connected)
		{
			tud_cdc_n_write_clear(i);
			uart->is_connected = 0;
		}
	}
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* line_coding)
{
	struct uart_device *uart;

	for (size_t i = 0; i < CDC_UART_INTF_COUNT; i++)
	{
		uart = &uart_devices[i];
		if (i == itf)
		{
			uart_deinit(uart->inst);
			tud_cdc_n_write_clear(itf);
			tud_cdc_n_read_flush(itf);
			uart_init(uart->inst, line_coding->bit_rate);
		}
		
	}
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
	//if (itf != UART_PORT_ITF)
		return;
	/* CDC drivers use linestate as a bodge to activate/deactivate the interface.
	* Resume our UART polling on activate, stop on deactivate */
    // DTR RTS not so
	// if (!dtr && !rts)
	// 	cdc_stopped = true;
	// else
	// 	cdc_stopped = false;
}

#endif // CDC_UART_INTF_COUNT
