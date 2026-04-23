# `src/main.c` Function Guide

This guide explains what the firmware in `src/main.c` does, why each function
exists, and how the logic flows at runtime.

## Big Picture

The application turns a Pico W / Pico 2 W into a small Bluetooth Low Energy
UART demo device. A phone connects with Adafruit Bluefruit Connect, sends text
commands, and receives replies or temperature telemetry.

At a high level, the firmware does four jobs:

1. Starts standard I/O, the onboard temperature reader, and the BLE stack.
2. Advertises a BLE UART device named `PICO-BLE-DEMO`.
3. Parses incoming UART text commands such as `led on` and `temp stop`.
4. Sends temperature data and updates the onboard wireless LED once per tick.

The BLE stack owns the main event loop. `main()` registers callbacks, then
`pico_ble_stack_run()` repeatedly calls those callbacks when something happens.
The tick callback is driven by the BLE stack heartbeat, currently once per
second.

## Important State

`UART_COMMAND_MAX_LEN` limits accepted command text to 47 characters plus a
null terminator. This keeps command parsing bounded and avoids writing past the
local command buffer.

`UART_HELP_TEXT` is the text returned by the `help` command.

`led_mode_t` describes the LED behavior:

- `LED_MODE_BLINK`: the periodic tick toggles the LED.
- `LED_MODE_ON`: the LED is held on.
- `LED_MODE_OFF`: the LED is held off.

`telemetry_format_t` describes how automatic temperature telemetry is sent:

- `TELEMETRY_FORMAT_STATUS`: sends labeled messages like
  `demo,1,temp_c=27.98`.
- `TELEMETRY_FORMAT_FLOAT`: sends only the number, like `27.98`.

The global variables hold the current operating mode:

- `led_mode`: starts as `LED_MODE_OFF`.
- `telemetry_enabled`: starts as `true`, so telemetry is sent automatically
  after a BLE UART connection exists.
- `telemetry_format`: starts as `TELEMETRY_FORMAT_STATUS`.
- `telemetry_counter`: increments every time a status telemetry packet is sent.

## Function Reference

### `read_temperature_c()`

```c
static float read_temperature_c(void)
```

Reads the onboard temperature sensor and returns Celsius.

This is a tiny wrapper around `read_onboard_temperature('C')` from the
`picotemp` helper library. Keeping this wrapper in `main.c` makes the rest of
the file read like application logic instead of library details.

### `apply_led_mode()`

```c
static void apply_led_mode(void)
```

Writes the current non-blinking LED mode to the Pico W wireless LED pin.

If `led_mode` is `LED_MODE_ON`, it writes `1`. If `led_mode` is
`LED_MODE_OFF`, it writes `0`. It intentionally does not handle
`LED_MODE_BLINK`, because blinking needs to toggle between on and off over
time. That time-based behavior lives in `handle_ble_tick()`.

### `set_led_mode()`

```c
static void set_led_mode(led_mode_t mode)
```

Stores a new LED mode and immediately applies it.

This helper is used for steady LED states so commands like `led on` and
`led off` update both the saved state and the physical LED in one place.

### `led_mode_name()`

```c
static const char *led_mode_name(void)
```

Converts the current LED mode into reply text:

- `LED_MODE_ON` -> `on`
- `LED_MODE_OFF` -> `off`
- `LED_MODE_BLINK` -> `blink`

This keeps status replies consistent. For example, the `temp` command uses this
function when it sends `status,temp_c=27.98,led=blink`.

### `copy_command()`

```c
static uint16_t copy_command(
    const uint8_t *packet,
    uint16_t size,
    char *command,
    uint16_t command_size
)
```

Copies raw BLE UART bytes into a normal C string.

The BLE receive callback gives the firmware a byte pointer and a byte count,
not a null-terminated string. This function makes the data safe for string
functions by:

1. Returning `0` if the destination size is invalid.
2. Copying at most `command_size - 1` bytes.
3. Adding `'\0'` at the end.
4. Removing trailing carriage return, newline, and space characters.
5. Returning the final command length.

The trimming step lets commands work whether the phone sends `led on`,
`led on\n`, or `led on\r\n`.

### `command_equals()`

```c
static bool command_equals(const char *command, const char *expected)
```

Compares one received command with one expected command.

It wraps `strcmp(command, expected) == 0`. The helper is small, but it makes
the command parser read clearly:

```c
if (command_equals(command, "led on"))
```

### `send_temperature_float()`

```c
static void send_temperature_float(void)
```

Sends only the temperature number over BLE UART, formatted with two decimal
places.

Example:

```text
27.98
```

This is useful for simple mobile apps, plotters, or scripts that want one
numeric value per line instead of a labeled status message.

### `handle_uart_command()`

```c
static void handle_uart_command(const uint8_t *packet, uint16_t size)
```

Handles text received from the BLE UART RX characteristic.

This is the main command parser. The logic is:

