

/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * Code 39 Barcode Reader
 * Team-B2
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define K_CONST 0.5
#define IR_GPIO 26
#define SAMPLE_COUNT 5
#define BLACK 1
#define WHITE 0
// #define THRESHOLD 0.031
#define THRESHOLD 0.0448
// Lookup table for the code39

#define UART_ID uart1
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_ODD

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_RX_PIN 5
#define UART_TX_PIN 4

// Define GPIO pins for the ultrasonic trigger and echo pins for all three sensors
#define C_TRIGGER_PIN 17
#define C_ECHO_PIN 16
#define L_TRIGGER_PIN 22
#define L_ECHO_PIN 27
#define R_TRIGGER_PIN 20
#define R_ECHO_PIN 21

// Barcode Call Start
static const uint32_t g_barcode_lookup_table[44] = {
    111221211, 211211112, 112211112, 212211111, 111221112,
    211221111, 112221111, 111211212, 211211211, 112211211,
    211112112, 112112112, 212112111, 111122112, 211122111,
    112122111, 111112212, 211112211, 112112211, 111122211,
    211111122, 112111122, 212111121, 111121122, 211121121,
    112121121, 111111222, 211111221, 112111221, 111121221,
    221111112, 122111112, 222111111, 121121112, 221121111,
    122121111, 121111212, 221111211, 122111211, 121212111,
    121211121, 121112121, 111212121, 121121211};

// Match to Lookup Table for Code39 for DECODED RESULT
static const char g_barcode_lookup_match[44] = {
    '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z', '-', '.', ' ', '$',
    '/', '+', '%', '*'};

// Function used to decode binary
void decodeBarcode(uint16_t barcode_index, uint16_t barcode_arr[30]);

// Used to find Maxmimum value detected for narrow bar
uint16_t findMax(uint16_t secondData, uint16_t thirdData, uint16_t fourthData, uint16_t fifthData, uint16_t sixthData);

/**
 * Digital Filter
 * Using a specific algorithm to calculate
 * By combining 2 different digital filtering
 */
bool repeating_timer_callback(struct repeating_timer *t)
{
    static uint8_t sample_index = 0;   // Indexing for Sampling
    static int16_t barcode_index = -1; // Barcode not detected yet.
    static uint8_t previous_bin = 1;   // 0 for white 1 for black, Starts off black
    static uint16_t barcode_array[30]; // Array to store the data of barcode
    static float average_data = 0;     // Average Data of Sample

    if (barcode_index < 20)
    {
        // PICO 12 bit ADC - 4096 taking for the adc voltage used 3.3v
        const float conversion_factor = 3.3f / 4096;
        // Read ADC Value
        uint16_t result = adc_read();

        // Calculate Voltage
        float voltage = result * conversion_factor;

        // Check if index of sample is sample count
        if (sample_index < SAMPLE_COUNT)
        {
            // Algorithm to first Filter with equations
            average_data = K_CONST * voltage + (1.0 - K_CONST) * average_data;

            sample_index++;
        }
        else
        {

            // Get average data from sample for digital filtering 2.
            average_data /= SAMPLE_COUNT;
            // printf("Data %f\n", average_data);
            // Note : modify the value to fit the environment
            if (average_data > THRESHOLD)
            {
                // Black Detected

                if (previous_bin == WHITE)
                {
                    // printf("Bar detected : %d \n", barcode_array[barcode_index - 1]);
                    barcode_index++;
                    previous_bin = BLACK;
                }
            }
            else
            {
                // White Detected

                // To determine if barcode has started scanning.
                if (barcode_index == -1)
                {
                    barcode_index++;
                    previous_bin = WHITE;
                }
                else
                {
                    // Check if previo
                    if (previous_bin == BLACK)
                    {

                        printf("Bar detected : %d \n", barcode_array[barcode_index - 1]);

                        barcode_index++;
                        previous_bin = WHITE;
                    }
                }
            }

            if (barcode_index != -1)
            {
                barcode_array[barcode_index] += 1;
            }

            // Reset Average Data and Sample Index
            average_data = 0;

            sample_index = 0;
        }
    }
    else
    {
        // Decode barcode
        printf("Decoding barcode in progress ...\n");
        decodeBarcode(barcode_index, barcode_array);

        printf("Clearing Barcode Memory...\n");

        memset(barcode_array, 0, sizeof barcode_array);

        barcode_index = -1;
    }
    return true;
}
// Barcode Call END

