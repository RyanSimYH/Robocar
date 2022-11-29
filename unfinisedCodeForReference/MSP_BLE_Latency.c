/**
 * UART APIs from: https://github.com/amartinezacosta/MSP432-HC-05
 * MSPrintf, MSPGets, etc.
 *
*/

#include <Hardware/CS_Driver.h>
#include <Devices/MSPIO.h>
#include <string.h>
#include <driverlib.h>

#define TICKPERIOD      1000
#define BUFFER_SIZE    128

/*Data Buffer*/
char Buffer[BUFFER_SIZE];
int intTimes;
int timerValue;
volatile int i = 1;

void Initalise_Timer(void);
static unsigned long getTime(void);

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
eUSCI_UART_Config UART0Config =
{
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     13,
     0,
     37,
     EUSCI_A_UART_NO_PARITY,
     EUSCI_A_UART_LSB_FIRST,
     EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE,
     EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 9600 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
eUSCI_UART_Config UART2Config =
{
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     156,
     4,
     0,
     EUSCI_A_UART_NO_PARITY,
     EUSCI_A_UART_LSB_FIRST,
     EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE,
     EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};

void Initialise_Timer(void)
{
    /* Timer_A UpMode Configuration Parameter */
    const Timer_A_UpModeConfig upConfig =
    {
            TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_24,          // SMCLK/24 = 1MHz
            TICKPERIOD,                             // 1000 tick period = 1 millisec per tick
            TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
            TIMER_A_DO_CLEAR                        // Clear value
    };

    int a = CS_getSMCLK();

    /* Configuring Timer_A0 for Up Mode */
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig);

    /* Enabling interrupts */
    Interrupt_enableInterrupt(INT_TA0_0);
    Timer_A_clearTimer(TIMER_A0_BASE);
}

void main(void)
{
    MAP_WDT_A_holdTimer();
    Initialise_Timer();

    /*MSP432 running at 24 MHz*/
    CS_Init();

    /*Initialize Hardware required for the HC-05*/
    UART_Init(EUSCI_A0_BASE, UART0Config);
    UART_Init(EUSCI_A2_BASE, UART2Config);

    MAP_Interrupt_enableMaster();

    intTimes = 0;

    for (; i < 16;){
        char timeData[8];
        int j = 0;
        for(; j < 5000000; j++);

        Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
        /*Send data to HC-05*/
        MSPrintf(EUSCI_A2_BASE, "AT\r\n");

        MSPgets(EUSCI_A2_BASE, Buffer, BUFFER_SIZE);
        Timer_A_stopTimer(TIMER_A0_BASE);
        sprintf(timeData, "%d", getTime());

        /*Send data to serial terminal*/
        MSPrintf(EUSCI_A0_BASE, "Attempt %i, response received: %stime: %i.", i, Buffer, intTimes);
        MSPrintf(EUSCI_A0_BASE, timeData);
        MSPrintf(EUSCI_A0_BASE, "\r\n");
        intTimes = 0;
        i++;
    }
}

/* Returns time taken in milliseconds (int) */
static unsigned long getTime(void)
{
    unsigned long timetaken=0;
    timetaken = 0;
    /* Number of times the interrupt occurred (1 interrupt = 1000 ticks)    */

    /* Number of ticks (between 1 to 999) before the interrupt could occur */
    timetaken += Timer_A_getCounterValue(TIMER_A0_BASE);

    /* Clear Timer */
    Timer_A_clearTimer(TIMER_A0_BASE);

    return timetaken;
}

void TA0_0_IRQHandler(void)
{
    /* Increment global variable (count number of interrupt occurred) */
    intTimes++;

    /* Clear interrupt flag */
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

