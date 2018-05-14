#define DEBUG
#include "bookscanner.h"

Bookscanner b = Bookscanner();

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
}

void loop() {
    char cmd;
    if (Serial.available() > 0) {
        // read the incoming byte:
        cmd = Serial.read();
        switch(cmd) {
        case 'u':
             Serial.print("UP\n");
             b.raise_box();
             break;
        case 'd':
             Serial.print("DOWN\n");
             b.lower_box();
             break;
        case 'f':
             Serial.print("FLIP\n");
             // fuk it I am hardcoding it, 
             // rebuild it if you wanna change it
             b.flip_page(100);
             break;
        }
    }
}