// Mapping & Navigation
#define maxsize 4
#define maxarraysize 25
#define NORTHCHECK 7
#define EASTCHECK 11
#define SOUTHCHECK 13
#define WESTCHECK 14
#define ACCESSIBLE 15

#define NORTHTOGGLE 8
#define EASTTOGGLE 4
#define SOUTHTOGGLE 2
#define WESTTOGGLE 1

struct Point
{
    short x;
    short y;
};

struct node
{
    struct Point point;
    struct node *next;
};

struct GraphNode
{
    short checked;    // if -1,not checked, if 1, checked, if 3, check for weight to point in A star
    short directions; // directions in binary in NESW eg 1111 1 = open, 0 = closed off
    short routes;     // verify routes that are open and close
};

struct QueueItem
{
    struct Point nodeName;
    struct Point startNode;
    short currWeight;
    short edgeWeight;
};
void push();         // Push element to the top of the stack
struct Point pop();  // Remove and return the top most element of the stack
struct Point peek(); // Return the top most element of the stack
bool isEmpty();      // Check if the stack is in Underflow state or not
char checkDirection(char turn);
int nextTravelDir(short currX, short currY);
void HowToTurn(char currDir, char intendedDir);
short getDirIndex(char direction);
struct Point getNodeWithUnvisitedRoute();
bool verifyNodeVisited(short currX, short currY);

void AStar(short currX, short currY, struct Point endPoint);
short CheckIsPoint(struct Point currPoint, struct Point endPoint);
void SortQueue(struct QueueItem queue[], short end);
void Swapper(struct QueueItem queue[], short currIndex);
bool CheckIfBlocked();
int GetXYPoint();
char compass = 'N'; // default North
short directionCount = 0;

char directionString[15] = {'<', 'v', 'J', '>', '=', '.', 'm', '^', '`', 'L', '3', 'l', 'w', 'E', '+'};
char routeDirString[20];

short xPoints = 5;
short yPoints = 4;
struct GraphNode graphArray[5][4];

struct Point arr[20]; // 20 nodes

short visitedPoints = 0;
struct Point startpoint;

short currX;
short currY;
//

// RX interrupt handler
void on_uart_rx()
{
    while (uart_is_readable(UART_ID))
    {
        uint8_t ch = uart_getc(UART_ID);
        // Can we send it back?
        // printf("DATA %c", ch);
        // uart_putc(UART_ID, 'a');
        // printf("DATA");
    }
}

