#include "bookscanner.h"

#include <Arduino.h>

// Relay Pins
#define VAC_PUMP 4
#define PES_PUMP 5
#define LAMP 6
#define FAN 7

void do_log(int line, const char *key, int val) {
    Serial.print(line, DEC);
    Serial.print(" - ");
    Serial.print(key);
    Serial.print(": ");
    Serial.println(val);
}

Bookscanner::Bookscanner():
    bmp180()
{
}

void Bookscanner::begin() {
    // Set up Relay board, then force everything off before anything else
    // (including the serial interface) can act on these pins.
    pinMode(FAN, OUTPUT);
    pinMode(VAC_PUMP, OUTPUT);
    pinMode(PES_PUMP, OUTPUT);
    pinMode(LAMP, OUTPUT);
    set_fan(false);
    set_blower(false);
    set_vacuum(false);
    set_light(false);
    // Redundant with the per-relay calls above — kept as belt-and-suspenders
    // for the safety-critical "nothing energized at boot" invariant.
    digitalWrite(FAN, HIGH);
    digitalWrite(VAC_PUMP, HIGH);
    digitalWrite(PES_PUMP, HIGH);
    digitalWrite(LAMP, HIGH);

    // Initialise Pressure Sensor
    bmp180.begin();

    DEBUG_LOG("READY", 1);
}

/// Vacuum Pump Helper (active-low relay)
void Bookscanner::set_vacuum(bool state) {
    digitalWrite(VAC_PUMP, !state);
}

/// Page-Separation ("Flutter") Fan Helper (active-low relay)
void Bookscanner::set_fan(bool state) {
    digitalWrite(FAN, !state);
}

/// Turn Blower ("Positive Pressure Pump") Helper (active-low relay)
void Bookscanner::set_blower(bool state) {
    digitalWrite(PES_PUMP, !state);
}

/// Illumination Helper (active-low relay)
void Bookscanner::set_light(bool state) {
    digitalWrite(LAMP, !state);
}

void Bookscanner::all_off() {
    set_vacuum(false);
    set_fan(false);
    set_blower(false);
}

float Bookscanner::read_pressure_avg(char oversampling, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) {
        char ms = bmp180.startTemperature();
        if (ms == 0) {
            DEBUG_LOG("startTemp failed", 0)
        } else {
            delay(ms);
        }
        double t;
        bmp180.getTemperature(t);

        ms = bmp180.startPressure(oversampling);
        if (ms == 0) {
            DEBUG_LOG("startPressure failed", 0)
        } else {
            delay(ms);
        }
        double p;
        bmp180.getPressure(p, t);
        sum += p;
    }
    return (float)(sum / n);
}

float Bookscanner::press_once() {
    return read_pressure_avg(3, 8);
}

float Bookscanner::press_stream_sample() {
    return read_pressure_avg(0, 1);
}
