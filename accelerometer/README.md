# Accelerometer sensor

Reads the X, Y and Z axis of the car and display readings on terminal. 

## Configuration
MPU6050 connected to Pico

###### MPU6050 --> Pico
VCC to 3V3(OUT)
GND to GND
SCL to GP19
SDA to GP18

## How to use
* Copy CMakeLists.txt, pico_sdk_import.cmake, MPU6050.h and main.c files into visual studio code
(Copy avgAlgo.c OR movingAvg.c instead of main.c to run the file with algorithm)
* Build project with GCC 10.3.1 arm compiler
* Flash MPU6050.uf2 file in build folder into Pico
* Open serial to show readings