float measure_distance(ECHO_PIN, TRIGGER_PIN)
{
    float distance_cm = 0.0;
    uint echo_pin = 0;
    gpio_set_input_enabled(ECHO_PIN, true); // Enable input for the echo pin
    gpio_put(TRIGGER_PIN, false);           // make sure that the pin is low
    sleep_ms(2);                            // wait
    gpio_put(TRIGGER_PIN, true);            // go high for 3 ms
    sleep_ms(3);
    gpio_put(TRIGGER_PIN, false);                                         // Switch the trigger low again to let the SR04 send the pulse
                                                                          /*
                                                                              speed of sound is 343 meters per second.
                                                                              speed = distance travelled / time taken
                                                                              so, distance = (speed * time taken)/2 -- Divide by 2 because echo includes to target and back
                                                                              Now, speed of sound is 343 meters/second, whch is 3.43 centimeters per second, which is
                                                                              3.43/ 1000000 = 0.0343 cm/microsecond.
                                                                           */
    absolute_time_t listen_start_time = get_absolute_time();              // initiate listening
    absolute_time_t max_wait_time = delayed_by_ms(listen_start_time, 30); // Stop waiting more than 30ms.
    do
    {
        absolute_time_t t_now = get_absolute_time();                // get the time now
        int64_t diff = absolute_time_diff_us(max_wait_time, t_now); // time difference between max waiting and current time
        if (diff > 0)
        { // means more than 30ms
            break;
        }

        echo_pin = gpio_get(ECHO_PIN); // get input from the echo pin

        if (echo_pin != 0)
        {                                            // Recieve an echo!
            absolute_time_t first_echo_time = t_now; // start timing echo
            while (echo_pin == 1 && diff < 0)
            {                                  // while echo pin is high and timing less than 30ms
                echo_pin = gpio_get(ECHO_PIN); // get input from echo pin
                t_now = get_absolute_time();   // update the t_now
            }
            if (echo_pin == 1)
            {          // if the timing is more than 30 ms but it is still on high
                break; // will return 0 cm.
            }

            int64_t pulse_high_time = absolute_time_diff_us(first_echo_time, t_now); // find difference between first echo to now, returns int64_t
            float pulse_high_time_float = (float)pulse_high_time;                    // convert to float

            distance_cm = pulse_high_time_float * 0.01715; // time taken *0.5 *speed of sound;
            break;
        }
    } while (true);
    gpio_set_input_enabled(ECHO_PIN, false); // disable the input for echo pin
    return distance_cm;
}
bool moveForward();
int main()
{
    stdio_init_all();
    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, 115200);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);

    printf("-------- Welcome to barcode --------\n");
    adc_init();

    gpio_init(IR_GPIO);

    gpio_set_dir(IR_GPIO, GPIO_IN);

    adc_gpio_init(IR_GPIO);

    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    struct repeating_timer timer;

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    // Ultrasonic Setup
    gpio_init(C_TRIGGER_PIN);
    gpio_init(C_ECHO_PIN);
    gpio_set_dir(C_TRIGGER_PIN, GPIO_OUT);
    gpio_set_dir(C_ECHO_PIN, GPIO_IN);

    gpio_init(L_TRIGGER_PIN);
    gpio_init(L_ECHO_PIN);
    gpio_set_dir(L_TRIGGER_PIN, GPIO_OUT);
    gpio_set_dir(L_ECHO_PIN, GPIO_IN);

    gpio_init(R_TRIGGER_PIN);
    gpio_init(R_ECHO_PIN);
    gpio_set_dir(R_TRIGGER_PIN, GPIO_OUT);
    gpio_set_dir(R_ECHO_PIN, GPIO_IN);

    // OK, all set up.

    // Mapping variable

    short leftSensorDistance = 100; // test
    short rightSensorDistance = 0;  // test
    short forwardSensorDistance;

    short direction = 0; // default 0 means closed off routes.
    short routes = 0;

    bool northInput;
    bool eastInput;
    bool southInput;
    bool westInput;

    // Timer Interrupt
    // Occur every 1ms
    add_repeating_timer_ms(1, repeating_timer_callback, NULL, &timer);

    while (1)
    {

        // Finding the starting point based on distance of walls on the left and right
        if (leftSensorDistance > rightSensorDistance)
        {
            // initialization
            // struct Point point;
            startpoint.x, startpoint.y, currX, currY = 0;

            // U turn to face north

            struct GraphNode a = {1, direction, direction};
            graphArray[startpoint.x][startpoint.y] = a;

            arr[visitedPoints] = startpoint;
        }

        else if (leftSensorDistance < rightSensorDistance)
        {
            // initialization
            // struct Point point;
            startpoint.x = 4;
            currX = 4;
            startpoint.y = 0;
            currY = 0;
            // U turn to face north

            struct GraphNode a = {1, direction, direction};
            graphArray[startpoint.x][startpoint.y] = a;

            arr[visitedPoints] = startpoint;
        }

        printf("StartPoint: %d, %d\n", startpoint.x, startpoint.y);

        // Successfully moving to another node will add count to visitedPoints
        if (nextTravelDir(startpoint.x, startpoint.y))
        {
            visitedPoints++;
        }

        // While loop that stops when all 20 nodes have been discovered
        while (visitedPoints < 20)
        {
            printf("\nVisited points: %d", visitedPoints);
            printf("\nNext point to travel to: (%d, %d): ", currX, currY);
            // for (short i = 0; i < 20; i++)
            // {
            //    printf("(%d, %d)  ", arr[i].x, arr[i].y);
            // }
            printf("\n");
            for (short j = 0; j < xPoints; j++)
            {
                printf(((graphArray[j][0].directions | NORTHCHECK) != ACCESSIBLE) ? " ___ " : "    ");
            }

            for (short i = 0; i < yPoints; i++)
            {
                printf("\n");
                for (short j = 0; j < xPoints; j++)
                {
                    printf(((graphArray[j][i].directions | WESTCHECK) != ACCESSIBLE) ? "|" : " ");
                    printf(((graphArray[j][i].directions | SOUTHCHECK) != ACCESSIBLE) ? "___ " : "    ");
                }

                printf(((graphArray[xPoints - 1][i].directions | EASTCHECK) != ACCESSIBLE) ? "|" : " ");
            }
            printf("\n");
            // Check if the current node the car is on has been visited
            if (verifyNodeVisited(currX, currY) == true)
            {
                // Go to the next unvisited node next to the current node
                if (graphArray[currX][currY].routes != 0)
                {
                    nextTravelDir(currX, currY);
                }
                else
                {
                    // Go to the next node with unvisited nodes
                    printf("routes: %d", graphArray[currX][currY].routes);
                    struct Point endpoint = getNodeWithUnvisitedRoute();
                    printf("\nA STAR HERE");
                    // Take the shortest path to the next node
                    AStar(currX, currY, endpoint);
                    currX = endpoint.x;
                    currY = endpoint.y;
                }
            }
            else
            {
                // Arriving at a new node
                // Scanning accessible paths in front, left and right side of the car with matching directions
                direction = 0;
                routes = 0;
                // Handling the directions with binary
                // Car facing North. Check North, East and West
                if (compass == 'N')
                {
                    // if y means flip to 1 (open route), else n is 0 (close route)

                    northInput = CheckIfBlocked(L_ECHO_PIN, L_TRIGGER_PIN);
                    eastInput = CheckIfBlocked(C_ECHO_PIN, C_TRIGGER_PIN);
                    westInput = CheckIfBlocked(R_ECHO_PIN, R_TRIGGER_PIN);
                    // fflush(stdin);

                    if (!westInput)
                    {
                        direction++;
                        routes++;
                    }

                    if (!eastInput)
                    {
                        direction += 4;
                        routes += 4;
                    }

                    if (!northInput)
                    {
                        direction += 8;
                        routes += 8;
                    }

                    direction += 2;
                }

                // Car facing East. Check North, East and South
                else if (compass == 'E')
                {
                    // if y means flip to 1 (open route), else n is 0 (close route)
                    northInput = CheckIfBlocked(L_ECHO_PIN, L_TRIGGER_PIN);
                    eastInput = CheckIfBlocked(C_ECHO_PIN, C_TRIGGER_PIN);
                    southInput = CheckIfBlocked(R_ECHO_PIN, R_TRIGGER_PIN);
                    if (!southInput)
                    {
                        direction += 2;
                        routes += 2;
                    }

                    if (!eastInput)
                    {
                        direction += 4;
                        routes += 4;
                    }

                    if (!northInput)
                    {
                        direction += 8;
                        routes += 8;
                    }

                    direction++;
                }

                // Car facing South. Check East, South and West
                else if (compass == 'S')
                {
                    // if y means flip to 1 (open route), else n is 0 (close route)

                    eastInput = CheckIfBlocked(L_ECHO_PIN, L_TRIGGER_PIN);
                    southInput = CheckIfBlocked(C_ECHO_PIN, C_TRIGGER_PIN);
                    westInput = CheckIfBlocked(R_ECHO_PIN, R_TRIGGER_PIN);

                    if (!westInput)
                    {
                        direction++;
                        routes++;
                    }

                    if (!southInput)
                    {
                        direction += 2;
                        routes += 2;
                    }

                    if (!eastInput)
                    {
                        direction += 4;
                        routes += 4;
                    }

                    direction += 8;
                }

                // Car facing West. Check North, South and West
                else if (compass == 'W')
                {
                    // if y means flip to 1 (open route), else n is 0 (close route)
                    northInput = CheckIfBlocked(L_ECHO_PIN, L_TRIGGER_PIN);
                    southInput = CheckIfBlocked(C_ECHO_PIN, C_TRIGGER_PIN);
                    westInput = CheckIfBlocked(R_ECHO_PIN, R_TRIGGER_PIN);

                    if (!westInput)
                    {
                        direction++;
                        routes++;
                    }

                    if (!southInput)
                    {
                        direction += 2;
                        routes += 2;
                    }

                    if (!northInput)
                    {
                        direction += 8;
                        routes += 8;
                    }

                    direction += 4;
                }

                // Create a new graph node
                struct GraphNode a = {1, direction, routes};
                // Add the new graph node into the graph array
                graphArray[currX][currY] = a;
                // Set this node as the current node of the car
                struct Point currPoint = {currX, currY};
                arr[visitedPoints] = currPoint;
                if (nextTravelDir(currX, currY))
                {
                    visitedPoints++;
                }
            }
        }

        // raise and send mapping finished interrupt to comms
        // receive end point from comms
        printf("\n");
        for (short j = 0; j < xPoints; j++)
        {
            printf(((graphArray[j][0].directions | NORTHCHECK) != ACCESSIBLE) ? " ___ " : "    ");
        }

        for (short i = 0; i < yPoints; i++)
        {
            printf("\n");
            for (short j = 0; j < xPoints; j++)
            {
                printf(((graphArray[j][i].directions | WESTCHECK) != ACCESSIBLE) ? "|" : " ");
                printf(((graphArray[j][i].directions | SOUTHCHECK) != ACCESSIBLE) ? "___ " : "    ");
            }

            printf(((graphArray[xPoints - 1][i].directions | EASTCHECK) != ACCESSIBLE) ? "|" : " ");
        }

        short counter = 0;
        printf("\n");
        // Populate the routeDirString array with routes mapped to the predetermined chars to be polled by comms
        for (short i = 0; i < yPoints; i++)
        {

            for (short j = 0; j < xPoints; j++)
            {
                routeDirString[counter] = directionString[graphArray[j][i].directions - 1];
                counter++;
            }
        }
        printf("\n");
        for (short i = 0; i < 20; i++)
        {
            printf("%c", routeDirString[i]);
        }

        tight_loop_contents();
    }
}

