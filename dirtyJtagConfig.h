#ifndef DirtyJtagConfig_h
#define DirtyJtagConfig_h

// Set to 0 to disable USB-CDC-UART bridge
#define USB_CDC_UART_BRIDGE  1


#define BOARD_PICO           0
#define BOARD_ADAFRUIT_ITSY  1
#define BOARD_SPOKE_RP2040   2
#define BOARD_QMTECH_RP2040_DAUGHTERBOARD 3
#define BOARD_WERKZEUG       4

// Select the board type from the above
#define BOARD_TYPE BOARD_PICO
//#define BOARD_TYPE BOARD_ADAFRUIT_ITSY
//#define BOARD_TYPE BOARD_SPOKE_RP2040
//#define BOARD_TYPE BOARD_WERKZEUG
//#define BOARD_TYPE BOARD_QMTECH_RP2040_DAUGHTERBOARD

// General mapping
// TDI  SPIO RX
// TDO  SPIO TX
// TCK  SPIO SCK
// TMS  SPIO CS
// RST  GPIO
// TRST GPIO

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

#if ( USB_CDC_UART_BRIDGE )
#define PIN_UART       uart0
#define PIN_UART_TX    12
#define PIN_UART_RX    13
#endif // USB_CDC_UART_BRIDGE

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

#if ( USB_CDC_UART_BRIDGE )
#define PIN_UART       uart0
#define PIN_UART_TX    0
#define PIN_UART_RX    1
#endif // USB_CDC_UART_BRIDGE

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

#if ( USB_CDC_UART_BRIDGE )
#define PIN_UART       uart0
#define PIN_UART_TX    28
#define PIN_UART_RX    29
#endif // USB_CDC_UART_BRIDGE


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

#if ( USB_CDC_UART_BRIDGE )
#define PIN_UART       uart0
#define PIN_UART_TX    28
#define PIN_UART_RX    29
#endif // USB_CDC_UART_BRIDGE
#elif ( BOARD_TYPE == BOARD_QMTECH_RP2040_DAUGHTERBOARD )

// in rp2040 daughterboard UART pins are connected to FPGA pins
// depending on the FPGA pin configuration there is a possibility
// of damage so these pins are not going to be setup
#define USB_CDC_UART_BRIDGE  0

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

#endif // BOARD_TYPE

#endif // DirtyJtagConfig_h
