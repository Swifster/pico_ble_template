#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "pico_ble_stack.h"
#include "picotemp.h"

#define UART_COMMAND_MAX_LEN 48
#define UART_HELP_TEXT "commands: led on, led off, led toggle, led blink, temp, temp float, temp status, temp start, temp stop, rssi, rssi start, rssi stop\r\n"

typedef enum
{
    LED_MODE_BLINK,
    LED_MODE_ON,
    LED_MODE_OFF,
} led_mode_t;

typedef enum
{
    TELEMETRY_FORMAT_STATUS,
    TELEMETRY_FORMAT_FLOAT,
} telemetry_format_t;

static led_mode_t led_mode = LED_MODE_OFF;
static bool telemetry_enabled = true;
static bool rssi_telemetry_enabled;
static telemetry_format_t telemetry_format = TELEMETRY_FORMAT_STATUS;
static unsigned telemetry_counter;

static float read_temperature_c(void)
{
    return read_onboard_temperature('C');
}

static void apply_led_mode(void)
{
    if (led_mode == LED_MODE_ON)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    }
    else if (led_mode == LED_MODE_OFF)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    }
}

static void set_led_mode(led_mode_t mode)
{
    led_mode = mode;
    apply_led_mode();
}

static const char *led_mode_name(void)
{
    switch (led_mode)
    {
    case LED_MODE_ON:
        return "on";
    case LED_MODE_OFF:
        return "off";
    case LED_MODE_BLINK:
    default:
        return "blink";
    }
}

static uint16_t copy_command(const uint8_t *packet, uint16_t size, char *command, uint16_t command_size)
{
    if (command_size == 0)
    {
        return 0;
    }

    uint16_t length = size;
    if (length >= command_size)
    {
        length = command_size - 1;
    }

    memcpy(command, packet, length);
    command[length] = '\0';

    while (length > 0 && (command[length - 1] == '\r' || command[length - 1] == '\n' || command[length - 1] == ' '))
    {
        command[--length] = '\0';
    }

    return length;
}

static bool command_equals(const char *command, const char *expected)
{
    return strcmp(command, expected) == 0;
}

static void send_temperature_float(void)
{
    pico_ble_stack_uart_sendf("%.2f\r\n", read_temperature_c());
}

static void handle_uart_command(const uint8_t *packet, uint16_t size)
{
    char command[UART_COMMAND_MAX_LEN];
    if (copy_command(packet, size, command, sizeof(command)) == 0)
    {
        return;
    }

    printf("Bluefruit command: %s\n", command);

    if (command_equals(command, "led on"))
    {
        set_led_mode(LED_MODE_ON);
        pico_ble_stack_uart_send("ok,led=on\r\n");
        return;
    }

    if (command_equals(command, "led off"))
    {
        set_led_mode(LED_MODE_OFF);
        pico_ble_stack_uart_send("ok,led=off\r\n");
        return;
    }

    if (command_equals(command, "led toggle"))
    {
        set_led_mode(led_mode == LED_MODE_ON ? LED_MODE_OFF : LED_MODE_ON);
        pico_ble_stack_uart_sendf("ok,led=%s\r\n", led_mode_name());
        return;
    }

    if (command_equals(command, "led blink"))
    {
        led_mode = LED_MODE_BLINK;
        pico_ble_stack_uart_send("ok,led=blink\r\n");
        return;
    }

    if (command_equals(command, "temp"))
    {
        pico_ble_stack_uart_sendf("status,temp_c=%.2f,led=%s\r\n",
                                  read_temperature_c(),
                                  led_mode_name());
        return;
    }

    if (command_equals(command, "temp float"))
    {
        telemetry_format = TELEMETRY_FORMAT_FLOAT;
        telemetry_enabled = true;
        send_temperature_float();
        return;
    }

    if (command_equals(command, "temp status"))
    {
        telemetry_format = TELEMETRY_FORMAT_STATUS;
        pico_ble_stack_uart_send("ok,temp=status\r\n");
        return;
    }

    if (command_equals(command, "temp start"))
    {
        telemetry_enabled = true;
        pico_ble_stack_uart_send("ok,temp=started\r\n");
        return;
    }

    if (command_equals(command, "temp stop"))
    {
        telemetry_enabled = false;
        pico_ble_stack_uart_send("ok,temp=stopped\r\n");
        return;
    }

    if (command_equals(command, "rssi"))
    {
        if (!pico_ble_stack_uart_request_rssi())
        {
            pico_ble_stack_uart_send("error,rssi=unavailable\r\n");
        }
        return;
    }

    if (command_equals(command, "rssi start"))
    {
        rssi_telemetry_enabled = true;
        pico_ble_stack_uart_send("ok,rssi=started\r\n");
        return;
    }

    if (command_equals(command, "rssi stop"))
    {
        rssi_telemetry_enabled = false;
        pico_ble_stack_uart_send("ok,rssi=stopped\r\n");
        return;
    }

    if (command_equals(command, "help"))
    {
        pico_ble_stack_uart_send(UART_HELP_TEXT);
        return;
    }

    pico_ble_stack_uart_sendf("error,unknown command: %s\r\n", command);
}

static void handle_ble_tick(void)
{
    if (telemetry_enabled && pico_ble_stack_uart_is_connected())
    {
        telemetry_counter++;
        if (telemetry_format == TELEMETRY_FORMAT_FLOAT)
        {
            send_temperature_float();
        }
        else
        {
            pico_ble_stack_uart_sendf("demo,%u,temp_c=%.2f\r\n",
                                      telemetry_counter,
                                      read_temperature_c());
        }
    }

    if (rssi_telemetry_enabled && pico_ble_stack_uart_is_connected())
    {
        pico_ble_stack_uart_request_rssi();
    }

    if (led_mode == LED_MODE_BLINK)
    {
        static bool led_on = true;
        led_on = !led_on;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
    }
    else
    {
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
    picotemp_init();
    pico_ble_stack_set_device_name("PICO-BLE-DEMO");
    pico_ble_stack_set_handlers(&ble_handlers);
    return pico_ble_stack_run();
}
