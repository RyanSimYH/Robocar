/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/*******************************************************************************


 *
 *                MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST         P1.1  |<--Toggle Switch
 *            |                  |
 *            |                  |
 *            |            P2.4  |--> Output PWM
 *            |                  |
 *            |                  |
 *
 *******************************************************************************/
/* DriverLib Includes */
#include <driverlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define WHEELCIRCUMFERENCE 20.42035

/* Timer_A PWM Configuration Parameter */
Timer_A_PWMConfig pwmLConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_24,
        10000,
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_RESET_SET,
        2000
};

Timer_A_PWMConfig pwmRConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_24,
        10000,
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_RESET_SET,
        2000
};

const Timer_A_UpModeConfig pidTimer =
{
        TIMER_A_CLOCKSOURCE_ACLK, //32 KHz
        TIMER_A_CLOCKSOURCE_DIVIDER_16, //32000 / 16 = 2000
        2000, // (1/2000) * 1000 = 0.5 sec
        TIMER_A_TAIE_INTERRUPT_DISABLE,
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
        TIMER_A_DO_CLEAR
};

const eUSCI_UART_Config uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,                 // SMCLK Clock Source
        1,                                             // BRDIV = 78
        10,                                              // UCxBRF = 2
        0,                                              // UCxBRS = 0
        EUSCI_A_UART_ODD_PARITY,                        // ODD Parity
        EUSCI_A_UART_LSB_FIRST,                         // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,                      // One stop bit
        EUSCI_A_UART_MODE,                              // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,  // Oversampling
};

int getNumNotch(bool add ,char side, bool reset);
int presetNotch(bool set, int notches);
void uPrintf(unsigned char * TxArray);
void carStop();
void carLeft();
void carRight();
char turnCheck(bool set, char side);

int main(void)
{
    /* Halting the watchdog */
    MAP_WDT_A_holdTimer();

    /* Selecting P1.2 and P1.3 in UART mode */
   GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

   /* Configuring UART Module */
   UART_initModule(EUSCI_A0_BASE, &uartConfig);

   /* Enable UART module */
   UART_enableModule(EUSCI_A0_BASE);

   uPrintf("program start PID \n\r");

    /* Configuring P4.4 and P4.5 as Output. P2.4 as peripheral output for PWM and P1.1 for button interrupt */
    //left
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5); // PIN5 Low = Forward, PIN4 Low = reverse
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4); // PIN5 High = reverse, PIN4 High = Forward
    // If both PIN4 and PIN5 are set to low = the PWM motor will stop
    // Both PIN4 and PIN5 has to be set in the right settings for the PWM motor to trigger either forward or reverse
    // If one of the PIN is not set correctly, it will not run
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmLConfig);

    //right
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
    Timer_A_generatePWM(TIMER_A2_BASE, &pwmRConfig);


    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN4);

    /* Configuring Timer_A to have a period of approximately 80ms and an initial duty cycle of 10% of that (1000 ticks)  */

    // Setting up interrupt for Left Encoder (P3.3)
    GPIO_clearInterruptFlag(GPIO_PORT_P3, GPIO_PIN3);
    GPIO_enableInterrupt(GPIO_PORT_P3, GPIO_PIN3);
    Interrupt_enableInterrupt(INT_PORT3);
    // Setting up interrupt for Right Encoder (P2.5)
    GPIO_clearInterruptFlag(GPIO_PORT_P2, GPIO_PIN5);
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN5);
    Interrupt_enableInterrupt(INT_PORT2);

    // Timer Interrupt
    Timer_A_configureUpMode(TIMER_A1_BASE, &pidTimer);
    Interrupt_enableInterrupt(INT_TA1_0);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    /* Enabling interrupts and starting the watchdog timer */
    Interrupt_enableInterrupt(INT_PORT1);
    Interrupt_enableSleepOnIsrExit();
    Interrupt_enableMaster();

    //set notch speed
    presetNotch(true,10);



    /* Sleeping when not in use */
    while (1)
    {
        PCM_gotoLPM0();
    }
}

