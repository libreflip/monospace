#define DEBUG
#include "bookscanner.h"

Bookscanner b = Bookscanner();

#define BOX         0b00000010
#define LIGHT       0b00000100
#define FLIP        0b00001000

#define ON          0b00000001
#define OFF         0b00000000


void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    b.begin();
    b.raise_box();
}

void reply(Response resp) {
    Serial.write(resp.error);
    Serial.write(resp.payload_len);

    for(int i = 0; i < resp.payload_len; i++) {
        Serial.write(resp.payload[i]);
    }
}

void loop() {
     char cmd;
     char payload;

     // A command is 2 bytes long so we want to have
     // at least that amount to read
     if (Serial.available() >= 2) {
        cmd = Serial.read();
        payload = Serial.read();

        switch(cmd) {
            case BOX:
                if(payload) reply(b.raise_box());
                else        reply(b.lower_box());
                break;

            case LIGHT:
                if(payload) reply(b.set_lights(true));
                else        reply(b.set_lights(false));
                break;

            case FLIP:      reply(b.flip_page(payload));
            default:        break;
        }
     }
}

