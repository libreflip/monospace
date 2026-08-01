#include "bookscanner.h"

#include <avr/wdt.h>
#include <string.h>

Bookscanner b = Bookscanner();

#define LINEBUF_SIZE 40
char linebuf[LINEBUF_SIZE];
uint8_t linelen = 0;
bool streaming = false;

void handle_line(char *line) {
    if      (!strcmp(line, "VACUUM ON"))    { b.set_vacuum(true);  Serial.println(F("OK")); }
    else if (!strcmp(line, "VACUUM OFF"))   { b.set_vacuum(false); Serial.println(F("OK")); }
    else if (!strcmp(line, "FAN ON"))       { b.set_fan(true);     Serial.println(F("OK")); }
    else if (!strcmp(line, "FAN OFF"))      { b.set_fan(false);    Serial.println(F("OK")); }
    else if (!strcmp(line, "BLOWER ON"))    { b.set_blower(true);  Serial.println(F("OK")); }
    else if (!strcmp(line, "BLOWER OFF"))   { b.set_blower(false); Serial.println(F("OK")); }
    else if (!strcmp(line, "LIGHT ON"))     { b.set_light(true);   Serial.println(F("OK")); }
    else if (!strcmp(line, "LIGHT OFF"))    { b.set_light(false);  Serial.println(F("OK")); }
    else if (!strcmp(line, "ALL OFF"))      { b.all_off();         Serial.println(F("OK")); }
    else if (!strcmp(line, "PRESS?"))       { Serial.print(F("OK ")); Serial.println(b.press_once(), 2); }
    else if (!strcmp(line, "PRESS START"))  { streaming = true;    Serial.println(F("OK")); }
    else if (!strcmp(line, "PRESS STOP"))   { streaming = false;   Serial.println(F("OK")); }
    else { Serial.println(F("ERR UNKNOWN_COMMAND")); }
}

void setup() {
    // Must be the very first thing that happens: clears a stale watchdog
    // reset-cause flag and disarms any watchdog config left over from a
    // previous reset, before we (re-)arm our own below. Skipping this can
    // cause a fast, silent reset loop on some AVR/bootloader combinations.
    MCUSR = 0;
    wdt_disable();

    // Arm the watchdog BEFORE b.begin(), not after: b.begin() calls into
    // the BMP180 driver, whose I2C read helper (SFE_BMP180.cpp) contains an
    // unbounded `while (Wire.available() != length) ;` wait with no timeout
    // of its own — exactly the I2C-lockup failure mode this watchdog exists
    // to recover from. Arming it any later would leave that call unprotected.
    wdt_enable(WDTO_8S);

    // Relays forced off before the serial interface starts accepting commands.
    b.begin();
    Serial.begin(115200);
}

void loop() {
    wdt_reset();

    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n') {
            if (linelen > 0 && linebuf[linelen - 1] == '\r') {
                linelen--;
            }
            linebuf[linelen] = '\0';
            handle_line(linebuf);
            linelen = 0;
        } else if (linelen < LINEBUF_SIZE - 1) {
            linebuf[linelen++] = c;
        } else {
            // Line too long for the buffer — drop it rather than overflow.
            linelen = 0;
        }
    }

    if (streaming) {
        Serial.print(F("PRESS "));
        Serial.println(b.press_stream_sample(), 2);
    }
}
