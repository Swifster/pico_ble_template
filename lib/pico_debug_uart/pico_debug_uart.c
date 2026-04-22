#include "pico_debug_uart.h"
#include <stdarg.h>

static uart_inst_t *debug_uart = NULL;

void debug_uart_init(void)
{
#if DEBUG_USE_UART0
    debug_uart = uart0;
#elif DEBUG_USE_UART1
    debug_uart = uart1;
#else
#error "Select either UART0 or UART1"
#endif

    // Select pins depending on UART
#if DEBUG_USE_UART0
    uart_init(debug_uart, DEBUG_BAUDRATE);
    gpio_set_function(DEBUG_UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(DEBUG_UART0_RX_PIN, GPIO_FUNC_UART);
#elif DEBUG_USE_UART1
    uart_init(debug_uart, DEBUG_BAUDRATE);
    gpio_set_function(DEBUG_UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(DEBUG_UART1_RX_PIN, GPIO_FUNC_UART);
#endif
}


void debug_uart_print(const char *s)
{
    if(!debug_uart) return;

    while (*s) {
        uart_putc_raw(debug_uart, *s++);
    }
}


void debug_uart_printf(const char *fmt, ...)
{
    if(!debug_uart) return;

    char buffer[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    debug_uart_print(buffer);
}
