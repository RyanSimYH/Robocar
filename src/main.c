#include<stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "MPU6050.h"

//codes referenced from https://github.com/viduraembedded/Raspberry-Pi-Pico/tree/master/MPU6050

//define the connected pins
#define SCL 19
#define SDA 18

int main()
{

    //intialize stdio
    stdio_init_all();
    //intialize i2c1
    i2c_init(I2C_PORT, 100*1000);
    //set i2c for the pins
    gpio_set_function(SCL, GPIO_FUNC_I2C);
    gpio_set_function(SDA, GPIO_FUNC_I2C);

    // need to enable the pullups
    gpio_pull_up(SCL);
    gpio_pull_up(SDA);

    int16_t accelerometer[3]; //create array to store accelerometer readings: x, y, z
    int16_t initial[3];
    MPU6050_Reset();    //reset accelerometer before reading the data
    while(1)
    {
        MPU6050_ReadData(accelerometer); //call the read data function to read the data of the accelerometer constantly

        printf("Accelerometer   X_OUT= %f   Y_OUT= %f   Z_OUT= %f\r\n", accelerometer[0], accelerometer[1], accelerometer[2] ); //print the readings on serial
        sleep_ms(500); //delay 0.5 seconds before next reading
    }

    return 0;
}
