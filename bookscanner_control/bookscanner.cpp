#include "bookscanner.h"

#include <Arduino.h>

// Relay Pins
#define VAC_PUMP 4
#define PES_PUMP 5
#define LAMP 6
#define FAN 7

// Start/Stop/E-Stop button contact (INPUT_PULLUP, active-low: pressed = LOW).
#define BUTTON 2

// RGB status-LED channels (PWM). Common-anode ring: the cathodes on these
// pins sink current, so "more on" means driving the pin more LOW — set_led()
// inverts accordingly (monospace.md §10.3).
#define LED_R 9
#define LED_G 10
#define LED_B 11

// Ignore further contact transitions for this long after a reported press
// edge — signal conditioning only, not "automatic behavior" (§3 point 8,
// §10.2). Exact value is an implementation detail per the spec.
#define BUTTON_DEBOUNCE_MS 30

void do_log(int line, const char *key, int val) {
    Serial.print(line, DEC);
    Serial.print(" - ");
    Serial.print(key);
    Serial.print(": ");
    Serial.println(val);
}

Bookscanner::Bookscanner():
    bmp180(),
    button_last_reading(HIGH),  // idle = HIGH (internal pull-up)
    button_last_edge_ms(0)
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

    // Status LED off before the serial interface starts accepting commands
    // (§3 point 7 — no boot-time exception here, unlike the light relay).
    // set_led(0,0,0) drives all three cathode pins HIGH (fully off) on this
    // common-anode ring.
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    set_led(0, 0, 0);

    // Start/Stop/E-Stop button, active-low with the internal pull-up.
    pinMode(BUTTON, INPUT_PULLUP);

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
    // Oversampling=2: measured ~49Hz / ~0.034mbar stdev on real hardware,
    // vs. OSS=0's ~78Hz / ~0.051mbar stdev - noticeably less per-sample
    // noise while still comfortably clear of the >=5Hz floor and close
    // to the <=50Hz ideal (monospace.md §6).
    return read_pressure_avg(2, 1);
}

/// RGB status LED (common-anode, active-low cathodes — monospace.md §10.3).
/// The host speaks in plain 0=off..255=on values; the inversion to the
/// cathode drive level happens here and is never exposed over the wire.
void Bookscanner::set_led(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

/// Debounced button poll (monospace.md §10.2). Reports one press per
/// idle->pressed edge; a simple time guard ignores contact bounce for
/// BUTTON_DEBOUNCE_MS after each reported edge. Release is not reported.
/// Uses unsigned millis() subtraction so the ~49-day wrap is handled.
bool Bookscanner::poll_button() {
    int reading = digitalRead(BUTTON);
    bool pressed_edge = false;

    if (reading == LOW && button_last_reading == HIGH) {
        unsigned long now = millis();
        if (now - button_last_edge_ms >= BUTTON_DEBOUNCE_MS) {
            pressed_edge = true;
            button_last_edge_ms = now;
        }
    }
    button_last_reading = reading;
    return pressed_edge;
}
