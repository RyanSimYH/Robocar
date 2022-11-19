/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define IN3 4
#define IN4 3
#define ENB 2
#define IN1 6
#define IN2 7
#define ENA 8
#define IR_GPIO 26
#define NO_SAMPLE 10
void forward();
void reverse();
static float averageData = 0;
static uint8_t sampleCount = 0;
static char sampleArray[28];
static uint16_t sampleArrayIndex = 0;

// Function for timer interupt
bool repeating_timer_callback(struct repeating_timer *t)
{
    // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
    // 4096 because it is a 12bit channel
    const float conversion_factor = 3.3f / 4096;
    uint16_t result = adc_read();
    //        printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);
    float voltage = result * conversion_factor;
    //
    if (sampleCount < NO_SAMPLE)
    {
        averageData += voltage;

        sampleCount++;
    }
    else
    {
        // TO get the Average Data of result
        averageData /= NO_SAMPLE;

        // TODO: To be removed when completed
        printf("Data Value : %f \n", averageData);

        sampleCount = 0;

        // Check if it have not exceed array size
        if (sampleArrayIndex < 28)
        {
            // Note : modify the value to fit the environment s s
            if (averageData < 0.2)
            {
                // Value for White
                sampleArray[sampleArrayIndex] = '0';
            }
            else
            {
                // Value for black
                sampleArray[sampleArrayIndex] = '1';
            }

            // Just to print out result of data
            for (int i = 0; i < sampleArrayIndex; i++)
            {
                printf("%c", sampleArray[i]);
            }
            printf("\n");
            sampleArrayIndex++;
        }
    }

    return true;
}

int main()
{
    stdio_init_all();

    printf("-------- Welcome to barcode --------\n");
    adc_init();

    // Initialize GPIO
    gpio_init(IN3);
    gpio_init(IN4);
    gpio_init(ENB);
    gpio_init(IN1);
    gpio_init(IN2);
    gpio_init(ENA);
    gpio_init(22);

    // Set Direction for GPIO
    gpio_set_dir(IN3, GPIO_OUT);
    gpio_set_dir(IN4, GPIO_OUT);
    gpio_set_dir(ENB, GPIO_OUT);
    gpio_set_dir(IN1, GPIO_OUT);
    gpio_set_dir(IN2, GPIO_OUT);
    gpio_set_dir(ENB, GPIO_OUT);
    gpio_set_dir(22, GPIO_IN);

    gpio_put(ENB, 1);
    gpio_put(ENA, 1);

    adc_gpio_init(IR_GPIO);

    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    struct repeating_timer timer;

    // Timer Interrupt
    // Occur every 0.1sec
    add_repeating_timer_ms(100, repeating_timer_callback, NULL, &timer);
    while (1)
    {
        tight_loop_contents();
    }
}

void forward()
{
    gpio_put(IN1, 1);
    gpio_put(IN2, 0);
    gpio_put(IN3, 1);
    gpio_put(IN4, 0);
}
void reverse()
{
    gpio_put(IN1, 0);
    gpio_put(IN2, 1);
    gpio_put(IN3, 0);
    gpio_put(IN4, 1);
}
