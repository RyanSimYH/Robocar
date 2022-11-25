/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "hardware/irq.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "time.h"

/// \tag::uart_advanced[]

#define UART_ID uart1
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 4
#define UART_RX_PIN 5

static volatile int i = 0;
static volatile int sendPacket = 1;
char buffer[1000] =
    "TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTE"
    "STTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST"
    "TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTE"
    "STTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST"
    "TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTE"
    "STTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST"
    "TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTE"
    "STTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST"
    "TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTE"
    "STTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST"
    "TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTE"
    "STTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST"
    "TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTE"
    "STTESTTESTTESTTESTTESTTESTTESTTESTTES";

int main() {
  stdio_init_all();
  // Set up our UART with a basic baud rate.
  uart_init(UART_ID, 2400);

  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  // Actually, we want a different speed
  // The call will return the actual baud rate selected, which will be as close
  // as possible to that requested
  int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

  // Set UART flow control CTS/RTS, we don't want these, so turn them off
  uart_set_hw_flow(UART_ID, false, false);

  // Set our data format
  uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

  for (; i < 35; i++) {
    sleep_ms(5000);

    uint64_t startTime = time_us_64();
    uart_puts(UART_ID, buffer);
    uart_puts(UART_ID, buffer);
    uart_puts(UART_ID, buffer);
    uart_puts(UART_ID, buffer);
    uart_puts(UART_ID, buffer);
    uart_puts(UART_ID, buffer);
    uart_puts(UART_ID, buffer);
    uart_puts(UART_ID, buffer);
    uart_puts(UART_ID, buffer);
    uart_puts(UART_ID, buffer);
    uint64_t endTime = time_us_64();

    double executionTime = (double)(endTime - startTime) / 1000000;
    printf("Attempt %i: %.8f sec\n", i, executionTime);
  }
}

/// \end:uart_advanced[]
