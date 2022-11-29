/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include "hardware/irq.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "time.h"

/// \tag::uart_advanced[]

#define UART_ID uart1
#define BAUD_RATE 38400
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 4
#define UART_RX_PIN 5

volatile int i = 1;

volatile uint64_t startTime;
volatile uint64_t endTime;

// char buffer[3] = "AT";

void on_uart_rx() {
  printf("Response: ");
  while (uart_is_readable(UART_ID)) {
    printf("%c", uart_getc(UART_ID));
  }

  endTime = time_us_64();
  double executionTime = (double)(endTime - startTime) / 1000000;
  printf("Attempt %i, time: %.8f sec\n", i, executionTime);
  i++;
}

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

  // Set up a RX interrupt
  // We need to set up the handler first
  // Select correct interrupt for the UART we are using
  int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

  // And set up and enable the interrupt handlers
  irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
  irq_set_enabled(UART_IRQ, true);

  // Now enable the UART to send interrupts - RX only
  uart_set_irq_enables(UART_ID, true, false);

  while (i < 6) {
    sleep_ms(7000);

    printf("\nSent: AT\n");
    startTime = time_us_64();
    uart_puts(UART_ID, "AT\r\n");
  }
}

/// \end:uart_advanced[]
