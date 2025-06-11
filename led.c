/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 David Williams (davidthings)
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
