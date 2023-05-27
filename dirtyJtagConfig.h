#ifndef DirtyJtagConfig_h
#define DirtyJtagConfig_h

// Set to 0 to disable USB-CDC-UART bridge
#define USB_CDC_UART_BRIDGE  1

#define BOARD_PICO           0
#define BOARD_ADAFRUIT_ITSY  1
#define BOARD_SPOKE_RP2040   2

// Select the board type from the above
#define BOARD_TYPE BOARD_PICO
//#define BOARD_TYPE BOARD_ADAFRUIT_ITSY
//#define BOARD_TYPE BOARD_SPOKE_RP2040

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

#endif // BOARD_TYPE

#endif // DirtyJtagConfig_h
