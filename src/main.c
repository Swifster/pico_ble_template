#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "pico_ble_stack.h"

typedef enum {
    LED_MODE_BLINK,
    LED_MODE_ON,
    LED_MODE_OFF,
} led_mode_t;

static led_mode_t led_mode = LED_MODE_OFF;

static void apply_led_mode(void)
{
    if (led_mode == LED_MODE_ON) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    } else if (led_mode == LED_MODE_OFF) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    }
}

static const char *led_mode_name(void)
{
    switch (led_mode) {
    case LED_MODE_ON:
        return "on";
    case LED_MODE_OFF:
        return "off";
    case LED_MODE_BLINK:
    default:
        return "blink";
    }
}

static bool command_char_is_space(char value)
{
    return value == ' ' || value == '\t' || value == '\r' || value == '\n';
}

static uint16_t normalize_command(const uint8_t *packet, uint16_t size, char *command, uint16_t command_size)
{
    uint16_t start = 0;
    while (start < size && command_char_is_space((char)packet[start])) {
        start++;
    }

    uint16_t end = size;
    while (end > start && command_char_is_space((char)packet[end - 1])) {
        end--;
    }

    uint16_t out = 0;
    bool previous_was_space = false;
    for (uint16_t i = start; i < end && out + 1 < command_size; i++) {
        char value = (char)packet[i];
        if (command_char_is_space(value)) {
            if (!previous_was_space) {
                command[out++] = ' ';
                previous_was_space = true;
            }
            continue;
        }

        if (value >= 'A' && value <= 'Z') {
            value = (char)(value - 'A' + 'a');
        }
        command[out++] = value;
        previous_was_space = false;
    }

    command[out] = '\0';
    return out;
}

static bool command_equals(const char *command, const char *expected)
{
    return strcmp(command, expected) == 0;
}

static void handle_uart_command(const uint8_t *packet, uint16_t size)
{
    char command[48];
    if (normalize_command(packet, size, command, sizeof(command)) == 0) {
        return;
    }

    printf("Bluefruit command: %s\n", command);

    if (command_equals(command, "light led") ||
        command_equals(command, "led on") ||
        command_equals(command, "on")) {
        led_mode = LED_MODE_ON;
        apply_led_mode();
        pico_ble_stack_uart_send("ok,led=on\r\n");
        return;
    }

    if (command_equals(command, "led off") ||
        command_equals(command, "off")) {
        led_mode = LED_MODE_OFF;
        apply_led_mode();
        pico_ble_stack_uart_send("ok,led=off\r\n");
        return;
    }

    if (command_equals(command, "led toggle") ||
        command_equals(command, "toggle led") ||
        command_equals(command, "toggle")) {
        led_mode = led_mode == LED_MODE_ON ? LED_MODE_OFF : LED_MODE_ON;
        apply_led_mode();
        pico_ble_stack_uart_sendf("ok,led=%s\r\n", led_mode_name());
        return;
    }

    if (command_equals(command, "led blink") ||
        command_equals(command, "blink led") ||
        command_equals(command, "blink")) {
        led_mode = LED_MODE_BLINK;
        pico_ble_stack_uart_send("ok,led=blink\r\n");
        return;
    }

    if (command_equals(command, "temp") ||
        command_equals(command, "status")) {
        pico_ble_stack_uart_sendf("status,temp_c=%.2f,led=%s\r\n",
                                  pico_ble_stack_get_temperature_c(),
                                  led_mode_name());
        return;
    }

    if (command_equals(command, "help")) {
        pico_ble_stack_uart_send("commands: light led, led off, led toggle, led blink, temp\r\n");
        return;
    }

    pico_ble_stack_uart_sendf("error,unknown command: %s\r\n", command);
}

static void handle_ble_tick(void)
{
    if (led_mode == LED_MODE_BLINK) {
        static bool led_on = true;
        led_on = !led_on;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
    } else {
        apply_led_mode();
    }
}

static void handle_ble_connection_change(bool connected)
{
    printf("Bluefruit UART %s\n", connected ? "connected" : "disconnected");
}

int main(void)
{
    const pico_ble_stack_handlers_t ble_handlers = {
        .on_uart_receive = handle_uart_command,
        .on_tick = handle_ble_tick,
        .on_connection_change = handle_ble_connection_change,
    };

    stdio_init_all();
    pico_ble_stack_set_handlers(&ble_handlers);
    return pico_ble_stack_run();
}
