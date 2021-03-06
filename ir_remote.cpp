#include "ir_remote.h"

#include "Arduino.h"
#include <avr/interrupt.h>

/**
 * Sends an IR mark for the given number of microseconds.
 * The mark output is modulated at the PWM frequency 
 *
 * @param time Microsecond duration of the mark
 */
void mark(int time) {
  TIMER_ENABLE_PWM;
  delayMicroseconds(time);
}

/**
 * Sends an IR space for the specified number of microseconds.
 * A space is no output, so the PWM output is disabled.
 *
 * @param time Microsecond duration of the space
 */
void space(int time) {
  TIMER_DISABLE_PWM;
  delayMicroseconds(time);
}

/**
 * Enable IR PWM controls at a given modulation frequency
 *
 * TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
 * controlling the duty cycle.
 * There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
 * To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
 * A few hours staring at the ATmega documentation and this will all make sense.
 * See post "Secrets of Arduino PWM" at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.
 *
 * @param khz The desired modulation frequency
 */
void enable_ir_out(int khz) {

  pinMode(TIMER_PWM_PIN, OUTPUT);
  digitalWrite(TIMER_PWM_PIN, LOW); // When not sending PWM, we want it low
  
  // COM2A = 00: disconnect OC2A
  // COM2B = 00: disconnect OC2B; to send signal set to 10: OC2B non-inverted
  // WGM2 = 101: phase-correct PWM with OCRA as top
  // CS2 = 000: no prescaling
  // The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR2A.
  TIMER_CONFIG_KHZ(khz);
}

/**
Sends a single IR NEC command

It uses the Arduino-IRremote library to do the clever PWM stuff.
Builds the whole payload (apple uid : cmd : remote id) and then
sends the bytes in order, but least significant bit first.
See https://en.wikipedia.org/wiki/Apple_Remote#Technical_details
for details on the cmd codes and parity bit.

@param cmd_code the command code to be sent
*/
void send_ir_cmd(unsigned char cmd_code)
{
    byte data[] = {0xEE,0x87,cmd_code,0x59};

    enable_ir_out(38);

    //Send AGC leader
    mark(NEC_HDR_MARK);
    space(NEC_HDR_SPACE);

    //Send each byte in order, but least sig bit first
    for (int i = 0; i < sizeof(data)/sizeof(byte); i++) {
        for(int j=0; j<8; j++) {
            if (data[i] & 1) {
                mark(NEC_BIT_MARK);
                space(NEC_ONE_SPACE);
            }
            else {
                mark(NEC_BIT_MARK);
                space(NEC_ZERO_SPACE);
            }
            data[i] >>= 1;
        }
    }
    mark(NEC_BIT_MARK);
    space(0);
}
