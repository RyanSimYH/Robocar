/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define GPIO_26_IN 26
void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it

    uint16_t result = 0;
    char data[100] = "";

    result = gpio_get(26);
    snprintf(data, 1, "%d", result);
    printf("data: %d\n", result);

}
int main() {
    stdio_init_all();
    printf("Digital Wheel Encoder Example, measuring GPIO26\n");

    gpio_init(GPIO_26_IN);

    // Make sure GPIO is high-impedance, no pullups etc
    gpio_set_pulls(GPIO_26_IN, true, false);
    // Select ADC input 0 (GPIO26)
    gpio_set_dir(GPIO_26_IN, GPIO_IN);

    // Set Interrupt callback for GPIO26
    gpio_set_irq_enabled_with_callback(GPIO_26_IN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    uint16_t result = 0;
    while (1);
}

