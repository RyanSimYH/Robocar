1. Throughput: MSP-UART Transmissions
// Relevant settings: 12MHz, 115200 baud, LSB first, no parity, LSB first, one stop bit
- We sent char arrays of various lengths (2k, 4k, 6k, 8k, 10k) through UART
- Each length of char array was sent 5 times, and we used the average time taken for each array length for our plotting
- Time taken for each attempt is printed to the serial terminal
- Time taken refers to the time for the UART transfer function to be completed (recorded via timerA)
![image](https://user-images.githubusercontent.com/98366701/204077869-77825e93-bb2d-47c6-92d2-1cfe3ff21543.png)

Based on the line of best fit, **y = 0.1731x - 0.0001**:
  If y = 1 second, amount of data we can transmit (x) is
    (y + 0.0001)/0.1731 = 1.0001/0.1731 = 5.778
Therefore, the throughput of MSP-UART transmissions, based on our configurations, is 5.778kBps

2. Throughput: Pico-HC05-UART Transmissions
// Relevant settings: 9600 baud, LSB first, no parity, one stop bit
- We sent char arrays of various lengths (2k, 4k, 6k, 8k, 10k) through UART
- Each length of char array was sent 5 times, and we used the average time taken for each array length for our plotting
- Time taken for each attempt is printed to the serial terminal
- Time taken refers to the time for the UART transfer function to be completed (recorded via Pico's time.h API)
![pico_ble_tp](https://user-images.githubusercontent.com/81850188/204078885-03c767d0-8c70-44a3-a681-62db510ab8a0.png)

Based on the line of best fit, **y = 2.0831x - 0.0341**:
  If y = 1 second, amount of data we can transmit (x) is
    (1 + 0.0341)/2.0831 = 0.49642
Therefore, the throughput is 0.4964kBps

3. Latency: Pico-HC05-UART Transmissions
// Relevant settings: 38400 baud, LSB first, no parity, one stop bit
- Command 'AT' was sent 5 times through UART
- Time taken for each attempt where the response is received is printed to the serial terminal
- Time taken refers to the time for the command to be sent to HC05 and respond back with OK (recorded via Pico's time.h API)
![pico_ble_lt](https://user-images.githubusercontent.com/81850188/204079517-95f66838-d6bf-4854-be49-8771bc06a58d.png)

Based on the graph,
  The average latency is 14.0164ms and transfer speed could be slower or faster based on a variance of 1.88us.
