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

void loop() {
     char cmd;
     char payload;

     // send data only when you receive data:
     if (Serial.available() > 0) {
        cmd = Serial.read();
        payload = Serial.read();

        switch(cmd) {
            case BOX:
                if(payload) b.raise_box();
                else        b.lower_box();
                break;

            case LIGHT:
                if(payload) b.set_lights(true);
                else        b.set_lights(false);
                break;

            case FLIP:      b.raise_box();
            default:        break;
        }

        Serial.write(cmd);
        Serial.write(payload);
     }
}

