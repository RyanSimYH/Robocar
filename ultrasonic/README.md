# Ultrasonic sensor

Sends out pulses to measure distance between device and object and displays readings

## Configuration
* 3 Ultrasonic connected to Pico (Left, Right, Centre)

### Centre
* Trigger GP17
* Echo GP16

### Left
* Trigger GP22
* Echo GP27

### Right
* Trigger GP20
* Echo GP21

## How to use
* Copy CMakeLists.txt, pico_sdk_import.cmake and main.c files into visual studio code
(Copy main_usingAverageAlgo.c OR main_usingMovingAverageAlgo.c instead of main.c to run the file with algorithm)
* Build project with GCC 10.3.1 arm compiler
* Flash uf2 file located in build folder into Pico
* Open serial to show readings
