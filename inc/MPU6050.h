#include<stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c1 //define i2c port

#define MPU6050_ADDRESS 0x68 //define the address of accelerometer

void MPU6050_Reset(); //reset function
void MPU6050_ReadData(int16_t accelerometer[3]); //read data function
