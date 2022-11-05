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
        7000
};

Timer_A_PWMConfig pwmRConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_24,
        10000,
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_RESET_SET,
        7000
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

   uPrintf("program start \n\r");

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
            if(rightNotch == 12)
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
            if(leftNotch == 12)
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
    float perL = 1;
    float perR = 1;
    float dcL;
    float dcR;
    static bool rampspeed = false;

    float cmPerSecL = 0;
    float cmPerSecR = 0;

    int diffL = notchL- presetNumNotch; //(Number of notches for the left wheel) - the preset number
    int diffR = notchR - presetNumNotch; //(Number of notches for the right wheel) - the preset number
    if(notchL == 0)
    {
        perL = presetNumNotch/1;
    }
    else
    {
        perL = presetNumNotch / notchL;
    }
    if(notchR == 0)
    {
        perR = presetNumNotch / 1;
    }
    else
    {
        perR = presetNumNotch / notchR;
    }

    unsigned char buffer[33] = "";
    uPrintf("notchL = ");
    sprintf(buffer,"%d",notchL);
    uPrintf(buffer);
    uPrintf("\n\r");
    uPrintf("notchR = ");
    sprintf(buffer,"%d",notchR);
    uPrintf(buffer);
    uPrintf("\n\r");
    uPrintf("DC L = ");
    sprintf(buffer,"%d",pwmLConfig.dutyCycle);
    uPrintf(buffer);
    uPrintf("\n\r");
    uPrintf("DC R = ");
    sprintf(buffer,"%d",pwmRConfig.dutyCycle);
    uPrintf(buffer);
    uPrintf("\n\r");
    uPrintf("diffL = ");
    sprintf(buffer,"%d",diffL);
    uPrintf(buffer);
    uPrintf("\n\r");
    uPrintf("diffR = ");
    sprintf(buffer,"%d",diffR);
    uPrintf(buffer);
    uPrintf("\n\r");
    uPrintf("perL = ");
    sprintf(buffer,"%f",perL);
    uPrintf(buffer);
    uPrintf("\n\r");
    uPrintf("perR = ");
    sprintf(buffer,"%f",perR);
    uPrintf(buffer);
    uPrintf("\n\r");

    cmPerSecL = (WHEELCIRCUMFERENCE / 20) * notchL;
    cmPerSecR = (WHEELCIRCUMFERENCE / 20) * notchR;

    uPrintf("Speed of left wheel = ");
    sprintf(buffer,"%f ",cmPerSecL);
    uPrintf(buffer);
    uPrintf("cm/s");
    uPrintf("\n\r");

    uPrintf("Speed of right wheel = ");
    sprintf(buffer,"%f ",cmPerSecR);
    uPrintf(buffer);
    uPrintf("cm/s");
    uPrintf("\n\r");

    if (numSample >2) {
//        if(diffL)
//        {
//            dcL = pwmLConfig.dutyCycle;
//            dcL*=perL;
//            if(dcL < 2000)
//            {
//                dcL = 2000;
//            }
//            pwmLConfig.dutyCycle = dcL;
//            Timer_A_generatePWM(TIMER_A0_BASE, &pwmLConfig);
//
//        }
//        if(diffR)
//        {
//            dcR = pwmRConfig.dutyCycle;
//            dcR*=perR;
//            if(dcR < 2000)
//            {
//                dcR = 2000;
//            }
//            pwmRConfig.dutyCycle = dcR;
//            Timer_A_generatePWM(TIMER_A2_BASE, &pwmRConfig);
//
//        }
        if((diffL>=5) && (diffR >=5) && rampspeed == true)
        {
            pwmLConfig.dutyCycle = 4000;
            pwmRConfig.dutyCycle = 4000;
            rampspeed = false;


        }
        if(diffL >= 1) //If the number of notches on the left wheel is greater than the preset number by more than or equals to 3
        {
            pwmLConfig.dutyCycle-=increment; //Decrease the duty cycle of the left wheel

        }
        if(diffR >= 1) //If the number of notches on the right wheel is greater than the preset number by more than or equals to 1
        {
            pwmRConfig.dutyCycle-=increment; //Decrease the duty cycle of the right wheel

        }
        if(diffL <= 1) //If the number of notches on the left wheel is lower than the preset number by less than or equals to 1
        {
            pwmLConfig.dutyCycle+=increment; //Increase the duty cycle of the left wheel

        }
        if(diffR <= 1) //If the number of notches on the right wheel is lower than the preset number by less than or equals to 3
        {
//            if( (pwmLConfig.dutyCycle - (pwmRConfig.dutyCycle + increment) ) >= 250 )
//            {
//                pwmRConfig.dutyCycle += increment; //Increase the duty cycle of the right wheel
//                Timer_A_generatePWM(TIMER_A2_BASE, &pwmRConfig);
//            }
            pwmRConfig.dutyCycle+=increment; //Increase the duty cycle of the right wheel

        }
        if(notchL == 0 && notchR == 0)
        {
            pwmLConfig.dutyCycle = 9000;
            pwmRConfig.dutyCycle = 9000;
            rampspeed = true;
        }
        Timer_A_generatePWM(TIMER_A2_BASE, &pwmRConfig);
        Timer_A_generatePWM(TIMER_A0_BASE, &pwmLConfig);
    }
    numSample++;

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
