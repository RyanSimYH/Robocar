

/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * Code 39 Barcode Reader
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
// #include "motor2/motor.h"

#define IN3 4
#define IN4 3
#define ENB 2
#define IN1 6
#define IN2 7
#define ENA 8
#define IR_GPIO 26
#define ENCODER_L 28
#define WHEELCIRCUMFERENCE 20.42035

struct repeating_timer timer01;

char turnCheck(bool set, char side);
void forward();
void reverse();
void stop();
int presetNotch(bool set, int notches);
int getNumNotch(bool add, char side, bool reset)
{
    static int numNotchL = 0; // Get the number of notches from the left wheel
    static int numNotchR = 0; // Get the number of notches from the right wheel
    int tempstoreL = 0;       // Temporarily store the number of notches for the left wheel (Will remove this var later)
    int tempstoreR = 0;

    if (side == 'L' && add == true) // If add is true and side is L
    {
        numNotchL++; // Increment the number of notches for the left wheel by 1
    }
    else if (side == 'R' && add == true) // If add is true and side is R
    {
        numNotchR++; // Increment the number of notches for the right wheel by 1
    }
    else if (side == 'L' && add == false) // If add is false and the side is L
    {

        if (reset) // If reset is true
        {
            tempstoreL = numNotchL;
            numNotchL = 0; // Reset the number of notches for the left wheel to 0
            return tempstoreL;
        }
        else
        {
            return numNotchL;
        }
    }
    else if (side == 'R' && add == false)
    {
        if (reset)
        {
            tempstoreR = numNotchR;
            numNotchR = 0; // Reset the number of notches for the right wheel to 0
            return tempstoreR;
        }
        else
        {
            return numNotchR;
        }
    }
}

