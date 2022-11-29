## Robocar Motor

The motor component of the car is ran on the MSP432.

Using 2 timers for the PWM and 1 timer for the car to perform the correction every 1 second to use the PID to correct the vehicle.

Initially, the car starts off with a 2000/10000 dutycycle and runs for 2 seconds before correction of the dutycycle values.

Next, after the car spins up and moves for 2 seconds, the correction kicks in. Using the value set by the presetNotch function, the car will attempt to either speed up or slow down based on the number of notches it needs to acheive in 1 second.

This is done by the PID controller which will attempt to correct both wheels to the preset notch value as long as there is a difference between the preset notch and the number of notches recorded from the encoder.

To control the direction of the car, functions are written to manipulate the direction pins of the motor driver to turn or stop the car. This functions can also be called via UART by the mapping componets located within the PICO.

Should a ramp be encountered, the car will attempt to go up the ramp by increasing the duty cycle to 9000 and get over the hump. Once its across the hump, if the number of notches recorded exceeds the preset notch by greater than 3, it slows the car down instantly by setting the dutycycle back to 2000.
