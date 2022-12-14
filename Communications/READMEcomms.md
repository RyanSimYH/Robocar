# Dashboard Setup
1. Download Node-RED (https://nodered.org/docs/getting-started/windows)
2. After downloading, go to command prompt and enter: node-red
3. In your browser, open localhost:1880
4. From github, clone Communications > Node-Red > dashboard_withMap.json
5. On Node-Red, import the .json file (menu icon on top right > import)
<img width="193" alt="image" src="https://user-images.githubusercontent.com/98366701/204533576-4d8465af-f216-4abc-b18c-04232e4e036e.png">
6. Click the Deploy button (top right, red button)
7. Open the dashboard tab, change the theme colours
 <img width="193" alt="image" src="https://user-images.githubusercontent.com/98366701/204532769-2289a953-cb5b-4c71-a761-82f712faf731.png">
8. Open localhost:1880/ui

# Setting up

## Configuration for MSP to M5StickCPlus

![image](https://user-images.githubusercontent.com/81850188/204540050-3f38715e-acb7-423a-abe4-e4a60e22a098.png)

> Head to https://flow.m5stack.com/ and connect the m5 and current device (eg. laptop) to the same network.
> Import the m5f file and run.


## Configuration for MSP to HC05

![image](https://user-images.githubusercontent.com/81850188/204540998-6eafab4a-1b63-44eb-80e6-e75d338e9a2d.png)

> Install tera terminal to pair and connect HC05 with master device (eg. laptop)
> 
> Data will also be displayed in it when data is sent from Pico


### Building Pico Code with Working Printf

> In CMakeLists.txt
>
> Change all `uart_advanced` to the name of the file


## Setting up configuration for Pico to HC05

![image](https://user-images.githubusercontent.com/81850188/204531729-273f0c6a-5aab-4321-bb59-8d77b4f3654a.png)


## Setting up configuration for Pico to M5StickC Plus

![image](https://user-images.githubusercontent.com/81850188/204536311-3b34b64c-e202-4b06-9c37-48116d022dbf.png)


# Throughput/Latency Tests

### MSP-UART Throughput
- Import the zipped folder's contents onto CCS
- Plug in the MSP through USB cable
- Open a serial terminal to the MSP's COM port (115200 baud)
- Flash the MSP

### MSP-BLE Throughput & Latency
- Import the MSP_BLE_Performance zipped folder's contents onto CCS
- Plug in the MSP through USB cable
- Open a serial terminal to the MSP's COM port (115200 baud)
- Open Tera Terminal and pair & connect to bluetooth port
- **For Latency Only** connect the EN pin to +5V to enable command mode
- Flash the MSP


# Flowchart
![communications-Page-2 drawio (1)](https://user-images.githubusercontent.com/94168656/204555288-5b3d692f-0be9-4ac0-bbdc-375b3ee842ce.png)