bool pwmTimer(struct repeating_timer *t)
{
    static int numSample = 0;
    int pwmRDutyCycle = 2000;
    int pwmLDutyCycle = 0;

    int presetNumNotch = presetNotch(false, 0);
    int notchL = getNumNotch(false, 'L', true);
    int notchR = getNumNotch(false, 'R', true);
    printf("data: %d\n", notchL);

    int increment = 100;
    int dcL = 0;                   // Duty cycle of the left wheel
    int dcR = 0;                   // Duty cycle of the right wheel
    static bool rampspeed = false; // If ramp speed is true, then the duty cycle of both wheels will be set to 9000
    static float integralL = 1.0;  // The integral of the left wheel refers to the summation of all the differences in notches of the left wheel
    static float integralR = 1.0;  // The integral of the right wheel refers to the summation of all the differences in notches of the right wheel
    static int lasterrorL = 0;     // The variable to store the previous error (used for derivative calculation) for the left wheel
    static int lasterrorR = 0;     // The variable to store the previous error (used for derivative calculation) for the right wheel
    int Ki = 1;                    // Constant multiplier that is used to regulate the proportional gain of the car
    int Kd = 10;                   // Constant multiplier used to determine the response time

    float cmPerSecL = 0;
    float cmPerSecR = 0;
    float cmPerSecAvg = 0;
    float distanceTravelled = 0;
    int cmAvgWhole = 0;
    int cmAvgRemainder = 0;

    int distanceTravelledWhole = 0;
    int distanceTravelledRemainder = 0;

    int diffL = presetNumNotch - notchL; // The preset number of notches of left wheel - (Number of notches for the left wheel)
    int diffR = presetNumNotch - notchR; // The preset number of notches of right wheel - (Number of notches for the right wheel)

    cmPerSecL = (WHEELCIRCUMFERENCE / 20) * notchL;
    cmPerSecR = (WHEELCIRCUMFERENCE / 20) * notchR;

    // Proportional controller = (increment * diffL) = constant multiplied by error of wheel. used to make large changes to match speed
    // Integral Controller = (Ki * integralL) = constant multiplied by summation of past error. used to make small changes to correct the errors that persists
    // derivative controller = (Kd * (diffL - lasterrorL)) = constant multiplied by current difference - previous difference. used to prevent overshooting of the intended value.
    unsigned char buffer[33] = "";
    // uPrintf("notchL = ");
    // sprintf(buffer, "%d", notchL);
    // uPrintf(buffer);
    // uPrintf("\n\r");
    // uPrintf("notchR = ");
    // sprintf(buffer, "%d", notchR);
    // uPrintf(buffer);
    // uPrintf("\n\r");
    // uPrintf("DC L = ");
    // sprintf(buffer, "%d", pwmLConfig.dutyCycle);
    // uPrintf(buffer);
    // uPrintf("\n\r");
    // uPrintf("DC R = ");
    // sprintf(buffer, "%d", pwmRConfig.dutyCycle);
    // uPrintf(buffer);
    // uPrintf("\n\r");

    if (numSample > 2)
    {                   // Initial 2 seconds are not measured for calibration purposes
        if (diffL != 0) // If the difference between the preset notches - number of left notches is not 0
        {
            integralL += (diffL * 0.5); // Increment integralL by error difference multiplied by the timer interval unit

            dcL = (increment * diffL) + (Ki * integralL) + (Kd * (diffL - lasterrorL)); // Duty cycle = P + I + D
            // increment is constant p, the proportional gain, for every unit of error (diffL), how much you going to ramp up by (increment)
            // pwmLConfig.dutyCycle += dcL;
            pwmLDutyCycle += dcL;
            lasterrorL = diffL; // lasterrorL is to store the difference between the preset number of notches and the left notches for the previous timer interval
        }
        if (diffR != 0)
        {
            integralR += (diffR * 0.5); // Increment integralR by error difference multiplied by the timer interval unit

            dcR = (increment * diffR) + (Ki * integralR) + (Kd * (diffR - lasterrorR));
            pwmRDutyCycle += dcR;

            // pwmRConfig.dutyCycle += dcR;
            lasterrorR = diffR;
        }

        cmPerSecAvg = (cmPerSecL + cmPerSecR) * 2;

        distanceTravelled += cmPerSecAvg;

        //        uPrintf("Speed of the car = ");
        //        sprintf(buffer,"%f ", cmPerSecAvg);
        //        uPrintf(buffer);
        //        uPrintf("cm/s");
        //        uPrintf("\n\r");
        //
        //        uPrintf("Distance travelled by car = ");
        //        sprintf(buffer,"%f ", distanceTravelled);
        //        uPrintf(buffer);
        //        uPrintf("cm");
        //        uPrintf("\n\r");

        cmAvgWhole = cmPerSecAvg;
        cmAvgRemainder = (cmPerSecAvg - cmAvgWhole) * 1000;

        distanceTravelledWhole = distanceTravelled;
        distanceTravelledRemainder = (distanceTravelled - distanceTravelledWhole) * 1000;

        if ((diffL <= -3) && (diffR <= -3) && rampspeed == true)
        // To measure that the number of notches on the left and right wheel is more than the preset number of notches
        // To turn off rampspeed mode
        {
            // Set back to original duty cycle
            // pwmLConfig.dutyCycle = 4000;
            // pwmRConfig.dutyCycle = 4000;
            rampspeed = false;
        }

        // Poor man's method under the bridge
        //        if(diffL >= 1) //If the number of notches on the left wheel is greater than the preset number by more than or equals to 3
        //        {
        //            pwmLConfig.dutyCycle-=increment; //Decrease the duty cycle of the left wheel
        //
        //        }
        //        if(diffR >= 1) //If the number of notches on the right wheel is greater than the preset number by more than or equals to 1
        //        {
        //            pwmRConfig.dutyCycle-=increment; //Decrease the duty cycle of the right wheel
        //
        //        }
        //        if(diffL <= 1) //If the number of notches on the left wheel is lower than the preset number by less than or equals to 1
        //        {
        //            pwmLConfig.dutyCycle+=increment; //Increase the duty cycle of the left wheel
        //
        //        }
        //        if(diffR <= 1) //If the number of notches on the right wheel is lower than the preset number by less than or equals to 3
        //        {
        ////            if( (pwmLConfig.dutyCycle - (pwmRConfig.dutyCycle + increment) ) >= 250 )
        ////            {
        ////                pwmRConfig.dutyCycle += increment; //Increase the duty cycle of the right wheel
        ////                Timer_A_generatePWM(TIMER_A2_BASE, &pwmRConfig);
        ////            }
        //            pwmRConfig.dutyCycle+=increment; //Increase the duty cycle of the right wheel
        //
        //        }
        if (notchL == 0 && notchR == 0) // If the number of notches detected on the left and right wheel is 0
        {
            // pwmLConfig.dutyCycle = 9000; // Set duty cycle of left and right wheel to 9000 (ramp speed mode)
            // pwmRConfig.dutyCycle = 9000;
            rampspeed = true;
        }
        pwm_set_gpio_level(ENA, pwmLDutyCycle);

        // Timer_A_generatePWM(TIMER_A2_BASE, &pwmRConfig);
        // Timer_A_generatePWM(TIMER_A0_BASE, &pwmLConfig);
    }
    numSample++;
}

