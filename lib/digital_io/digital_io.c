#include "digital_io.h"

// Track if CYW43 has been initialized (Pico W only)
static bool digital_io_initialized = false;

void digitalIO_init(void) {
#if DIGITAL_IO_HAS_CYW43
    if (digital_io_initialized) return;

    // Initialize CYW43 (needed for Pico W LED)
    if (cyw43_arch_init()) {
        // Initialization failed - LED + wifi unavailable
        return;
    }

    digital_io_initialized = true;
#endif
}

void pinMode(uint pin, uint mode) {
    // Make sure system is initialized
    digitalIO_init();

#if DIGITAL_IO_HAS_CYW43
    // Special case: Pico W onboard LED (CYW43)
    if (pin == PICO_W_LED_PIN) {
        // No pinMode needed, LED is managed through cyw43
        return;
    }
#endif

    gpio_init(pin);

    switch (mode) {
        case INPUT:
            gpio_set_dir(pin, GPIO_IN);
            gpio_disable_pulls(pin);
            break;

        case INPUT_PULLUP:
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_up(pin);
            break;

        case INPUT_PULLDOWN:
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_down(pin);
            break;

        case OUTPUT:
            gpio_set_dir(pin, GPIO_OUT);
            gpio_disable_pulls(pin);
            break;

        default:
            gpio_set_dir(pin, GPIO_IN);
            break;
    }
}


void digitalWrite(uint pin, uint value) {
    digitalIO_init();

#if DIGITAL_IO_HAS_CYW43
    // Special case for Pico W LED
    if (pin == PICO_W_LED_PIN) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, value);
        return;
    }
#endif

    gpio_put(pin, value ? 1 : 0);
}


int digitalRead(uint pin) {
    digitalIO_init();

#if DIGITAL_IO_HAS_CYW43
    // LED cannot be read
    if (pin == PICO_W_LED_PIN) {
        return LOW;
    }
#endif

    return gpio_get(pin);
}
