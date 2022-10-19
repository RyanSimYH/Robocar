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
 * MSP432 Timer_A - Variable PWM
 *
 * Description: In this example, the Timer_A module is used to create a precision
 * PWM with an adjustable duty cycle. The PWM initial period is 80 ms and is
 * output on P2.4. The initial duty cycle of the PWM is 10%, however when the
 * button is pressed on P1.1 the duty cycle is sequentially increased by 10%.
 * Once the duty cycle reaches 90%, the duty cycle is reset to 10% on the
 * following button press.
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

/* Timer_A PWM Configuration Parameter */
Timer_A_PWMConfig pwmLConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK, // SMCLK is 3MHz, ACLK is 16KHz
        TIMER_A_CLOCKSOURCE_DIVIDER_24, // 30000000 / 24 = 125000Hz
        10000, // 1/125000 = 0.000008 second per tick = 8 microsecond per tick
        // 0.000008 * 10000 = 0.08 second = 80 ms
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_RESET_SET,
        1000 // 10% duty cycle
};

Timer_A_PWMConfig pwmRConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_24,
        10000,
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_RESET_SET,
        1000
};


int main(void)
{
    /* Halting the watchdog */
    MAP_WDT_A_holdTimer();

    /* Configuring P4.4 and P4.5 as Output. P2.4 as peripheral output for PWM and P1.1 for button interrupt */
    //left
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN4); //Set PORT 4.4 and PORT 4.5 as output pins
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5); // PORT 4.5 Low = Forward, PORT 4.5 High = reverse
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4); // PORT 4.4 High = Forward, PORT 4.4 Low = reverse
    // If both PORT 4.4 and PORT 4.5 are set to low = the PWM motor will stop
    // Both PORT 4.4 and PORT 4.5 has to be set in the right settings for the PWM motor to trigger either forward or reverse
    // If one of the PORT is not set correctly, it will not run
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
    //To setup the motor driver board as output
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmLConfig); // Generate PWM left motor based on config

    //right
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0); //Set PORT 4.0 and PORT 4.2 to be output pins
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN2); // PORT 4.2 Low = Forward, PORT 4.2 High = Reverse
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0); // PORT 4.0 High = Forward, PORT 4.0 Low = Reverse
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
    Timer_A_generatePWM(TIMER_A2_BASE, &pwmRConfig);


    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1); // Set switch PORT 1.1 to be input PIN
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);

    /* Configuring Timer_A to have a period of approximately 80ms and an initial duty cycle of 10% of that (1000 ticks)  */

    /* Enabling interrupts and starting the watchdog timer */
    Interrupt_enableInterrupt(INT_PORT1);
    Interrupt_enableSleepOnIsrExit();
    Interrupt_enableMaster();

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

    if (status & GPIO_PIN1) // When Switch PIN 1.1 is pressed
    {
        if(pwmRConfig.dutyCycle == 7000) // If the duty cycle of right motor is 7000
            pwmRConfig.dutyCycle = 1000; // Set duty cycle to 1000
        else
            pwmRConfig.dutyCycle += 1000; // If the duty cycle is not 7000, increment it by 1000

        if(pwmLConfig.dutyCycle == 7000) // if the duty cycle of left motor is 7000
            pwmLConfig.dutyCycle = 1000; // Set duty cycle to 1000
        else
            pwmLConfig.dutyCycle += 1000; // If the duty cycle is not 7000, increment it by 1000

        Timer_A_generatePWM(TIMER_A0_BASE, &pwmLConfig); // Save the new config changes for left motor
        Timer_A_generatePWM(TIMER_A2_BASE, &pwmRConfig); // Save the new conig changes for the right motor


    }
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
}

void carRight()
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5); // PIN5 Low = Forward, PIN4 Low = reverse
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4);

    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN2);
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
