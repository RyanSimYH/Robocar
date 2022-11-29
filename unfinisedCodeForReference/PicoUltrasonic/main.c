#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/time.h"
#include "pico/types.h"

// https://github.com/vipullal-github/HC-SR04_UltrasonicSensor
uint led_pin = 25;
//Define GPIO pins for the ultrasonic trigger and echo pins for all three sensors
uint C_TRIGGER_PIN = 17;
uint C_ECHO_PIN = 16;
uint L_TRIGGER_PIN = 18;
uint L_ECHO_PIN = 19;
uint R_TRIGGER_PIN = 20;
uint R_ECHO_PIN = 21;
// -----------------------------------------------------------
float   measure_distance(ECHO_PIN,TRIGGER_PIN){
    float   distance_cm = 0.0;
    gpio_set_input_enabled( ECHO_PIN, true );   //Enable input for the echo pin
    gpio_put(TRIGGER_PIN, false);               // make sure that the pin is low
    sleep_ms(2);                                // wait 
    gpio_put(TRIGGER_PIN, true);                // go high for 3 ms
    sleep_ms(3);                               
    gpio_put(TRIGGER_PIN, false);               // Switch the trigger low again to let the SR04 send the pulse
	/*
		The echo pin woutputs a pulse between 150 micro-seconds and 25 mili-seconds, or if no object is found, it will send a 38 ms pulse. 
		speed of sound is 343 meters per second.
		speed = distance travelled / time taken
		so, distance = (speed * time taken)/2 -- Divide by 2 because echo includes to target and back
		Now, speed of sound is 343 meters/second, whch is 3.43 centimeters per second, which is
		3.43/ 1000000 = 0.0343 cm/microsecond. 
	 */
    absolute_time_t  listen_start_time = get_absolute_time(); // initiate listening
    absolute_time_t  max_wait_time  = delayed_by_ms( listen_start_time, 30 );  // Stop waiting more than 30ms. 
	do{
        absolute_time_t  t_now = get_absolute_time(); // get the time now
        int64_t diff = absolute_time_diff_us( max_wait_time,t_now); //time difference between max waiting and current time
        if( diff>0 ){ //means more than 30ms
            break;
        }
        echo_pin = gpio_get(ECHO_PIN); // get input from the echo pin

        if( echo_pin != 0 ){   // Recieve an echo!
            absolute_time_t  first_echo_time = t_now; // start timing echo
            while( echo_pin == 1 && diff<0 ){ //while echo pin is high and timing less than 30ms
                echo_pin = gpio_get( ECHO_PIN ); //get input from echo pin
                t_now = get_absolute_time(); // update the t_now
            }
            if( echo_pin == 1 ){ // if the timing is more than 30 ms but it is still on high
                break;  // will return 0 cm.
                }
    
        int64_t pulse_high_time = absolute_time_diff_us( first_echo_time, t_now ); // find difference between first echo to now, returns int64_t 
        float pulse_high_time_float = (float) pulse_high_time; //convert to float
            
        distance_cm = pulse_high_time_float * 0.01715; //time taken *0.5 *speed of sound;
        break;
        }
    }
    while( true );
    gpio_set_input_enabled( ECHO_PIN, false );//disable the input for echo pin  
    return distance_cm;
}



// ---------------------------------------------------------------
int main() {
    
    stdio_init_all();
    // setup the trigger pin to output mode and echo pin to input mode
    gpio_init( C_TRIGGER_PIN );
    gpio_init( C_ECHO_PIN );
    gpio_set_dir( C_TRIGGER_PIN, GPIO_OUT );
    gpio_set_dir( C_ECHO_PIN, GPIO_IN );

    gpio_init( L_TRIGGER_PIN );
    gpio_init( L_ECHO_PIN );
    gpio_set_dir( L_TRIGGER_PIN, GPIO_OUT );
    gpio_set_dir( L_ECHO_PIN, GPIO_IN );

    gpio_init( R_TRIGGER_PIN );
    gpio_init( R_ECHO_PIN );
    gpio_set_dir( R_TRIGGER_PIN, GPIO_OUT );
    gpio_set_dir( R_ECHO_PIN, GPIO_IN );

    gpio_init(led_pin);
    gpio_set_dir( led_pin, GPIO_OUT);
    
    
    while (true) {
        // Blinks once per reading 
        //reads distance every 0.02 sec
        gpio_put(led_pin, true);
        sleep_ms(10);
        gpio_put(led_pin, false);
        sleep_ms(10);
        
        float  C_distance = measure_distance(C_ECHO_PIN,C_TRIGGER_PIN); // get reading from center sensor
        float  L_distance = measure_distance(L_ECHO_PIN,L_TRIGGER_PIN); // get reading from left sensor
        float  R_distance = measure_distance(R_ECHO_PIN,R_TRIGGER_PIN); // get reading from right sensor
        
        
        // the range of the ultrasonic sensor is 2 - 400 cm 
        if (C_distance>400){ 
            printf("\n C Out of range.");
        }else if(C_distance<2){
            printf("\n C Out of range. Too close!");
        }else{
        printf(" \nC Closest object is %5.2f cm away\n", C_distance );
        }
        if (L_distance>400){
            printf("\n L Out of range.");
        }else if(L_distance<2){
            printf("\n L Out of range. Too close!");
        }else{
        printf(" \nL Closest object is %5.2f cm away\n", L_distance );
        }
        if (R_distance>400){
            printf("\n R Out of range.");
        }else if(R_distance<2){
            printf("\n R Out of range. Too close!");
        }else{
        printf(" \n R Closest object is %5.2f cm away\n", R_distance );
        }

    
}
}
