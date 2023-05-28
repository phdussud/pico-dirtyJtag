#include "led.h"
#include "hardware/gpio.h"

bool LedInverted = false;
int LedRxPin = 0;
int LedTxPin = 0;
int LedErrorPin = 0;

void led_init( bool inverted, int ledRxPin, int ledTXPin, int ledErrorPin_ ) {
    LedTxPin = ledTXPin;
    LedRxPin = ledRxPin;
    LedErrorPin = ledErrorPin_;
    LedInverted = inverted;

    if ( LedTxPin != -1 ) {
        gpio_init( LedTxPin);
        gpio_set_dir(LedTxPin, GPIO_OUT);
        gpio_put(LedTxPin, (LedInverted) ? 1 : 0);
    }

    if ( LedTxPin != LedRxPin ) {
        if ( LedRxPin != -1 ) {
            gpio_init( LedRxPin);
            gpio_set_dir(LedRxPin, GPIO_OUT);
            gpio_put(LedRxPin, (LedInverted) ? 1 : 0);
        }
    }

    if ( ( LedErrorPin != LedRxPin ) && ( LedErrorPin != LedTxPin ) ) {
        if ( LedErrorPin != -1 ) {
            gpio_init( LedErrorPin);
            gpio_set_dir(LedErrorPin, GPIO_OUT);
            gpio_put(LedErrorPin, (LedInverted) ? 1 : 0 );
        }
    }
}

void led_tx( bool state ) {
    if ( LedTxPin != -1 )  
        gpio_put(LedRxPin, LedInverted ^ state);
}

void led_rx( bool state ) {
    if ( LedRxPin != -1 )  
        gpio_put(LedRxPin, LedInverted ^ state);
}

void led_error( bool state ) {
    if ( LedErrorPin != -1 )  
        gpio_put(LedErrorPin, LedInverted ^ state);
}
