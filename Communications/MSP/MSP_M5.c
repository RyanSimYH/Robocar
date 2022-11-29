/**
 * MSP to M5 Code (Working)
 * */

#include <driverlib.h>
#include <ESP8266.h>
#include <UART_Driver.h>
#include "stdio.h"
#include "string.h"

volatile int adcValue;
// char ESP8266_Buffer[ESP8266_BUFFER_SIZE];

void Initalise_Timer(void);
//static unsigned long getTime(void);

// Function prototypes
void getIntDatapoints(void);
void comms(void);
int barcodeAvailable(void);

// const int NUMBER_OF_DATAPOINTS = 6; // max number of data points expected in one loop around the submodules
unsigned int previousIntegerData[5];
unsigned int currentIntegerData[5];
static char *datapointNames[6]={"speed","turning","distanceWhole","distanceDecimal", "humpHeight","barcode"};
#define SPEED = 0;
#define TURNING = 1;
#define DISTANCE_WHOLE = 2;
#define DISTANCE_DECIMAL = 3;
#define HUMP_HEIGHT = 4;
#define BARCODE = 5;
// uint8_t navEndpoint[3];
const int maxDatapoints = 7; // including 1 for navigation

char fullString[255] = "{";

int intTimes;
int timerValue;
int lastCalledAt=0;
#define TICKPERIOD      1000

void Initialise_Timer(void)
{
    /* Timer_A UpMode Configuration Parameter */
    const Timer_A_UpModeConfig upConfig =
    {
            TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_12,          // SMCLK/12 = 1MHz
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

/* struct for MSP's serial */
eUSCI_UART_Config UART0Config =
{
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     6,
     8,
     0,
     EUSCI_A_UART_NO_PARITY,
     EUSCI_A_UART_LSB_FIRST,
     EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE,
     EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};

/* Struct for M5's serial */
eUSCI_UART_Config UART2Config =
{
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     6,
     8,
     0,
     EUSCI_A_UART_NO_PARITY,
     EUSCI_A_UART_LSB_FIRST,
     EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE,
     EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};

void main()
{
    MAP_WDT_A_holdTimer();
    Initialise_Timer();

    /*Ensure MSP432 is Running at 12 MHz*/
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);
    PCM_setCoreVoltageLevel(PCM_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    /*Initialize required hardware peripherals for the MSP's Serial Port*/
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_UART_initModule(EUSCI_A0_BASE, &UART0Config);
    MAP_UART_enableModule(EUSCI_A0_BASE);
    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);

    /*Initialize required hardware peripherals for the M5's Serial Port*/
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_UART_initModule(EUSCI_A2_BASE, &UART2Config);
    MAP_UART_enableModule(EUSCI_A2_BASE);
    MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA2);

    MAP_Interrupt_enableMaster();

    UART_Printf(EUSCI_A0_BASE, "\n\n****************Communications Start***************\r\n");
    /*Start ESP8266 serial terminal, will not return*/
    int j = 0;
    int i = 0;
    intTimes = 0;
    MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);



    while(1)
    {
        comms();
    }
}

void comms(void){

    while(intTimes-lastCalledAt < 1700);
    if(intTimes>100000){
        intTimes = 0;
    }
    lastCalledAt=intTimes;
    int datapoints = 0; // number of datapoints in the message to be sent
    int i=0; //for-loop counter
    if(barcodeAvailable() == 1){
       strcat(fullString, "\"");
       strcat(fullString, datapointNames[5]);
       strcat(fullString, "\": \"");
       // getBarcode()); //barcode is string, once getBarcode is called, barcode side should reset its barcodeAvail to not avail
       strcat(fullString, "barcode");
       strcat(fullString, "\"");
       datapoints++;
   }

    getIntDatapoints();

    for(i=0; i<5; i++){
    // compare the integer datapoints with their previous values, send only those that have changed
    // if(currentIntegerData[i] != previousIntegerData[i]){
        // structure the message
        if(datapoints != 0){
            strcat(fullString, ", ");
        }
        strcat(fullString, "\"");
        strcat(fullString, datapointNames[i]);
        strcat(fullString, "\": \"");
        char tempData[8];
        sprintf(tempData, "%d", currentIntegerData[i]);
        strcat(fullString, tempData);
        strcat(fullString, "\"");

        // store current value as prev value for the next round
        previousIntegerData[i] = currentIntegerData[i];
        datapoints++; // increment number of datapoints within the message
    // }
    }

    if(datapoints > 0){
        strcat(fullString, "}"); //finish packaging the message since datapoints have been gathered
        UART_Printf(EUSCI_A2_BASE, fullString);
        UART_Printf(EUSCI_A0_BASE, fullString);

        /* clear the message */
        memset(fullString, '\0', strlen(fullString));
        strcat(fullString, "{");
    }
}


void getIntDatapoints(void){
    currentIntegerData[0] = 1; // getSpeed();
    currentIntegerData[1] = 1; // getTurning();
    currentIntegerData[2] = 1; // getDistWhole();
    currentIntegerData[3] = 1; // getDistDecimal();
    currentIntegerData[4] = 1; // getHump_Height();
}

int barcodeAvailable(void){
    return 1;
}

void TA0_0_IRQHandler(void)
{
    /* Increment global variable (count number of interrupt occurred) */
    intTimes++;

    /* Clear interrupt flag */
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
}