void encoder_left(uint gpio, uint32_t events)
{
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    static int rightNotch = 0;

    uint16_t result = 0;
    char data[100] = "";

    result = gpio_get(ENCODER_L);
    getNumNotch(true, 'L', false);
    if (turnCheck(false, 'A') == 'R')
    {
        rightNotch++;
        if (rightNotch == 18)
        {
            stop();
            turnCheck(true, 'A');
            rightNotch = 0;

            forward();
        }
    }
}

int main()
{
    stdio_init_all();

    printf("-------- Welcome to barcode --------\n");

    // Initialize GPIO
    gpio_init(IN3);
    gpio_init(IN4);
    gpio_init(IN1);
    gpio_init(IN2);
    gpio_init(ENCODER_L);

    // Set Direction for GPIO
    gpio_set_dir(IN3, GPIO_OUT);
    gpio_set_dir(IN4, GPIO_OUT);
    // gpio_set_dir(ENB, GPIO_OUT);
    gpio_set_dir(IN1, GPIO_OUT);
    gpio_set_dir(IN2, GPIO_OUT);
    // gpio_set_dir(ENB, GPIO_OUT);

    // Configure PWM slice and set it running
    const uint count_top = 10000;
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_wrap(&cfg, count_top);
    pwm_config_set_clkdiv(&cfg, 1000);
    pwm_init(pwm_gpio_to_slice_num(ENA), &cfg, true);
    pwm_init(pwm_gpio_to_slice_num(ENB), &cfg, true);

    // Note we aren't touching the other pin yet -- PWM pins are outputs by
    // default, but change to inputs once the divider mode is changed from
    // free-running. It's not wise to connect two outputs directly together!
    gpio_set_function(ENA, GPIO_FUNC_PWM);
    gpio_set_function(ENB, GPIO_FUNC_PWM);
    pwm_set_gpio_level(ENA, 2000);
    // pwm_set_gpio_level(ENB, 200);

    // Make sure GPIO is high-impedance, no pullups etc
    gpio_set_pulls(ENCODER_L, true, false);
    // Select ADC input 0 (GPIO26)
    gpio_set_dir(ENCODER_L, GPIO_IN);

    // Set Interrupt callback for GPIO26
    gpio_set_irq_enabled_with_callback(ENCODER_L, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_left);

    uint16_t result = 0;

    // set notch speed
    presetNotch(true, 7);

    add_repeating_timer_ms(500, pwmTimer, NULL, &timer01);

    forward();

    while (1)
    {
        /* code */
    }
}

void forward()
{
    gpio_put(IN1, 1);
    gpio_put(IN2, 0);
    gpio_put(IN3, 1);
    gpio_put(IN4, 0);
}
void reverse()
{
    gpio_put(IN1, 0);
    gpio_put(IN2, 1);
    gpio_put(IN3, 0);
    gpio_put(IN4, 1);
}

void stop()
{
    gpio_put(IN1, 0);
    gpio_put(IN2, 0);
    gpio_put(IN3, 0);
    gpio_put(IN4, 0);
}

int presetNotch(bool set, int notches)
{
    static int presetNumNotch = 0; // Set the number of notch to 0
    if (set)                       // If set is true
    {
        presetNumNotch = notches; // Set the preset number with the function argument 'notches'
    }
    else
    {
        return presetNumNotch; // If false, just return the value
    }
    return 0;
}

char turnCheck(bool set, char side)
{
    static char turnSide = 'A';
    if (set)
    {
        turnSide = side;
    }

    return turnSide;
}