#define DEBUG
#include "bookscanner.h"

Bookscanner b = Bookscanner();

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("INIT");
    b.begin();
    Serial.println("READY");
    pinMode(7, OUTPUT);
}

void loop() {
    char cmd;
    //digitalWrite(7, 1);
    //delay(500);
    //digitalWrite(7, 0);
    //delay(500);  
    //Serial.println("Ready");
    if (Serial.available() > 0) {
        // read the incoming byte:
        cmd = Serial.read();
        Serial.println(cmd);
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
