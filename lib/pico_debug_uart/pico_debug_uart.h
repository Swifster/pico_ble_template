#ifndef PICO_DEBUG_UART_H
#define PICO_DEBUG_UART_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

// ==============================
// Configuration
// ==============================

// UART selection
// set ONE to 1 and the other to 0
#define DEBUG_USE_UART0   1
#define DEBUG_USE_UART1   0

// UART baudrate
#define DEBUG_BAUDRATE    115200


// UART0 pins (edit if needed)
#define DEBUG_UART0_TX_PIN   0
#define DEBUG_UART0_RX_PIN   1

// UART1 pins (edit if needed)
#define DEBUG_UART1_TX_PIN   4
#define DEBUG_UART1_RX_PIN   5


// Public functions
void debug_uart_init(void);
void debug_uart_print(const char *s);
void debug_uart_printf(const char *fmt, ...);

#endif