// Barcode START
void decodeBarcode(uint16_t barcode_index, uint16_t barcode_arr[30])
{
    // Code 39 Start and End will always Contain this binary 1000101110111010
    if (barcode_index >= 20)
    {
        uint16_t min = 0;
        uint32_t barcodeValue = 0;
        uint8_t barcodeIndex = 9;
        uint16_t firstMax = findMax(barcode_arr[3], barcode_arr[4], barcode_arr[6], barcode_arr[8], barcode_arr[10]);

        // Decode Normally
        for (int i = 11; i < 20; i++)
        {
            int temp_value = 0;
            if (barcode_arr[i] > firstMax)
            {
                temp_value = 2;
            }
            else
            {
                temp_value = 1;
            }

            for (int k = 0; k < barcodeIndex - 1; k++)
            {
                temp_value *= 10;
            }
            barcodeValue += temp_value;
            barcodeIndex--;
        }

        for (int i = 0; i < 44; i++)
        {

            if (g_barcode_lookup_table[i] == barcodeValue)
            {

                printf("Result : ");

                printf("%c", g_barcode_lookup_match[i]);
                printf(" \n");
                break;
            }
        }
    }
}

uint16_t findMax(uint16_t secondData, uint16_t thirdData, uint16_t fourthData, uint16_t fifthData, uint16_t sixthData)
{
    // // If 1st value is Largest
    // if (firstData > secondData && firstData > thirdData && firstData > fourthData && firstData > fifthData && firstData > sixthData)
    // {
    //     return firstData;
    // }

    // If 2nd value is largest
    if (secondData > thirdData && secondData > fourthData && secondData > fifthData && secondData > sixthData)
    {
        return secondData;
    }

    // If 3rd data is largest
    if (thirdData > secondData && thirdData > fourthData && thirdData > fifthData && thirdData > sixthData)
    {
        return thirdData;
    }

    // If 4th data is largest
    if (fourthData > secondData && fourthData > thirdData && fourthData > fifthData && fourthData > sixthData)
    {
        return fourthData;
    }

    // If 5th data is largest
    if (fifthData > secondData && fifthData > thirdData && fifthData > fourthData && fifthData > sixthData)
    {
        return fifthData;
    }

    // if 6th data is largest
    if (sixthData > secondData && sixthData > thirdData && sixthData > fourthData && sixthData > fifthData)
    {
        return sixthData;
    }
}
// Barcode END
// Mapping & Navigation START
// Determines the direction and the node that the car travels to
// Car moves after determining the node
int nextTravelDir(short xPoint, short yPoint)
{
    if ((graphArray[xPoint][yPoint].routes | NORTHCHECK) == ACCESSIBLE) // Checks if north node can be travelled to
    {
        // Remove north from routes in struct
        graphArray[xPoint][yPoint].routes -= NORTHTOGGLE;
        HowToTurn(compass, 'N');
        printf("NORTH\n");
        // move forward 27cm
        moveForward();
        currY -= 1;
        return 1;
    }

    if ((graphArray[xPoint][yPoint].routes | EASTCHECK) == ACCESSIBLE) // Checks if east node can be travelled to
    {
        // go east
        // Remove east from routes in struct
        graphArray[xPoint][yPoint].routes -= EASTTOGGLE;
        HowToTurn(compass, 'E');
        printf("EAST\n");
        // move forward 27cm
        moveForward();
        currX += 1;
        return 2;
    }

    if ((graphArray[xPoint][yPoint].routes | SOUTHCHECK) == ACCESSIBLE) // Checks if south node can be travelled to
    {
        // go south
        // Remove south from routes in struct
        graphArray[xPoint][yPoint].routes -= SOUTHTOGGLE;
        HowToTurn(compass, 'S');
        printf("SOUTH\n");
        // move forward 27cm
        moveForward();
        currY += 1;
        return 3;
    }

    if ((graphArray[xPoint][yPoint].routes | WESTCHECK) == ACCESSIBLE) // Checks if west node can be travelled to
    {
        // go west
        // Remove west from routes in struct
        graphArray[xPoint][yPoint].routes -= WESTTOGGLE;
        HowToTurn(compass, 'W');
        printf("WEST\n");
        // move forward 27cm
        moveForward();
        currX -= 1;
        return 4;
    }

    if (graphArray[xPoint][yPoint].routes == 0) // Checks if west node can be travelled to
    {
        // u-turn
        // change direction(r)
        // change direction(r)

        // move forward 27cm
        moveForward();

        // currDir if N (y-1), E(x+1), S(y+1), W(x-1)

        return 4;
    }
}

