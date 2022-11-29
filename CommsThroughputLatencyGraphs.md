1. Throughput: MSP-M5StickC-UART Transmissions
// Relevant settings: 12MHz, 115200 baud, LSB first, no parity, LSB first, one stop bit
- We sent char arrays of various lengths (2k, 4k, 6k, 8k, 10k) through UART
- Each length of char array was sent 5 times, and we used the average time taken for each array length for our plotting
- Time taken for each attempt is printed to the serial terminal
- Time taken refers to the time for the UART transfer function to be completed (recorded via timerA)
![image](https://user-images.githubusercontent.com/98366701/204511564-21e0773e-a55a-49df-aff9-028a8a1c39ff.png)

Based on the line of best fit, **y = 0.0867x - 0.0001**:
  If y = 1 second, amount of data we can transmit (x) is
    (y + 0.0001)/0.0867 = 1.0001/0.0867 = 11.535
Therefore, the throughput of MSP-UART transmissions, based on our configurations, is 11.535kBps

2. Throughput: Pico-M5StickC-UART Transmissions
// Relevant settings: 115200 baud, LSB first, no parity, one stop bit
- We sent char arrays of various lengths (2k, 4k, 6k, 8k, 10k) through UART
- Each length of char array was sent 5 times, and we used the average time taken for each array length for our plotting
- Time taken for each attempt is printed to the serial terminal
- Time taken refers to the time for the UART transfer function to be completed (recorded via Pico's time.h API)
![image](https://user-images.githubusercontent.com/94168656/204120091-d30ae8a9-d969-400f-b020-1577f03f4a03.png)

Based on the line of best fit, **y = 0.1736x - 0.0028**:
  If y = 1 second, amount of data we can transmit (x) is
    (y + 0.0028)/0.1736 = 1.0028/0.1736 = 5.776
Therefore, the throughput of MSP-UART transmissions, based on our configurations, is 5.776kBps

3. Throughput: MSP-HC05-UART Transmissions
// Relevant settings: 9600 baud, LSB first, no parity, one stop bit
- We sent char arrays of various lengths (2k, 4k, 6k, 8k, 10k) through UART
- Each length of char array was sent 5 times, and we used the average time taken for each array length for our plotting
- Time taken for each attempt is printed to the serial terminal
- Time taken refers to the time for the UART transfer function to be completed (recorded via timerA)
![msp_ble_tp](https://user-images.githubusercontent.com/81850188/204087067-5858af22-cb1d-48a6-9d30-08e837531a48.png)

Based on the line of best fit, **y = 2.0811x - 0.0014**:
  If y = 1 second, amount of data we can transmit (x) is
    (1 + 0.0014)/2.0811 = 0.4812
Therefore, the throughput is 0.4812kBps

4. Latency: MSP-HC05-UART Transmissions
// Relevant settings: 38400 baud, LSB first, no parity, one stop bit
- Command 'AT' was sent 5 times through UART
- Time taken for each attempt where the response is received is printed to the serial terminal
- Time taken refers to the time for the command to be sent to HC05 and respond back with OK (recorded via timerA)
![msp_ble_lt](https://user-images.githubusercontent.com/81850188/204087141-a72fe6ee-b856-4f35-aca1-67ffb20765a2.png)

Based on the graph,
  The average latency is 2.473ms and transfer speed could be slower or faster based on a variance of 0.5us.

5. Throughput: Pico-HC05-UART Transmissions
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

6. Latency: Pico-HC05-UART Transmissions
// Relevant settings: 38400 baud, LSB first, no parity, one stop bit
- Command 'AT' was sent 5 times through UART
- Time taken for each attempt where the response is received is printed to the serial terminal
- Time taken refers to the time for the command to be sent to HC05 and respond back with OK (recorded via Pico's time.h API)
![pico_ble_lt](https://user-images.githubusercontent.com/81850188/204079517-95f66838-d6bf-4854-be49-8771bc06a58d.png)

Based on the graph,
  The average latency is 14.0164ms and transfer speed could be slower or faster based on a variance of 1.88us.
