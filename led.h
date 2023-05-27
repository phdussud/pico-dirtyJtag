#ifndef LED_H
#define LED_H

#include <stdbool.h>

void led_init( bool inverted, int ledTxPin, int ledRxPin, int errorLedPin );

void led_tx( bool state );
void led_rx( bool state );
void led_error( bool state );

#endif