// current direction takes in from checkDirection
void HowToTurn(char currDir, char intendedDir)
{
    short dirToTurn = (getDirIndex(intendedDir) - getDirIndex(currDir));
    // printf("%d", dirToTurn);
    compass = intendedDir;

    if (dirToTurn == 0)
    {
        // stay
        printf("STAY  ");
    }
    else if (dirToTurn == 1 || dirToTurn == -3)
    {
        // right
        printf("right  ");
        uart_putc(UART_ID, 'd');
    }
    else if (dirToTurn == 2 || dirToTurn == -2)
    {
        // u turn
        printf("u turn  ");
        uart_putc(UART_ID, 'u');
    }
    else if (dirToTurn == 3 || dirToTurn == -1)
    {
        // left
        printf("left  ");
        uart_putc(UART_ID, 'a');
    }
}

bool moveForward()
{
    for (int i = 0; i < 5; i++)
    {
        uart_putc(UART_ID, 'w');
    }
}

// get the index of the direction
short getDirIndex(char direction)
{
    if (direction == 'N')
    {
        return 0;
    }
    else if (direction == 'E')
    {
        return 1;
    }
    else if (direction == 'S')
    {
        return 2;
    }
    else if (direction == 'W')
    {
        // u turn
        return 3;
        // getDirection
        // check through
    }
}

