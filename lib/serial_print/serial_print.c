#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "serial_print.h"

static bool serial_stdio_initialized = false;

void serial_init(void) {
    if (serial_stdio_initialized) {
        return;
    }

    stdio_init_all();
    sleep_ms(10); // allow stdio backends such as USB CDC to come up
    serial_stdio_initialized = true;
}

// strings
void serial_print(const char* s) {
    if (!s) {
        return;
    }
    printf("%s", s);
}

void serial_println(const char* s) {
    if (!s) {
        return;
    }
    printf("%s\n", s);
}

// characters
void serial_print_char(char c) {
    putchar(c);
}

void serial_println_void(void) {
    printf("\n");
}

// integers
void serial_print_int(int value) {
    printf("%d", value);
}

void serial_println_int(int value) {
    printf("%d\n", value);
}

// unsigned
void serial_print_uint(unsigned int value) {
    printf("%u", value);
}

void serial_println_uint(unsigned int value) {
    printf("%u\n", value);
}

// float
void serial_print_float(float value, int digits) {
    printf("%.*f", digits, value);
}

void serial_println_float(float value, int digits) {
    printf("%.*f\n", digits, value);
}

// hex
void serial_print_hex(uint32_t value) {
    printf("0x%08X", value);
}

void serial_println_hex(uint32_t value) {
    printf("0x%08X\n", value);
}