/* Port1 ISR - This ISR will progressively step up the duty cycle of the PWM on a button press */
void PORT1_IRQHandler(void)
{
    uint32_t status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    if (status & GPIO_PIN1)
    {
//        if(pwmRConfig.dutyCycle == 7000)
//            pwmRConfig.dutyCycle = 1000;
//        else
//            pwmRConfig.dutyCycle += 1000;
//
//        if(pwmLConfig.dutyCycle == 7000)
//            pwmLConfig.dutyCycle = 1000;
//        else
//            pwmLConfig.dutyCycle += 1000;
//
//        Timer_A_generatePWM(TIMER_A0_BASE, &pwmLConfig);
//        Timer_A_generatePWM(TIMER_A2_BASE, &pwmRConfig);
        Timer_A_stopTimer(TIMER_A1_BASE);
        carStop();
        unsigned char buffer[33] = "";
        uPrintf("notchL = ");
        sprintf(buffer,"%d",getNumNotch(false, 'L', true));
        uPrintf(buffer);
        uPrintf("\n\r");
        uPrintf("notchR = ");
        sprintf(buffer,"%d",getNumNotch(false, 'R', true));
        uPrintf(buffer);
        uPrintf("\n\r");


    }
    else if(status & GPIO_PIN4)
    {
        volatile uint32_t ii = 0;
        carStop();
        getNumNotch(false, 'L', true);
        getNumNotch(false, 'R', true);
        Timer_A_stopTimer(TIMER_A1_BASE);
        pwmLConfig.dutyCycle = 4000;
        pwmRConfig.dutyCycle = 4000;

        Timer_A_generatePWM(TIMER_A0_BASE, &pwmLConfig);
        Timer_A_generatePWM(TIMER_A2_BASE, &pwmRConfig);
        carRight();
    }
}



//ISR for when the encoder detected one notch
void PORT2_IRQHandler(void) //This function will be triggered if there is an interrupt occurred
{
    uint32_t statusR;
    static int rightNotch = 0;
    statusR = GPIO_getEnabledInterruptStatus(GPIO_PORT_P2); //to receive the interrupt status and stored in the status local variable
    getNumNotch(true,'R',false); //call getNumNotch() to increment the number of notch by 1 for the right wheel
    if(turnCheck(false,'A')=='R')
        {
            rightNotch++;
            if(rightNotch == 14)
            {
                carStop();
                turnCheck(true,'A');
                rightNotch = 0;
            }
        }
    GPIO_clearInterruptFlag(GPIO_PORT_P2, statusR); //Clear interrupt flag for Port 2
}

void PORT3_IRQHandler(void) //This function will be triggered if there is an interrupt occurred
{
    uint32_t statusL;
    static int leftNotch = 0;
    statusL = GPIO_getEnabledInterruptStatus(GPIO_PORT_P3); //to receive the interrupt status and stored in the status local variable
    getNumNotch(true,'L',false); //call getNumNotch() to increment the number of notch by 1 for the left wheel
    if(turnCheck(false,'A')=='L')
        {
            leftNotch++;
            if(leftNotch == 14)
            {
                carStop();
                turnCheck(true,'A');
                leftNotch=0;
            }
        }
    GPIO_clearInterruptFlag(GPIO_PORT_P3, statusL); //Clear interrupt flag for Port 3
}

void TA1_0_IRQHandler(void)
{
    static int numSample = 0;
    int presetNumNotch = presetNotch(false, 0);
    int notchL = getNumNotch(false, 'L', true);
    int notchR = getNumNotch(false, 'R', true);
    int increment = 500;
    int dcL = 0; //Duty cycle of the left wheel
    int dcR = 0; //Duty cycle of the right wheel
    static bool rampspeed = false; //If ramp speed is true, then the duty cycle of both wheels will be set to 9000
    static float integralL = 1.0; // The integral of the left wheel refers to the summation of all the differences in notches of the left wheel
    static float integralR = 1.0; // The integral of the right wheel refers to the summation of all the differences in notches of the right wheel
    static int lasterrorL = 0; //The variable to store the previous error (used for derivative calculation) for the left wheel
    static int lasterrorR = 0; //The variable to store the previous error (used for derivative calculation) for the right wheel
    int Ki = 10; // Constant multiplier that is used to regulate the proportional gain of the car
    int Kd = 1; // Constant multiplier used to determine the response time

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

    if (numSample > 2) { // Initial 2 seconds are not measured for calibration purposes
        if(diffL != 0) // If the difference between the preset notches - number of left notches is not 0
        {
            integralL += (diffL * 0.5); //Increment integralL by error difference multiplied by the timer interval unit

            dcL = (increment * diffL) + (Ki * integralL) + (Kd * (diffL - lasterrorL)); //Duty cycle = P + I + D
            // increment is constant p, the proportional gain, for every unit of error (diffL), how much you going to ramp up by (increment)
            pwmLConfig.dutyCycle = dcL;
            lasterrorL = diffL; //lasterrorL is to store the difference between the preset number of notches and the left notches for the previous timer interval

        }
        if(diffR != 0)
        {
            integralR += (diffR * 0.5); //Increment integralR by error difference multiplied by the timer interval unit

            dcR = (increment * diffR) + (Ki*integralR) + (Kd * (diffR - lasterrorR));

            pwmRConfig.dutyCycle = dcR;
            lasterrorR = diffR;

        }

        unsigned char buffer[33] = "";
        cmPerSecAvg = (cmPerSecL + cmPerSecR) * 2;

        distanceTravelled += cmPerSecAvg;

        uPrintf("Speed of the car = ");
        sprintf(buffer,"%f ", cmPerSecAvg);
        uPrintf(buffer);
        uPrintf("cm/s");
        uPrintf("\n\r");

        uPrintf("Distance travelled by car = ");
        sprintf(buffer,"%f ", distanceTravelled);
        uPrintf(buffer);
        uPrintf("cm");
        uPrintf("\n\r");

        cmAvgWhole = cmPerSecAvg;
        cmAvgRemainder = (cmPerSecAvg - cmAvgWhole) * 1000;

        distanceTravelledWhole = distanceTravelled;
        distanceTravelledRemainder = (distanceTravelled - distanceTravelledWhole) * 1000;

        if((diffL <= -3) && (diffR <= -3) && rampspeed == true)
            //To measure that the number of notches on the left and right wheel is more than the preset number of notches
            //To turn off rampspeed mode
        {
            //Set back to original duty cycle
            pwmLConfig.dutyCycle = 4000;
            pwmRConfig.dutyCycle = 4000;
            rampspeed = false;


        }

        //Poor man's method under the bridge
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
        if(notchL == 0 && notchR == 0) //If the number of notches detected on the left and right wheel is 0
        {
            pwmLConfig.dutyCycle = 9000; //Set duty cycle of left and right wheel to 9000 (ramp speed mode)
            pwmRConfig.dutyCycle = 9000;
            rampspeed = true;
        }
        Timer_A_generatePWM(TIMER_A2_BASE, &pwmRConfig);
        Timer_A_generatePWM(TIMER_A0_BASE, &pwmLConfig);
    }
    numSample++;

    sendData(speedAverageWhole, cmAvgWhole);
    sendData(speedAverageRemainder, cmAvgRemainder);

    sendData(distanceWhole, distanceTravelledWhole);
    sendData(distanceRemainder, distanceTravelledRemainder);

    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);

}


