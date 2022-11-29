#include<stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "MPU6050.h"

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

    //variables to store sum and average of each axis
    int count =0;
    float sumX=0;
    float sumY=0;
    float sumZ=0;
    float averageX=0;
    float averageY=0;
    float averageZ=0;

    int16_t accelerometer[3]; //array to store accelerometer values
    MPU6050_Reset(); //reset accelerometer before reading data 
    while(1)
    {
        MPU6050_ReadData(accelerometer); //call read data function to get readings
        //add up all the axis values in their respective variable to get the sum of each axis
        sumX+=accelerometer[0];
        sumY+=accelerometer[1];
        sumZ+=accelerometer[2];


        count++; //increment count
        //add up 10 readings of each axis
        if(count==10){ //when count reaches 10
            //divide the sum of each axis by 10 to get average
            averageX=sumX/10;
            averageY=sumY/10;
            averageZ=sumZ/10;

            //Reset all the values
            count =0;
            sumX=0;
            sumY=0;
            sumZ=0;

        }
        printf("Accelerometer   X_OUT= %f   Y_OUT= %f   Z_OUT= %f\r\n",averageX, averageY, averageZ); //print the average of 10 readings
        sleep_ms(500); //delay before new readings
    }

    return 0;
}
