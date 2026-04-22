#ifndef PICO_BLE_STACK_H
#define PICO_BLE_STACK_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*pico_ble_uart_receive_handler_t)(const uint8_t *data, uint16_t size);
typedef void (*pico_ble_tick_handler_t)(void);
typedef void (*pico_ble_connection_handler_t)(bool connected);

typedef struct {
    pico_ble_uart_receive_handler_t on_uart_receive;
    pico_ble_tick_handler_t on_tick;
    pico_ble_connection_handler_t on_connection_change;
} pico_ble_stack_handlers_t;

void pico_ble_stack_set_handlers(const pico_ble_stack_handlers_t *handlers);
void pico_ble_stack_set_device_name(const char *name);
const char *pico_ble_stack_get_device_name(void);
bool pico_ble_stack_uart_is_connected(void);
void pico_ble_stack_uart_send(const char *message);
void pico_ble_stack_uart_sendf(const char *format, ...);
float pico_ble_stack_get_temperature_c(void);
int pico_ble_stack_run(void);

#endif
