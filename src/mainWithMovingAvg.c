#include<stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "MPU6050.h"

//define connected pins
#define SCL 19
#define SDA 18

int main()
{

    //intialize stdio
    stdio_init_all();
    //intialize i2c1
    i2c_init(I2C_PORT, 100*1000);
    gpio_set_function(SCL, GPIO_FUNC_I2C);
    gpio_set_function(SDA, GPIO_FUNC_I2C);

    // need to enable the pullups
    gpio_pull_up(SCL);
    gpio_pull_up(SDA);

    int16_t accelerometer[3];  //array to store accelerometer values 
    
    //collect sample of each axis
    float sampleX[10];
    float sampleY[10];
    float sampleZ[10];
    int counterForSamples=0;
    
    //sum and average variables for each axis
    float sumX=0;
    float sumY=0;
    float sumZ=0;
    float avgX =0;
    float avgY =0;
    float avgZ =0;
  
    MPU6050_Reset(); //reset before reading
    while(1)
    {
        MPU6050_ReadData(accelerometer); //read data
        //put each axis reading in a respective queue
        sampleX[counterForSamples]=accelerometer[0];
        sampleY[counterForSamples]=accelerometer[1];
        sampleZ[counterForSamples]=accelerometer[2];
        for (int i=0;i<=counterForSamples;i++){
            //get the sum of the values in each axis queue
            sumX+=sampleX[i];
            sumY+=sampleY[i];
            sumZ+=sampleZ[i];
        }
        
        counterForSamples+=1; //increment counter
        //divide the sum with the length of the queue to get the average of each axis
        avgX=sumX/counterForSamples;
        avgY=sumY/counterForSamples;
        avgZ=sumZ/counterForSamples;
      
        //once queue reaches 10, overwrite the queue with new data from the beginning
        if (counterForSamples ==10){
            counterForSamples=0; //reset counter
        }
        
        //reset variables
        sumX=0;
        sumY=0;
        sumZ=0;
        printf("Accelerometer   X_OUT= %f   Y_OUT= %f   Z_OUT= %f\r\n",avgX, avgY, avgZ); //print out the moving average
        sleep_ms(500);
    }

    
    return 0;
}