struct Point getNodeWithUnvisitedRoute()
{

    for (int i = 0; i < 20; i++)
    {
        if (graphArray[arr[i].x][arr[i].y].routes != 0)
        {
            return arr[i];
        }
    }
}

// Checks if the current node has been visited
bool verifyNodeVisited(short currX, short currY)
{
    // printf("Verifying: (%d, %d)", currX,currY);
    //  20 is the size of arr
    for (short i = 0; i < visitedPoints; i++)
    {
        if (arr[i].x == currX && arr[i].y == currY)
        {
            // printf("true");
            return true;
        }
    }
    // printf("false");
    return false;
}

void AStar(short currX, short currY, struct Point endPoint)
{
    printf("\nStart point:(%d,%d)   End Point: (%d,%d)", currX, currY, endPoint.x, endPoint.y);
    short currQueue = 0;                        // The number of items in the queue
    short paths = 0;                            // Number of paths travelled
    struct QueueItem queuedNode = {0, 0, 0, 0}; // The node that needs to be checked

    queuedNode.nodeName.x = currX;
    queuedNode.nodeName.y = currY;
    struct Point pointToAdd = {0, 0};
    struct Point neighbourNode[maxsize];          // Array to hold the neighbours of the node
    short neighbours = 0;                         // Number of neighbours in the node
    struct QueueItem queue[maxarraysize];         // Initialise the queue array with the QueueItem struct, this will hold the queue for the paths to travel
    struct QueueItem pathTravelled[maxarraysize]; // Initialise the pathTravelled array with the QueueItem struct, this will hold the QueueItems of the paths that have been popped from the queue
    graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].checked = 3;

    while (CheckIsPoint(queuedNode.nodeName, endPoint) == 0) // If the current node to check is not the goal node
    {
        if ((graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].directions | NORTHCHECK) == ACCESSIBLE) // Checks if north node can be travelled to
        {

            pointToAdd.x = queuedNode.nodeName.x;
            pointToAdd.y = queuedNode.nodeName.y - 1;
            if (graphArray[pointToAdd.x][pointToAdd.y].checked != 3) // Checks if north node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;             // Add the north node of the current point to the neighbours array
                neighbours++;                                       // Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3; // Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                                                                    //  printf("\nAdd neighbour north: (%i, %i)", pointToAdd.x, pointToAdd.y);
            }
        }

        if ((graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].directions | EASTCHECK) == ACCESSIBLE) // Checks if east node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x + 1;
            pointToAdd.y = queuedNode.nodeName.y;

            if (graphArray[pointToAdd.x][pointToAdd.y].checked != 3) // Checks if east node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;             // Add the east node of the current point to the neighbours array
                neighbours++;                                       // Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3; // Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                // printf("\nAdd neighbour east: (%i, %i)", pointToAdd.x, pointToAdd.y);
            }
        }

        if ((graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].directions | SOUTHCHECK) == ACCESSIBLE) // Checks if south node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x;
            pointToAdd.y = queuedNode.nodeName.y + 1;

            if (graphArray[pointToAdd.x][pointToAdd.y].checked != 3) // Checks if south node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;             // Add the south node of the current point to the neighbours array
                neighbours++;                                       // Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3; // Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                // printf("\nAdd neighbour south: (%i, %i)", pointToAdd.x, pointToAdd.y);
            }
        }

        if ((graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].directions | WESTCHECK) == ACCESSIBLE) // Checks if west node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x - 1;
            pointToAdd.y = queuedNode.nodeName.y;

            if (graphArray[pointToAdd.x][pointToAdd.y].checked != 3) // Checks if west node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;             // Add the west node of the current point to the neighbours array
                neighbours++;                                       // Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3; // Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                // printf("\nAdd neighbour west: (%i, %i)\n", pointToAdd.x, pointToAdd.y);
            }
        }

        for (short i = neighbours - 1; i >= 0; i--) // For each neighbour in the neighbourNode array
        {

            short heuCost = abs(neighbourNode[i].x - endPoint.x) + abs(neighbourNode[i].y - endPoint.y);
            short currWeight = queuedNode.edgeWeight + 1;
            short totalWeight = currWeight + heuCost;

            struct QueueItem newItem = {neighbourNode[i], queuedNode.nodeName, totalWeight, currWeight};
            queue[currQueue] = newItem; // Adds the new item in the queue
            currQueue++;                // Increments the number of items in the queue
        }

        SortQueue(queue, currQueue); // To sort the queue from highest to lowest weight

        // After queue is sorted
        neighbours = 0;                    // Resets the number of neighbours
        currQueue -= 1;                    // Subtracts 1 from the number of items in the queue
        queuedNode = queue[currQueue];     // Sets the next node to check to the last item in the array (The one with the lowest weight)                                                                               //          (For checking)
        pathTravelled[paths] = queuedNode; // Adds the next node to the paths that have been travelled to
        paths++;                           // Increments the number of paths

        //          (For checking)
        // for (short i = paths - 1; i >= 0; i--)
        // {
        //     printf("\nPath %i, travelled: (%i, %i) from: (%i, %i)", i, pathTravelled[i].nodeName.x, pathTravelled[i].nodeName.y, pathTravelled[i].startNode.x, pathTravelled[i].startNode.y);
        // }
    }

    // After the goal has been reached
    struct Point finalPath[paths];                // Initialise an array for the final path to take
    paths -= 1;                                   // Subtract 1 from the number of paths
    short route = 1;                              // Initialise the number of routes
    finalPath[0] = pathTravelled[paths].nodeName; // Set the first index of the finalPath array to the last item in the pathTravelled array

    // Appends the routes to take to the finalPath array
    // while (CheckIsPoint(finalPath[route - 1], startpoint) == 0) // While the last index in the finalPath array is not the starting node
    //{
    for (short i = paths; i >= 0; i--) // Iterates through the pathTravelled array backwards
    {
        if (CheckIsPoint(pathTravelled[i].nodeName, finalPath[route - 1]) == 1) // If the name of the current iteration of the pathTravelled array matches the name of the last index of the finalPath array
        {
            finalPath[route] = pathTravelled[i].startNode; // Appends the current pathTravelled value to the finalPath array
            route++;                                       // Increments the number of routes
                                                           // break;
        }
    }
    // }

    // To print out the route to take
    printf("\n\nFinal Path: ");
    for (short i = route - 1; i > 0; i--)
    {
        printf("(%i,%i), ", finalPath[i].x, finalPath[i].y);

        if (finalPath[i].x > finalPath[i - 1].x)
        {
            // go west
            HowToTurn(compass, 'W');
        }
        else if (finalPath[i - 1].x > finalPath[i].x)
        {
            // go east
            HowToTurn(compass, 'E');
        }
        else if (finalPath[i - 1].y > finalPath[i].y)
        {
            // go south
            HowToTurn(compass, 'S');
        }
        else if (finalPath[i].y > finalPath[i - 1].y)
        {
            // go north
            HowToTurn(compass, 'N');
        }
    }

    printf("(%i,%i), ", finalPath[0].x, finalPath[0].y);

    // resets the checked variable of graphArray
    for (short i = 0; i < yPoints; i++)
    {
        printf("\n");
        for (short j = 0; j < xPoints; j++)
        {
            graphArray[j][i].checked = 1;
        }
    }
}

