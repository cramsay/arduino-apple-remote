/**
 * @file apple_remote.ino
 * @author Craig Ramsay
 * @date 29 Sep 2014
 * @brief Replacement arduino based apple remote control
 */

#include "ir_remote.h"
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

//Button state
typedef enum {off,debounce,on} btn_state_e;
typedef struct
{
    btn_state_e state;
    int pin;
} btn_state_t;
btn_state_t btns[] = {{off,7},{off,8},{off,9},{off,10},{off,11},{off,12}};
#define NUM_BTNS sizeof(btns)/sizeof(btn_state_t)

//Cmd codes
#define CMD_BYT_UP 0x0B
#define CMD_BYT_DOWN 0x0D
#define CMD_BYT_LEFT 0x08
#define CMD_BYT_RIGHT 0x07
#define CMD_BYT_CENTRE 0x04
#define CMD_BYT_MENU 0x02
byte cmd_bytes[] = {CMD_BYT_UP,CMD_BYT_LEFT,CMD_BYT_RIGHT,
					CMD_BYT_DOWN,CMD_BYT_CENTRE,CMD_BYT_MENU};

//Sleep stuff
//#define NO_SLEEP

/**
Setup routine run once at power on

Disables various unused features for power savings.
Sets up button connections with internal pullup resistors
*/
void setup()
{
    //Turn off I2C, SPI, ADC, USART0
    ADCSRA = 0;
    PRR = B10010111;

    //Setup btn pins (input with internal pullup enabled)
    for(int i=0; i<NUM_BTNS; i++) {
        pinMode(btns[i].pin,INPUT);
        digitalWrite(btns[i].pin,HIGH);
    }
}

/**
Reset all recorded button states to "off"
*/
void clear_btn_state() {
    for(int i=0; i<NUM_BTNS; i++) {
        btns[i].state=off;
    }

}

/**
Check all current button states and dispatch IR commands

Each active button is promoted to "debounce" state then
"on" on the next call. Whenever a button is promoted to
"on", the corresponding IR command is sent.
*/
void check_btns() {
    //For each button
    for(int i=0; i<NUM_BTNS; i++) {
        //If btn is pressed
        if(digitalRead(btns[i].pin)==0) {

            //Promote debouncing btns to pressed
            if(btns[i].state==debounce) {
                btns[i].state=on;
                //Send button command
                send_ir_cmd(cmd_bytes[i]);
            }
            //Promote off btns to "just pressed"
            else if(btns[i].state==off)
                btns[i].state=debounce;

            //Set to "not pressed"
        } else {
            btns[i].state=off;
        }
    }
}

/**
Sends the MCU to sleep safely

The MCU is set to the PWR_DOWN sleep mode.
An interrupt will wake the MCU when any button is depressed.
*/
void sleepNow(void)
{
#ifdef NO_SLEEP
    return;
#endif

    // Allow sleeping (disabled on interrupt)
    sleep_enable();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    // Set pin 2 as interrupt and attach handler:
    attachInterrupt(0, pinInterrupt, LOW);

	// Disable brown out detection when sleeping
	sleep_bod_disable();

    // Sleep
    sleep_cpu();

    // Code continutes from here once woken up
    sleep_disable();
}

/**
Simple interrupt on pin 2 to wake up the MCU from sleep mode
*/
void pinInterrupt(void)
{
    detachInterrupt(0);
    //Stop the MCU going into sleep if this is triggered too early
    sleep_disable();
}

/**
Main control loop

When no buttons are active sleep mode is triggered.
Otherwise, buttons are checked as normal.
*/
void loop() {
    //If no buttons are pressed, go to sleep (infer from interrupt signal)
    if(digitalRead(2)) {
        sleepNow();
        clear_btn_state();
    }
    //Check buttons at least twice due to debouncing code
    check_btns();
    delay(25);
    check_btns();
    delay(25);
}
