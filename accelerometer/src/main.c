#include<stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "MPU6050.h"

//codes referenced from https://github.com/viduraembedded/Raspberry-Pi-Pico/tree/master/MPU6050

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

    int16_t accelerometer[3];
    MPU6050_Reset();
    while(1)
    {
        MPU6050_ReadData(accelerometer);
        printf("Accelerometer   X_OUT= %f   Y_OUT= %f   Z_OUT= %f\r\n", accelerometer[0], accelerometer[1], accelerometer[2] );
        //printf("%d  %d  %d\r\n", accelerometer[0], accelerometer[1], accelerometer[2] );
        sleep_ms(500);
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//average algorithm
// #include<stdio.h>
// #include "pico/stdlib.h"
// #include "pico/time.h"
// #include "hardware/gpio.h"
// #include "hardware/i2c.h"
// #include "pico/binary_info.h"
// #include "MPU6050.h"

// #define SCL 19
// #define SDA 18

// int main()
// {

//     //intialize stdio
//     stdio_init_all();
//     //intialize i2c1
//     i2c_init(I2C_PORT, 100*1000);
//     gpio_set_function(SCL, GPIO_FUNC_I2C);
//     gpio_set_function(SDA, GPIO_FUNC_I2C);

//     // need to enable the pullups
//     gpio_pull_up(SCL);
//     gpio_pull_up(SDA);

//     int count =0;
//     float sumX=0;
//     float sumY=0;
//     float sumZ=0;
//     float averageX=0;
//     float averageY=0;
//     float averageZ=0;

//     float sumXgyro=0;
//     float sumYgyro=0;
//     float sumZgyro=0;
//     float averageXgyro=0;
//     float averageYgyro=0;
//     float averageZgyro=0;

//     int16_t accelerometer[3], gyro[3];
//     MPU6050_Reset();
//     while(1)
//     {
//         MPU6050_ReadData(accelerometer);
//         sumX+=accelerometer[0];
//         sumY+=accelerometer[1];
//         sumZ+=accelerometer[2];

//         // sumXgyro+=gyro[0];
//         // sumYgyro+=gyro[1];
//         // sumZgyro+=gyro[2];

//         count++;
//         if(count==10){
//             averageX=sumX/10;
//             averageY=sumY/10;
//             averageZ=sumZ/10;

//             // averageXgyro=sumXgyro/10;
//             // averageYgyro=sumYgyro/10;
//             // averageZgyro=sumZgyro/10;

//             //Reset all the values
//             count =0;
//             sumX=0;
//             sumY=0;
//             sumZ=0;
//             // sumXgyro=0;
//             // sumYgyro=0;
//             // sumZgyro=0;
//         }
//         printf("Accelerometer   X_OUT= %f   Y_OUT= %f   Z_OUT= %f\r\n",averageX, averageY, averageZ);
//         //printf("Gyro   X_OUT= %f   Y_OUT= %f   Z_OUT= %f\r\n", averageXgyro, averageYgyro, averageZgyro);
//         sleep_ms(500);
//     }

//     return 0;
// }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//moving average algorithm

// #include<stdio.h>
// #include "pico/stdlib.h"
// #include "pico/time.h"
// #include "hardware/gpio.h"
// #include "hardware/i2c.h"
// #include "pico/binary_info.h"
// #include "MPU6050.h"

// #define SCL 19
// #define SDA 18

// float movingAvg(int *ptrArrNumbers, float *ptrSum, int pos, int len, int nextNum)
// {
//   //Subtract the oldest number from the prev sum, add the new number
//   *ptrSum = *ptrSum - ptrArrNumbers[pos] + nextNum;
//   //Assign the nextNum to the position in the array
//   ptrArrNumbers[pos] = nextNum;
//   //return the average
//   return *ptrSum / len;
// }

// int main()
// {

//     //intialize stdio
//     stdio_init_all();
//     //intialize i2c1
//     i2c_init(I2C_PORT, 100*1000);
//     gpio_set_function(SCL, GPIO_FUNC_I2C);
//     gpio_set_function(SDA, GPIO_FUNC_I2C);

//     // need to enable the pullups
//     gpio_pull_up(SCL);
//     gpio_pull_up(SDA);

//     int16_t accelerometer[3];    
    

//     float sampleX[10];// collect sample
//     float sampleY[10];
//     float sampleZ[10];
//     int counterForSamples=0;
//     float sumX=0;
//     float sumY=0;
//     float sumZ=0;
//     float avgX =0;
//     float avgY =0;
//     float avgZ =0;
//     MPU6050_Reset();
//     while(1)
//     {
//         MPU6050_ReadData(accelerometer);
//         sampleX[counterForSamples]=accelerometer[0];
//         sampleY[counterForSamples]=accelerometer[1];
//         sampleZ[counterForSamples]=accelerometer[2];
//         for (int i=0;i<=counterForSamples;i++){
//             sumX+=sampleX[i];
//             sumY+=sampleY[i];
//             sumZ+=sampleZ[i];
//         }
        
//         counterForSamples+=1;
//         avgX=sumX/counterForSamples;
//         avgY=sumY/counterForSamples;
//         avgZ=sumZ/counterForSamples;
//         if (counterForSamples ==10){
//             counterForSamples=0;
//         }
//         sumX=0;
//         sumY=0;
//         sumZ=0;
//         printf("Accelerometer   X_OUT= %f   Y_OUT= %f   Z_OUT= %f\r\n",avgX, avgY, avgZ);
//         sleep_ms(500);
//     }

    
//     return 0;
// }