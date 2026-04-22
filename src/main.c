#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/adc.h"
#include "picotemp.h"
#include "lowpass.h"
#include "math.h"
#include "serial_print.h"
#include "digital_io.h"
#include "pico_debug_uart.h"
#include "pico/cyw43_arch.h"


// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 4
#define UART_RX_PIN 5


const uint LED_PIN = CYW43_WL_GPIO_LED_PIN;
//const uint HIGH = 1;
//const uint LOW = 0;

const uint delayTime = 50;

#define TEMPERATURE_UNITS 'C'

LowPassFilter lowPassFilter;
float unfiltered = 0;
float filtered = 0;

/* References for this implementation:
 * raspberry-pi-pico-c-sdk.pdf, Section '4.1.1. hardware_adc'
 * pico-examples/adc/adc_console/adc_console.c */

static bool get_signal_for_filtering(repeating_timer_t *rt)
{
    // ---- 1) Read ADC
    uint16_t raw = adc_read();

    // ---- 2) Convert to voltage
    float voltage = raw * 3.3f / 4095.0f;

    // ---- 3) Remove ACS712 zero offset (mid-rail)
    // If you did auto-zero at startup, use that instead of 1.65f
    float v_no_offset = voltage - 1.65f;

    // ---- 4) Convert to current (A)
    // For ACS712-05: 185 mV/A = 0.185 V/A
    const float sensitivity = 0.185f;
    float current = v_no_offset / sensitivity;

    // ---- 5) Keep an unfiltered copy (useful for debugging)
    unfiltered = current;

    // ---- 6) Low-pass filter the current (not the raw voltage!)
    filtered = lowpass_filt(&lowPassFilter, current);

    return true; // keep repeating
}


int main()
{
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));
    // gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    // gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // stdio_init_all();
    // cyw43_arch_init();

    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
    // analog input for filter
    adc_gpio_init(28);   // enable GPIO28 as ADC
    adc_select_input(2); // select ADC channel 2

    // gpio_init(LED_PIN);
    // gpio_set_dir(LED_PIN, GPIO_OUT);

    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART

    // Send out a string, with CR/LF conversions
    uart_puts(UART_ID, " Hello, UART!\n");

    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    // filter
    // lowpass_init(&lowPassFilter, 3.0f, 0.707f, 1000.0f); // 3 Hz LPF
    lowpass_init(&lowPassFilter, 2.0f, Q_BUTTERWORTH, 250.0f);

    // load timer, timer handel
    repeating_timer_t timer;

    // repeating timer -4 means everyy 4 ms and ron right away if +4 means run after 4ms, the function that is called, NULL not used, timer where SDK stores timer data
    add_repeating_timer_ms(-4, get_signal_for_filtering, NULL, &timer);

    // init serial
    serial_init();
    // init digital io
    digitalIO_init();
    // led as output
    pinMode(LED_PIN, OUTPUT);

    // init uart

    while (true)
    { // main loop taimer started
        uint64_t start = time_us_64();

        digitalWrite(LED_PIN, HIGH);
        // cyw43_arch_gpio_put(LED_PIN, HIGH);
        //  sends to UART
        uart_puts(UART_ID, " LED ON!\n");
        // sends to USB
        // printf("LED ON!\n");
        sleep_ms(delayTime);

        digitalWrite(LED_PIN, LOW);
        // cyw43_arch_gpio_put(LED_PIN, LOW);
        //  sends to USB
        // printf("LED OFF!\n");
        //  sends to UART
        uart_puts(UART_ID, "LED OFF\n");
        sleep_ms(delayTime);

        uart_putc_raw(UART_ID, 'U');

        // Send out a character but do CR/LF conversions
        uart_putc(UART_ID, 'A');
        float temperature = read_onboard_temperature(TEMPERATURE_UNITS);
        // printf("Temp = %.02f %c\n", temperature, TEMPERATURE_UNITS);

        // sends string over UART
        uart_puts(UART_ID, " TEMP: ");
        // sends temp float over uart

        char buffer[10]; // Buffer for float conversion

        // Convert float to string and send
        snprintf(buffer, sizeof(buffer), "%.4f\n", temperature);
        uart_puts(UART_ID, buffer);

        // use of pico_debug_uart

        serial_print_float(unfiltered, 2);
        serial_print(" ");
        serial_println_float(filtered, 2);

        // main loop taimer stoped
        uint64_t end = time_us_64();
        uint64_t duration_us = end - start;
        /* serial_println(" ");
        serial_print_int(duration_us);
        serial_println(" "); */

     
    }
}