1. Copy and trim the incoming packet with `copy_command()`.
2. Ignore empty commands.
3. Print the command to local stdio for debugging.
4. Compare the command against known text commands.
5. Change firmware state or send data.
6. Send a BLE UART response.
7. Return immediately after a command matches.
8. Send an error if nothing matched.

Supported commands:

| Command | Behavior |
| --- | --- |
| `led on` | Sets the LED mode to on and replies `ok,led=on`. |
| `led off` | Sets the LED mode to off and replies `ok,led=off`. |
| `led toggle` | Switches between on and off, then reports the new LED mode. |
| `led blink` | Sets the LED mode to blink and replies `ok,led=blink`. |
| `temp` | Sends one labeled status line with temperature and LED mode. |
| `temp float` | Enables telemetry, switches to numeric-only telemetry, and sends one value immediately. |
| `temp status` | Switches telemetry back to labeled status messages. |
| `temp start` | Enables automatic telemetry. |
| `temp stop` | Disables automatic telemetry. |
| `help` | Sends the command list. |

The parser uses exact string matches. That means `LED ON`, `led  on`, and
`led on now` are not accepted unless new parsing logic is added.

### `handle_ble_tick()`

```c
static void handle_ble_tick(void)
```

Runs once per BLE stack tick. In `lib/pico_ble_stack/pico_ble_stack.c`, that
tick is currently configured as a 1000 ms heartbeat.

This function handles periodic behavior rather than one-time command behavior.
It has two independent jobs.

First, it sends telemetry if both conditions are true:

- `telemetry_enabled` is true.
- `pico_ble_stack_uart_is_connected()` reports an active BLE UART connection.

When telemetry is active, it increments `telemetry_counter`. Then it sends
either:

- a numeric-only temperature from `send_temperature_float()`, or
- a labeled status packet like `demo,1,temp_c=27.98`.

Second, it updates the LED:

- In `LED_MODE_BLINK`, it toggles a private `static bool led_on` value and
  writes that value to the LED pin.
- In steady on/off modes, it calls `apply_led_mode()` so the physical LED stays
  aligned with `led_mode`.

The key idea is that commands change state, while the tick repeatedly acts on
that state.

### `handle_ble_connection_change()`

```c
static void handle_ble_connection_change(bool connected)
```

Logs BLE UART connection changes to local stdio.

It does not change application state. Its purpose is visibility while debugging:

```text
Bluefruit UART connected
Bluefruit UART disconnected
```

### `main()`

```c
int main(void)
```

Initializes the firmware and hands control to the BLE stack.

The important steps are:

1. Create `ble_handlers`, a callback table that points the BLE stack at this
   file's application functions:
   - `handle_uart_command` for received phone text.
   - `handle_ble_tick` for periodic work.
   - `handle_ble_connection_change` for connect/disconnect logs.
2. Call `stdio_init_all()` so `printf()` can write to enabled stdio backends.
3. Call `picotemp_init()` so temperature reads are ready.
4. Set the BLE device name to `PICO-BLE-DEMO`.
5. Register the callback table with `pico_ble_stack_set_handlers()`.
6. Start the BLE stack with `pico_ble_stack_run()`.

`pico_ble_stack_run()` does not return during normal operation. It owns the
event loop after startup.

## Runtime Flow

Startup flow:

```text
main()
  -> stdio_init_all()
  -> picotemp_init()
  -> pico_ble_stack_set_device_name("PICO-BLE-DEMO")
  -> pico_ble_stack_set_handlers(...)
  -> pico_ble_stack_run()
```

Command flow:

```text
Phone sends "led on"
  -> BLE stack receives UART data
  -> handle_uart_command(packet, size)
  -> copy_command(...)
  -> command_equals(command, "led on")
  -> set_led_mode(LED_MODE_ON)
  -> pico_ble_stack_uart_send("ok,led=on\r\n")
```

Telemetry flow:

```text
BLE stack tick
  -> handle_ble_tick()
  -> if telemetry is enabled and UART is connected
  -> read_temperature_c()
  -> send formatted BLE UART line
```

Blink flow:

```text
Phone sends "led blink"
  -> handle_uart_command()
  -> led_mode = LED_MODE_BLINK

Every later BLE tick
  -> handle_ble_tick()
  -> toggle LED pin
```

## How To Extend It

To add a new text command, add another exact-match block inside
`handle_uart_command()` before the final unknown-command error.

Example:

```c
if (command_equals(command, "hello"))
{
    pico_ble_stack_uart_send("hello from pico\r\n");
    return;
}
```

To add new periodic behavior, place it in `handle_ble_tick()`. Keep command
handlers focused on changing state or sending immediate replies, and keep the
tick focused on repeated work.

To expose more status, extend the `temp` reply or add a new status command.
Prefer consistent machine-readable text such as:

```text
status,temp_c=27.98,led=blink
```

That format is easy to read by humans and easy to parse from a phone app or
script.
