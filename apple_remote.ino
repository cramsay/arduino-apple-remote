#include <IRremote.h>
#include <IRremoteInt.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

//#define DEBUG 1

//Cmd codes
#define CMD_BYT_UP 0x0B
#define CMD_BYT_DOWN 0x0D
#define CMD_BYT_LEFT 0x08
#define CMD_BYT_RIGHT 0x07
#define CMD_BYT_CENTRE 0x04
#define CMD_BYT_MENU 0x02
byte cmd_bytes[] = {CMD_BYT_UP,CMD_BYT_LEFT,CMD_BYT_RIGHT,CMD_BYT_DOWN,
					CMD_BYT_CENTRE,CMD_BYT_MENU};

//Sleep stuff
#define SLEEP_POSTPONE_MS 1000
#define SLEEP_POSTPONE sleep_time=millis()+SLEEP_POSTPONE_MS
unsigned long sleep_time;

//Button state
typedef enum {off,debounce,on} btn_state_e;
typedef struct
{
	btn_state_e state;
	int pin;
} btn_state_t;
btn_state_t btns[] = {{off,7},{off,8},{off,9},{off,10},{off,11},{off,12}};
#define NUM_BTNS sizeof(btns)/sizeof(btn_state_t)

//IR library object
IRsend irsend;

void setup()
{	
	Serial.begin(9600);

	//Setup btn pins (input with internal pullup enabled)
	for(int i=0;i<NUM_BTNS;i++){
		pinMode(btns[i].pin,INPUT);
		digitalWrite(btns[i].pin,HIGH);
	}

	//Setup "awake" flag LED
	pinMode(13,OUTPUT);
    digitalWrite(13,HIGH);

	//Set an initial sleep time
	SLEEP_POSTPONE;
}

void send_cmd(byte cmd_code)
{
    byte data[] = {0xEE,0x87,cmd_code,0x59};

	#ifdef DEBUG
		Serial.print("Sending cmd: ");
		Serial.println(cmd_code,HEX);
	#endif

    irsend.enableIROut(38);

	//Send AGC leader
    irsend.mark(NEC_HDR_MARK);
    irsend.space(NEC_HDR_SPACE);

	//Send each byte in order, but least sig bit first
    for (int i = 0; i < sizeof(data)/sizeof(byte); i++) {
        for(int j=0; j<8; j++) {
            if (data[i] & 1) {
                irsend.mark(NEC_BIT_MARK);
                irsend.space(NEC_ONE_SPACE);
            }
            else {
                irsend.mark(NEC_BIT_MARK);
                irsend.space(NEC_ZERO_SPACE);
            }
            data[i] >>= 1;
        }
    }
    irsend.mark(NEC_BIT_MARK);
    irsend.space(0);
}

void clear_btn_state(){
	for(int i=0;i<NUM_BTNS;i++){
		btns[i].state=off;
	}

}

void check_btns(){

	//For each button
	for(int i=0;i<NUM_BTNS;i++){
		//If btn is pressed 
		if(digitalRead(btns[i].pin)==0){

			//Promote debouncing btns to pressed
			if(btns[i].state==debounce){
				btns[i].state=on;
				//Send button command
				send_cmd(cmd_bytes[i]);
				SLEEP_POSTPONE;
			}
			//Promote off btns to "just pressed"
			else if(btns[i].state==off)
				btns[i].state=debounce;
			

		//Set to "not pressed"
		}else{
			btns[i].state=off;
		}
	}

}

void sleepNow(void)
{
	#ifdef DEBUG
		Serial.print("Sending to sleep");
	#endif
    delay(100);
    digitalWrite(13,LOW);   // turn LED off to indicate sleep
    sleep_enable();
    // Set pin 2 as interrupt and attach handler:
    attachInterrupt(0, pinInterrupt, LOW);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_cpu();
    // Upon waking up, sketch continues from this point.
    sleep_disable();
	#ifdef DEBUG
		Serial.print("Waking up");
	#endif
    digitalWrite(13,HIGH);   // turn LED on to indicate awake
	SLEEP_POSTPONE;
}
void pinInterrupt(void)
{
    detachInterrupt(0);
    sleep_disable();
	#ifdef DEBUG
		Serial.print("In interrupt");
	#endif
}

void loop() {
	if(millis()>=sleep_time){
		sleepNow();
		clear_btn_state();
	}
	check_btns();	
	delay(10);
}