short CheckIsPoint(struct Point currPoint, struct Point endPoint) // Check if the current point is equal to the endpoint
{
    if (currPoint.x == endPoint.x && currPoint.y == endPoint.y)
    {
        return 1;
    }
    return 0;
}

void SortQueue(struct QueueItem queue[], short end) // To sort the queue from highest to lowest
{
    for (short sorted = end - 1; sorted >= 0; sorted--) // iterates through queue array
    {
        for (short i = end - 1; i > 0; i--) // When the unsorted portion is not at the end yet
        {
            if (queue[i].currWeight >= queue[i - 1].currWeight) // if the curr weight at the current index is greater or equal to the weight of the current index - 1, swap places
            {
                Swapper(queue, i);
            }
        }
    }
}

void Swapper(struct QueueItem queue[], short currIndex) // To swap the values around in the queue
{
    struct QueueItem temp = queue[currIndex]; // Saves the value of the unsorted item into temp
    queue[currIndex] = queue[currIndex - 1];
    queue[currIndex - 1] = temp;
}

bool CheckIfBlocked(int echo_pin, int trigger_pin)
{
    // take in ultrasonic
    // Ultra Sonic Start
    float distance = measure_distance(echo_pin, trigger_pin); // get reading from center sensor

    // the range of the ultrasonic sensor is 2 - 400 cm
    if (distance > 400)
    {
        // printf("\n C Out of range.");
        return true;
    }
    else if (distance < 2)
    {
        // printf("\n C Out of range. Too close!");
        return true;
    }
    else
    {
        // printf(" \nC Closest object is %5.2f cm away\n\r", C_distance);
        return false;
    }
}

int GetXYPoint()
{
    int xyPoint = currX * 10 + currY;
    return xyPoint;
}
// Mapping & Navigation END