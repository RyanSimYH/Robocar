#include<stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "MPU6050.h"

//Reset the accelerometer
void MPU6050_Reset()
{
    uint8_t reg[] = {0x6B, 0x00}; //Register hex 6B and bit 00
    i2c_write_blocking(I2C_PORT, MPU6050_ADDRESS, reg, sizeof(reg), false ); //Send the variable reg[] to reset
}

//Get readings for the accelerometer
void MPU6050_ReadData(int16_t accelerometer[3])
{
    uint8_t buffer[6]; //Store readings
    // reading the accelerometer data
    uint8_t reg = 0x3B; //Register for accelerometer reading
    i2c_write_blocking(I2C_PORT, MPU6050_ADDRESS, &reg, sizeof(reg), false ); //Pass the register to MPU6050
    i2c_read_blocking(I2C_PORT, MPU6050_ADDRESS, buffer, sizeof(buffer), false ); //Read the data from MPU6050

    //x, y, z values
    accelerometer[0] = (buffer[0] << 8) | buffer[1]; //x axis
    accelerometer[1] = (buffer[2] << 8) | buffer[3]; //y axis
    accelerometer[2] = (buffer[4] << 8) | buffer[5]; //z axis

    //Gyro data    
    // reg = 0x43; //Register for gyro
    // i2c_write_blocking(I2C_PORT, MPU6050_ADDRESS, &reg, sizeof(reg), false ); //Pass the register to MPU6050
    // i2c_read_blocking(I2C_PORT, MPU6050_ADDRESS, buffer, sizeof(buffer), false ); //Read the data from MPU6050

    // gyro[0] = (buffer[0] << 8) | buffer[1];
    // gyro[1] = (buffer[2] << 8) | buffer[3];
    // gyro[2] = (buffer[4] << 8) | buffer[5];

    
}