int getNumNotch(bool add, char side, bool reset)
{
    static int numNotchL = 0; //Get the number of notches from the left wheel
    static int numNotchR = 0; //Get the number of notches from the right wheel
    int tempstoreL = 0; //Temporarily store the number of notches for the left wheel (Will remove this var later)
    int tempstoreR = 0;

    if (side == 'L' && add == true) //If add is true and side is L
    {
        numNotchL++; //Increment the number of notches for the left wheel by 1
    }
    else if (side == 'R' && add == true) //If add is true and side is R
    {
        numNotchR++; //Increment the number of notches for the right wheel by 1
    }
    else if (side == 'L' && add == false) //If add is false and the side is L
    {

        if(reset) //If reset is true
        {
            tempstoreL = numNotchL;
            numNotchL = 0; //Reset the number of notches for the left wheel to 0
            return tempstoreL;
        }
        else
        {
            return numNotchL;
        }


    }
    else if (side == 'R' && add == false)
    {
        if(reset)
        {
            tempstoreR = numNotchR;
            numNotchR = 0; //Reset the number of notches for the right wheel to 0
            return tempstoreR;
        }
        else
        {
            return numNotchR;
        }
    }



    return 0;
}

int presetNotch(bool set, int notches)
{
    static int presetNumNotch = 0; //Set the number of notch to 0
    if(set) //If set is true
    {
        presetNumNotch = notches; //Set the preset number with the function argument 'notches'
    }
    else
    {
        return presetNumNotch; //If false, just return the value
    }
    return 0;
}

void carForward()
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5); // PIN5 Low = Forward, PIN4 Low = reverse
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4);

    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);
}

void carLeft()
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN4); // PIN5 Low = Forward, PIN4 Low = reverse
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);

    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);
    turnCheck(true,'L');
}

void carRight()
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5); // PIN5 Low = Forward, PIN4 Low = reverse
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4);

    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN2);
    turnCheck(true,'R');
}

void carBackwards()
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN4); // PIN5 Low = Forward, PIN4 Low = reverse
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);

    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN2);
}
void carStop()
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5); // PIN5 Low = Forward, PIN4 Low = reverse
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN4);

    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0); // PIN5 Low = Forward, PIN4 Low = reverse
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN2);
}

void uPrintf(unsigned char * TxArray)
{
    unsigned short i = 0;
    while(*(TxArray+i))
    {
        UART_transmitData(EUSCI_A0_BASE, *(TxArray+i));  // Write the character at the location specified by pointer
        i++;                                             // Increment pointer to point to the next character
    }
}

//Function to check if the vehicle is turning since it can only turn in one direction at one time
char turnCheck(bool set, char side)
{
    static char turnSide = 'A';
    if(set)
    {
        turnSide = side;
    }

    return turnSide;

}
