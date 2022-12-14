

/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * Code 39 Barcode Reader
 * Team-B2
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define K_CONST 0.5
#define IR_GPIO 26
#define SAMPLE_COUNT 5
#define BLACK 1
#define WHITE 0
// #define THRESHOLD 0.031
#define THRESHOLD 0.034
// Lookup table for the code39
static const uint32_t g_barcode_lookup_table[44] = {
    111221211, 211211112, 112211112, 212211111, 111221112,
    211221111, 112221111, 111211212, 211211211, 112211211,
    211112112, 112112112, 212112111, 111122112, 211122111,
    112122111, 111112212, 211112211, 112112211, 111122211,
    211111122, 112111122, 212111121, 111121122, 211121121,
    112121121, 111111222, 211111221, 112111221, 111121221,
    221111112, 122111112, 222111111, 121121112, 221121111,
    122121111, 121111212, 221111211, 122111211, 121212111,
    121211121, 121112121, 111212121, 121121211};

// Match to Lookup Table for Code39 for DECODED RESULT
static const char g_barcode_lookup_match[44] = {
    '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z', '-', '.', ' ', '$',
    '/', '+', '%', '*'};

// Function used to decode binary
void decodeBarcode(uint16_t barcode_index, uint16_t barcode_arr[30]);

// Used to find Maxmimum value detected for narrow bar
uint16_t findMax(uint16_t secondData, uint16_t thirdData, uint16_t fourthData, uint16_t fifthData, uint16_t sixthData);

/**
 * Digital Filter
 * Using a specific algorithm to calculate
 * By combining 2 different digital filtering
 */
bool repeating_timer_callback(struct repeating_timer *t)
{
    static uint8_t sample_index = 0;   // Indexing for Sampling
    static int16_t barcode_index = -1; // Barcode not detected yet.
    static uint8_t previous_bin = 1;   // 0 for white 1 for black, Starts off black
    static uint16_t barcode_array[30]; // Array to store the data of barcode
    static float average_data = 0;     // Average Data of Sample

    if (barcode_index < 20)
    {
        // PICO 12 bit ADC - 4096 taking for the adc voltage used 3.3v
        const float conversion_factor = 3.3f / 4096;
        // Read ADC Value
        uint16_t result = adc_read();

        // Calculate Voltage
        float voltage = result * conversion_factor;

        // Check if index of sample is sample count
        if (sample_index < SAMPLE_COUNT)
        {
            // Algorithm to first Filter with equations
            average_data = K_CONST * voltage + (1.0 - K_CONST) * average_data;

            sample_index++;
        }
        else
        {

            // Get average data from sample for digital filtering 2.
            average_data /= SAMPLE_COUNT;

            // Note : modify the value to fit the environment
            if (average_data > THRESHOLD)
            {
                // Black Detected

                if (previous_bin == WHITE)
                {
                    printf("Bar detected : %d \n", barcode_array[barcode_index - 1]);
                    barcode_index++;
                    previous_bin = BLACK;
                }
            }
            else
            {
                // White Detected

                // To determine if barcode has started scanning.
                if (barcode_index == -1)
                {
                    barcode_index++;
                    previous_bin = WHITE;
                }
                else
                {
                    // Check if previo
                    if (previous_bin == BLACK)
                    {

                        printf("Bar detected : %d \n", barcode_array[barcode_index - 1]);

                        barcode_index++;
                        previous_bin = WHITE;
                    }
                }
            }

            if (barcode_index != -1)
            {
                barcode_array[barcode_index] += 1;
            }

            // Reset Average Data and Sample Index
            average_data = 0;

            sample_index = 0;
        }
    }
    else
    {
        // Decode barcode
        printf("Decoding barcode in progress ...\n");
        decodeBarcode(barcode_index, barcode_array);

        printf("Clearing Barcode Memory...\n");

        memset(barcode_array, 0, sizeof barcode_array);

        barcode_index = -1;
    }
    return true;
}

int main()
{
    stdio_init_all();

    printf("-------- Welcome to barcode --------\n");
    adc_init();

    gpio_init(IR_GPIO);

    gpio_set_dir(IR_GPIO, GPIO_IN);

    adc_gpio_init(IR_GPIO);

    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    struct repeating_timer timer;

    // Timer Interrupt
    // Occur every 3m
    add_repeating_timer_ms(1, repeating_timer_callback, NULL, &timer);
    while (1)
    {

        tight_loop_contents();
    }
}
void decodeBarcode(uint16_t barcode_index, uint16_t barcode_arr[30])
{
    // Code 39 Start and End will always Contain this binary 1000101110111010
    if (barcode_index >= 20)
    {
        uint16_t min = 0;
        uint32_t barcodeValue = 0;
        uint8_t barcodeIndex = 9;
        uint16_t firstMax = findMax(barcode_arr[3], barcode_arr[4], barcode_arr[6], barcode_arr[8], barcode_arr[10]);

        // Decode Normally
        for (int i = 11; i < 20; i++)
        {
            int temp_value = 0;
            if (barcode_arr[i] > firstMax)
            {
                temp_value = 2;
            }
            else
            {
                temp_value = 1;
            }

            for (int k = 0; k < barcodeIndex - 1; k++)
            {
                temp_value *= 10;
            }
            barcodeValue += temp_value;
            barcodeIndex--;
        }

        for (int i = 0; i < 44; i++)
        {

            if (g_barcode_lookup_table[i] == barcodeValue)
            {

                printf("Result : ");

                printf("%c", g_barcode_lookup_match[i]);
                printf(" \n");
                break;
            }
        }
    }
}

uint16_t findMax(uint16_t secondData, uint16_t thirdData, uint16_t fourthData, uint16_t fifthData, uint16_t sixthData)
{
    // // If 1st value is Largest
    // if (firstData > secondData && firstData > thirdData && firstData > fourthData && firstData > fifthData && firstData > sixthData)
    // {
    //     return firstData;
    // }

    // If 2nd value is largest
    if (secondData > thirdData && secondData > fourthData && secondData > fifthData && secondData > sixthData)
    {
        return secondData;
    }

    // If 3rd data is largest
    if (thirdData > secondData && thirdData > fourthData && thirdData > fifthData && thirdData > sixthData)
    {
        return thirdData;
    }

    // If 4th data is largest
    if (fourthData > secondData && fourthData > thirdData && fourthData > fifthData && fourthData > sixthData)
    {
        return fourthData;
    }

    // If 5th data is largest
    if (fifthData > secondData && fifthData > thirdData && fifthData > fourthData && fifthData > sixthData)
    {
        return fifthData;
    }

    // if 6th data is largest
    if (sixthData > secondData && sixthData > thirdData && sixthData > fourthData && sixthData > fifthData)
    {
        return sixthData;
    }
}