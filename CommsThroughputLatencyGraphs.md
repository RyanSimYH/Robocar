1. Throughput: MSP-UART Transmissions
// Relevant settings: 12MHz, 115200 baud, LSB first, no parity, LSB first, one stop bit
- We sent char arrays of various lengths (2k, 4k, 6k, 8k, 10k) through UART
- Time taken refers to the time for the UART transfer function to be completed (recorded via timerA)
![image](https://user-images.githubusercontent.com/98366701/204077876-6a4bacbe-4b33-4f8b-97d0-53cf2e118e9d.png)
![image](https://user-images.githubusercontent.com/98366701/204077869-77825e93-bb2d-47c6-92d2-1cfe3ff21543.png)

Based on the line of best fit, **y = 0.1731x - 0.0001**:
  If y = 1 second, amount of data we can transmit (x) is
    (y + 0.0001)/0.1731 = 1.0001/0.1731 = 5.778
Therefore, the throughput of MSP-UART transmissions, based on our configurations, is 5.778kBps
