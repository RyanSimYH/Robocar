/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"


/// \tag::uart_advanced[]

#define UART_ID uart0
#define UART_ID_M5 uart1
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define UART_TX_PIN_M5 4
#define UART_RX_PIN_M5 5

static int chars_rxed = 0;

// Communications variables
unsigned int previousIntegerData[5];
static char *datapointNames[6]={"speed","turning","distanceWhole","distanceDecimal", "humpHeight", "barcode"};
unsigned int currentIntegerData[5];
#define SPEED = 0; // round to a whole number
#define TURNING = 1; // 0 = straight, 1 = L, 2 = R
#define DISTANCE_WHOLE = 2;
#define DISTANCE_DECIMAL = 3;
#define HUMP_HEIGHT = 4;
#define BARCODE = 5;
// don't do checking for barcode
// navigation endpoint placeholder
// navigation data placeholder

char fullString[255] = "{"; // change to 512 if sending nav data too

const int maxDatapoints = 7; // including 1 for navigation


// Function prototypes
void getIntDatapoints(void);
void comms(void);

// // RX interrupt handler for Pico to read from M5 to serial
// void on_uart_rx() {
//     while (uart_is_readable(UART_ID)) {
//         uint8_t ch = uart_getc(UART_ID);
//         // Can we send it back?
//         if (uart_is_writable(UART_ID)) { // print to terminal
//             uart_putc(UART_ID, ch);
//         }
//         if (uart_is_writable(UART_ID_M5)) { // write to M5
//             uart_putc(UART_ID_M5, ch);
//         }
//         chars_rxed++;
//     }
// }

int main() {
    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    // OK, all set up.
    // Lets send a basic string out, and then run a loop and wait for RX interrupts
    // The handler will count them, but also reflect the incoming data back with a slight change!
    uart_puts(UART_ID, "\nHello, uart interrupts\n");

    while (1){
        // other sub modules
        comms();
    }
        
}

void comms(void){
    int datapoints = 0; // number of datapoints in the message to be sent
    int i=0; //for-loop counter
    if(barcodeAvailable() == 1){
        strcat(fullString, datapointNames[BARCODE]);
        strcat(fullString, getBarcode()); //barcode is string, once getBarcode is called, barcode side \
                                                should reset its barcodeAvail to not avail
        datapoints++;
    }

    getIntDatapoints(); //fetch all the integer datapoints at once

    for(i=0; i<5; i++){
        // compare the integer datapoints with their previous values, \
        send only those that have changed
        if(currentIntegerData[i] != previousIntegerData[i]){
            // structure the message
            if(datapoints != 0){
                strcat(fullString, ", ");
            }
            strcat(fullString, "\"");
            strcat(fullString, datapointNames[i]);
            strcat(fullString, "\": ");
            strcat(fullString, currentIntegerData[i]);
            
            // store current value as prev value for the next round
            previousIntegerData[i] = currentIntegerData[i]; 
            datapoints++; // increment number of datapoints within the message
        }
    }

    if(datapoints > 0){
        
        //##################### Arshad's Part ########################
        //software interrupt
        //{         
            strcat(fullString, "}"); //finish packaging the message since datapoints have been gathered
            // ################ Jacelyn's Part ################
            // send fullString to M5 stick via UART
            uart_puts(UART_ID, fullString);

        //}

        /* clear the message */
        memset(fullString, '\0', strlen(fullString));
        strcat(fullString, "{");
    }
}

void getIntDatapoints(void){
    currentData[0] = getSpeed();
    currentData[1] = getTurning();
    currentData[2] = getDistWhole();
    currentData[3] = getDistDecimal();
    currentData[4] = getHump_Height();
}
