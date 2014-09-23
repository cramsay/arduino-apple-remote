#include <IRremote.h>
#include <IRremoteInt.h>
IRsend irsend;

//Cmd codes
#define CMD_BYT_UP 0x0B
#define CMD_BYT_DOWN 0x0D
#define CMD_BYT_LEFT 0x08
#define CMD_BYT_RIGHT 0x07
#define CMD_BYT_CENTRE 0x04
#define CMD_BYT_MENU 0x02

//Cmd keys
#define CMD_KEY_UP 'w'
#define CMD_KEY_DOWN 's'
#define CMD_KEY_LEFT 'a'
#define CMD_KEY_RIGHT 'd'
#define CMD_KEY_CENTRE 'q'
#define CMD_KEY_MENU 'e'

void setup()
{
    Serial.begin(9600);
}

void loop() {
    char cmd_char;
    if ( -1 != (cmd_char=Serial.read()) ) {
        switch(cmd_char) {
        case CMD_KEY_UP:
            send_cmd(CMD_BYT_UP);
            break;
        case CMD_KEY_DOWN:
            send_cmd(CMD_BYT_DOWN);
            break;
        case CMD_KEY_LEFT:
            send_cmd(CMD_BYT_LEFT);
            break;
        case CMD_KEY_RIGHT:
            send_cmd(CMD_BYT_RIGHT);
            break;
        case CMD_KEY_CENTRE:
            send_cmd(CMD_BYT_CENTRE);
            break;
        case CMD_KEY_MENU:
            send_cmd(CMD_BYT_MENU);
            break;
        default:
            break;
        }
    }
}

void send_cmd(byte cmd_code)
{
    byte data[] = {0xEE,0x87,cmd_code,0x59};

    irsend.enableIROut(38);
    irsend.mark(NEC_HDR_MARK);
    irsend.space(NEC_HDR_SPACE);
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

