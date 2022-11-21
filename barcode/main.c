

/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * Code 39 Barcode Reader
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define IR_GPIO 26
#define SAMPLE_COUNT 5
#define BLACK 1
#define WHITE 0

static const uint32_t code39LookupTable[44] = {
    111221211, 211211112, 112211112, 212211111, 111221112,
    211221111, 112221111, 111211212, 211211211, 112211211,
    211112112, 112112112, 212112111, 111122112, 211122111,
    112122111, 111112212, 211112211, 112112211, 111122211,
    211111122, 112111122, 212111121, 111121122, 211121121,
    112121121, 111111222, 211111221, 112111221, 111121221,
    221111112, 122111112, 222111111, 121121112, 221121111,
    122121111, 121111212, 221111211, 122111211, 121212111,
    121211121, 121112121, 111212121, 121121211};

static const char code39LookupMatch[44] = {
    '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z', '-', '.', ' ', '$',
    '/', '+', '%', '*'};

static uint8_t sampleIndex = 0;
static int16_t barcodeBarIndex = -1; // Basrcode not detected yet.
static uint8_t previousBin = 1;      // 0 for white 1 for black, Starts off black
static float averageData = 0;
static uint16_t barcodeArray[30];

void decodeBarcode();

// Used to find Maxmimum value detected for narrow bar
uint16_t findMax(uint16_t firstData, uint16_t secondData, uint16_t thirdData, uint16_t fourthData, uint16_t fifthData, uint16_t sixthData);

/**
 * Digital Filter
 * Using A specific number samples to get 1 result
 * Result per specific number of sample is
 * duration of timer interrupt * SAMPLE_COUNT
 * Using the voltage to determine black and white
 */
bool repeating_timer_callback(struct repeating_timer *t)
{

    if (barcodeBarIndex < 26)
    {
        // PICO 12 bit ADC - 4096 taking for the adc voltage used 3.3v
        const float conversion_factor = 3.3f / 4096;
        // Read ADC Value
        uint16_t result = adc_read();

        // Calculate Voltage
        float voltage = result * conversion_factor;

        // Check if index of sample is sample count
        if (sampleIndex < SAMPLE_COUNT)
        {
            averageData += voltage;

            sampleIndex++;
        }
        else
        {

            // Get average data from sample for digital filtering.
            averageData /= SAMPLE_COUNT;

            // Note : modify the value to fit the environment
            if (averageData > 0.171)
            {
                // Black Detected

                if (previousBin == WHITE)
                {
                    printf("Bar detected : %d \n", barcodeArray[barcodeBarIndex - 1]);
                    barcodeBarIndex++;
                    previousBin = BLACK;
                }
            }
            else
            {
                // White Detected

                // To determine if barcode has started scanning.
                if (barcodeBarIndex == -1)
                {
                    barcodeBarIndex++;
                    previousBin = WHITE;
                }
                else
                {
                    // Check if previo
                    if (previousBin == BLACK)
                    {

                        printf("Bar detected : %d \n", barcodeArray[barcodeBarIndex - 1]);

                        barcodeBarIndex++;
                        previousBin = WHITE;
                    }
                }
            }

            if (barcodeBarIndex != -1)
            {
                barcodeArray[barcodeBarIndex] += 1;
            }

            // Reset Average Data and Sample Index
            averageData = 0;

            sampleIndex = 0;
        }
    }
    else
    {
        printf("Decoding barcode in progress ...\n");
        decodeBarcode();

        printf("Clearing Barcode Memory...\n");

        memset(barcodeArray, 0, sizeof barcodeArray);

        barcodeBarIndex = -1;
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
    add_repeating_timer_ms(3, repeating_timer_callback, NULL, &timer);
    while (1)
    {

        tight_loop_contents();
    }
}
void decodeBarcode()
{
    // Code 39 Start and End will always Contain this binary 1000101110111010
    if (barcodeBarIndex >= 20)
    {
        uint16_t min = 0;
        uint32_t barcodeValue = 0;
        uint8_t barcodeIndex = 9;
        uint16_t firstMax = findMax(barcodeArray[1], barcodeArray[3], barcodeArray[4], barcodeArray[6], barcodeArray[8], barcodeArray[10]);

        // Decode Normally
        for (int i = 11; i < 20; i++)
        {
            int temp_value = 0;
            if (barcodeArray[i] > firstMax)
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

            if (code39LookupTable[i] == barcodeValue)
            {

                printf("Result : ");

                printf("%c", code39LookupMatch[i]);
                printf(" \n");
                break;
            }
        }
    }
}

uint16_t findMax(uint16_t firstData, uint16_t secondData, uint16_t thirdData, uint16_t fourthData, uint16_t fifthData, uint16_t sixthData)
{
    // If 1st value is Largest
    if (firstData > secondData && firstData > thirdData && firstData > fourthData && firstData > fifthData && firstData > sixthData)
    {
        return firstData;
    }

    // If 2nd value is largest
    if (secondData > firstData && secondData > thirdData && secondData > fourthData && secondData > fifthData && secondData > sixthData)
    {
        return secondData;
    }

    // If 3rd data is largest
    if (thirdData > firstData && thirdData > secondData && thirdData > fourthData && thirdData > fifthData && thirdData > sixthData)
    {
        return thirdData;
    }

    // If 4th data is largest
    if (fourthData > firstData && fourthData > secondData && fourthData > thirdData && fourthData > fifthData && fourthData > sixthData)
    {
        return fourthData;
    }

    // If 5th data is largest
    if (fifthData > firstData && fifthData > secondData && fifthData > thirdData && fifthData > fourthData && fifthData > sixthData)
    {
        return fifthData;
    }

    // if 6th data is largest
    if (sixthData > firstData && sixthData > secondData && sixthData > thirdData && sixthData > fourthData && sixthData > fifthData)
    {
        return sixthData;
    }
}