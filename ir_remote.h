/**
 * This is a stripped down version of Shirriff's Arduino-IRremote
 * library (https://github.com/shirriff/Arduino-IRremote).
 *
 * There is an added NEC transmission function (send_ir_cmd)
 * which will send a valid apple remote signal given a command byte.
 * There is only support for transmitting IR signals.
 */
#include <Arduino.h>

//System clock
#ifdef F_CPU
#define SYSCLOCK F_CPU     // main Arduino clock
#else
#define SYSCLOCK 16000000  // main Arduino clock
#endif

// pulse parameters in usec
#define NEC_HDR_MARK	9000
#define NEC_HDR_SPACE	4500
#define NEC_BIT_MARK	560
#define NEC_ONE_SPACE	1600
#define NEC_ZERO_SPACE	560
#define NEC_RPT_SPACE	2250

// Timer macros
#define TIMER_PWM_PIN        3
#define TIMER_ENABLE_PWM     (TCCR2A |= _BV(COM2B1))
#define TIMER_DISABLE_PWM    (TCCR2A &= ~(_BV(COM2B1)))
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint8_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR2A = _BV(WGM20); \
  TCCR2B = _BV(WGM22) | _BV(CS20); \
  OCR2A = pwmval; \
  OCR2B = pwmval / 3; \
})

//Function declarations
void mark(int time);
void space(int time);
void enable_ir_out(int khz);
void send_ir_cmd(unsigned char cmd);
