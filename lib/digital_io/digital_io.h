#ifndef DIGITAL_IO_H
#define DIGITAL_IO_H

#include "pico/stdlib.h"

#if defined(PICO_CYW43_ARCH_POLL) || defined(PICO_CYW43_ARCH_THREADSAFE_BACKGROUND) || defined(PICO_CYW43_ARCH_FREERTOS)
#define DIGITAL_IO_HAS_CYW43 1
#else
#define DIGITAL_IO_HAS_CYW43 0
#endif

#if DIGITAL_IO_HAS_CYW43
#include "pico/cyw43_arch.h"
#endif

// Pin modes
#define INPUT           0
#define OUTPUT          1
#define INPUT_PULLUP    2
#define INPUT_PULLDOWN  3

// Logic levels
#define LOW  0
#define HIGH 1

// Special Pico W onboard LED pin (invalid on non-W targets)
#if DIGITAL_IO_HAS_CYW43
#define PICO_W_LED_PIN CYW43_WL_GPIO_LED_PIN
#else
#define PICO_W_LED_PIN ((uint)(-1))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Initialize GPIO + CYW43 LED
void digitalIO_init(void);

void pinMode(uint pin, uint mode);
void digitalWrite(uint pin, uint value);
int  digitalRead(uint pin);

#ifdef __cplusplus
}
#endif

#endif
