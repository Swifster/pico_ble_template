#include <stdio.h>

#include "pico/stdlib.h"
#include "pico_ble_stack.h"

int main(void)
{
    stdio_init_all();
    return pico_ble_stack_run();
}
