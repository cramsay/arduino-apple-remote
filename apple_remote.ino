#include <SPI.h>
#include <Ethernet.h>
#include <IRremote.h>
#include <IRremoteInt.h>

//String buffer size
#define BUFSIZE 256

//Cmd codes
#define CMD_BYT_UP 0x0B
#define CMD_BYT_DOWN 0x0D
#define CMD_BYT_LEFT 0x08
#define CMD_BYT_RIGHT 0x07
#define CMD_BYT_CENTRE 0x04
#define CMD_BYT_MENU 0x02

//Ethernet connection
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192,168,0,69);
EthernetServer server(80);

//IR library object
IRsend irsend;

void setup()
{
    Ethernet.begin(mac, ip);
    server.begin();
}

void send_cmd(byte cmd_code)
{
    byte data[] = {0xEE,0x87,cmd_code,0x59};
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

void loop() {

    // listen for incoming clients
    EthernetClient client = server.available();
    if (client) {

        char clientline[BUFSIZE];
        int index = 0;

        while (client.connected()) {

            if (client.available()) {
                char c = client.read();

                // fill url the buffer
                if(c != '\n' && c != '\r' && index < BUFSIZE) {
                    clientline[index++] = c;
                    continue;
                }
				
				//Only reaches here on a new line in the header
				//Flush out the rest of the request
                client.flush();

				//Parse URL and run commands if matched
				String urlString = String(clientline);
				if(urlString.startsWith("GET /menu")){
					send_cmd(CMD_BYT_MENU);
				}
				else if(urlString.startsWith("GET /centre")){
					send_cmd(CMD_BYT_CENTRE);
				}
				else if(urlString.startsWith("GET /up")){
					send_cmd(CMD_BYT_UP);
				}
				else if(urlString.startsWith("GET /down")){
					send_cmd(CMD_BYT_DOWN);
				}
				else if(urlString.startsWith("GET /left")){
					send_cmd(CMD_BYT_LEFT);
				}
				else if(urlString.startsWith("GET /right")){
					send_cmd(CMD_BYT_RIGHT);
				}

                // send a standard http response header
                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: text/html");
                client.println("Connection: close");  // the connection will be closed after completion of the response
                client.println();
                client.println("<!DOCTYPE HTML>");
                client.println("<html><head></head><body>Thanks.</body></html>");
                break;
            }
        }
        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        client.stop();
    }
}
