#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize serial (USB/UART depending on stdio config)
void serial_init(void);

// Basic printing
void serial_print(const char *s);
void serial_println(const char *s);

// Characters
void serial_print_char(char c);
void serial_println_void(void);

// Integers
void serial_print_int(int value);
void serial_println_int(int value);

// Unsigned
void serial_print_uint(unsigned int value);
void serial_println_uint(unsigned int value);

// Float (digits = decimals)
void serial_print_float(float value, int digits);
void serial_println_float(float value, int digits);

// Hex (32-bit)
void serial_print_hex(uint32_t value);
void serial_println_hex(uint32_t value);

#ifdef __cplusplus
}
#endif
