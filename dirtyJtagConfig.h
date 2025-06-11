/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2025 Patrick Dussud
 * Copyright (c) 2023 David Williams (davidthings)
 * Copyright (c) 2023 Chandler Klüser
 * Copyright (c) 2023 inc
 * Copyright (c) 2023 Miguel Angel Ajo Pelayo (mangelajo)
 * Copyright (c) 2023 luyi1888
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

#ifndef DirtyJtagConfig_h
#define DirtyJtagConfig_h





#define BOARD_PICO           0
#define BOARD_ADAFRUIT_ITSY  1
#define BOARD_SPOKE_RP2040   2
#define BOARD_QMTECH_RP2040_DAUGHTERBOARD 3
#define BOARD_WERKZEUG       4
#define BOARD_RP2040_ZERO    5

// Select the board type from the above
#define BOARD_TYPE BOARD_PICO
//#define BOARD_TYPE BOARD_ADAFRUIT_ITSY
//#define BOARD_TYPE BOARD_SPOKE_RP2040
//#define BOARD_TYPE BOARD_WERKZEUG
//#define BOARD_TYPE BOARD_QMTECH_RP2040_DAUGHTERBOARD
//#define BOARD_TYPE BOARD_RP2040_ZERO

// General mapping
// TDI  SPIO RX
// TDO  SPIO TX
// TCK  SPIO SCK
// TMS  SPIO CS
// RST  GPIO
// TRST GPIO

// Set CDC_UART_INTF_COUNT to 0 to disable USB-CDC-UART bridge

#if ( BOARD_TYPE == BOARD_PICO )

#define PIN_TDI 16 
#define PIN_TDO 17
#define PIN_TCK 18
#define PIN_TMS 19
#define PIN_RST 20
#define PIN_TRST 21

#define LED_INVERTED   0
#define PIN_LED_TX     25
#define PIN_LED_ERROR  25
#define PIN_LED_RX     25

#define CDC_UART_INTF_COUNT 2
#define PIN_UART0 uart0
#define PIN_UART0_TX    12
#define PIN_UART0_RX    13
#define PIN_UART1 uart1
#define PIN_UART1_TX    4
#define PIN_UART1_RX    5

#elif ( BOARD_TYPE == BOARD_ADAFRUIT_ITSY )

#define PIN_TDI 28 
#define PIN_TDO 27
#define PIN_TCK 26
#define PIN_TMS 29
#define PIN_RST 24
#define PIN_TRST 25

// no regular LEDs on the Itsy - it's a neopixel
#define LED_INVERTED   0
#define PIN_LED_TX     -1
#define PIN_LED_ERROR  -1
#define PIN_LED_RX     -1

#define CDC_UART_INTF_COUNT 1
#define PIN_UART0       uart0
#define PIN_UART0_TX    0
#define PIN_UART0_RX    1

#elif ( BOARD_TYPE == BOARD_SPOKE_RP2040 )

#define PIN_TDI 23 
#define PIN_TDO 20
#define PIN_TCK 22
#define PIN_TMS 21
#define PIN_RST 26
#define PIN_TRST 27

#define LED_INVERTED   1
#define PIN_LED_TX     16
#define PIN_LED_ERROR  17
#define PIN_LED_RX     18

#define CDC_UART_INTF_COUNT 1
#define PIN_UART0       uart0
#define PIN_UART0_TX    28
#define PIN_UART0_RX    29


#elif ( BOARD_TYPE == BOARD_WERKZEUG )

#define PIN_TDI 1
#define PIN_TDO 2
#define PIN_TCK 0
#define PIN_TMS 3
#define PIN_RST 4
#define PIN_TRST 5

#define LED_INVERTED   1
#define PIN_LED_TX     20
#define PIN_LED_ERROR  21
#define PIN_LED_RX     20

#define CDC_UART_INTF_COUNT 1
#define PIN_UART0       uart0
#define PIN_UART0_TX    28
#define PIN_UART0_RX    29
#elif ( BOARD_TYPE == BOARD_QMTECH_RP2040_DAUGHTERBOARD )

// in rp2040 daughterboard UART pins are connected to FPGA pins
// depending on the FPGA pin configuration there is a possibility
// of damage so these pins are not going to be setup
#define CDC_UART_INTF_COUNT 0

#define PIN_TDI  16 
#define PIN_TDO  17
#define PIN_TCK  18
#define PIN_TMS  19
// in rp2040 daughterboard these pins are connected to FPGA pins
// depending on the FPGA pin configuration there is a possibility
// of damage so these pins are not going to be setup
// #define PIN_RST  X
// #define PIN_TRST X

#define LED_INVERTED   0
#define PIN_LED_TX     25
#define PIN_LED_ERROR  25
#define PIN_LED_RX     25

#elif ( BOARD_TYPE == BOARD_RP2040_ZERO )

#define PIN_TDI 0
#define PIN_TDO 3
#define PIN_TCK 2
#define PIN_TMS 1
#define PIN_RST 4
#define PIN_TRST 5

// the LED is actually a ws2812 neopixel, using a
// spare pin where we could attach a led, updating the
// neopixel would probably be slow
#define LED_INVERTED   1
#define PIN_LED_TX     29
#define PIN_LED_ERROR  29
#define PIN_LED_RX     29

#define CDC_UART_INTF_COUNT 2
#define PIN_UART0       uart0
#define PIN_UART0_TX    12
#define PIN_UART0_RX    13
#define PIN_UART1       uart1
#define PIN_UART1_TX    8
#define PIN_UART1_RX    9


#endif // BOARD_TYPE

#endif // DirtyJtagConfig_h
