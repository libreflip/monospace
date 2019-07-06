#include "bookscanner.h"

#include <Arduino.h>

#define STEPS 200
#define STEPS_PER_MM 34 //6.5

// Motor Shield pins
#define DIR_A 12
#define DIR_B 13
#define PWM_A 3
#define PWM_B 11
#define BRK_A 9
#define BRK_B 8

// Relay Pins
#define FAN 7
#define VAC_PUMP 4
#define PES_PUMP 5
#define LAMP 6

// Limit Switch
#define LIM_SW 10 // TBC

void do_log(int line, const char *key, int val) {
    Serial.print(line, DEC);
    Serial.print(" - ");
    Serial.print(key);
    Serial.print(": ");
    Serial.println(val);
}

/// Flutter Fan Helper
void set_fan(bool state) {
    digitalWrite(FAN, !state);
}

/// Vaccum Pump Helper
void set_vac_pump(bool state) {
    digitalWrite(VAC_PUMP, !state);
}

/// Positive Pressure Pump Helper
void set_blow_pump(bool state) {
    digitalWrite(PES_PUMP, !state);
}

Bookscanner::Bookscanner():
    motor(STEPS, DIR_B, DIR_A),
    bmp180()
    {

}

void Bookscanner::begin() {
    // Set up extended Stepper control
    pinMode(PWM_A, OUTPUT);
    pinMode(PWM_B, OUTPUT);
    pinMode(BRK_A, OUTPUT);
    pinMode(BRK_B, OUTPUT);
    // Brakes off; Drivers off
    digitalWrite(BRK_A, 0);
    digitalWrite(BRK_B, 0);
    digitalWrite(PWM_A, 0);
    digitalWrite(PWM_B, 0);
    drivers = false;
    //the Brakes here are meaningless, you won't here from them again.
    motor.setSpeed(50);

    set_drivers(false);
    // everything is off so we can assume gravity has done
    // it's job and the box is at the bottom of the track.
    this->head_pos = 0; // resting on the book

    // Set up Relay board
    pinMode(FAN, OUTPUT);
    pinMode(VAC_PUMP, OUTPUT);
    pinMode(PES_PUMP, OUTPUT);
    pinMode(LAMP, OUTPUT);
    // Set everything off
    set_fan(false);
    set_blow_pump(false);
    set_vac_pump(false);
    digitalWrite(FAN, 1);
    digitalWrite(VAC_PUMP, 1);
    digitalWrite(PES_PUMP, 1);
    digitalWrite(LAMP, 1);

    // Initialise Pressure Sensor
    bmp180.begin();

    // configure Limit switch
    pinMode(LIM_SW, INPUT_PULLUP);
    DEBUG_LOG("READY", 1);
    // Box is set up
}

bool Bookscanner::read_lim() {
    // Normally Open Switch to ground with pullup
    return digitalRead(LIM_SW);
}

Response Bookscanner::raise_box() {
    DEBUG_LOG("RAISING", 1);
    set_drivers(true);
    while (!read_lim()) {
        motor.step(1);
    }
    DEBUG_LOG("LIM_HIT", 1);
    head_pos = 32768; //Max 16Bit int
    set_drivers(false);
    return new_response(ERROR_OK);
}

Response Bookscanner::lower_box() {
    set_drivers(false);
    head_pos = 0;
    return new_response(ERROR_OK);
}

/// Illumination assistence
Response Bookscanner::set_lights(bool state) {
    digitalWrite(LAMP, !state);
    return new_response(ERROR_OK);
}

/// Move Head to specified position
/// Blocks while moving
bool Bookscanner::move_to(int pos) {
    int diff = pos - head_pos;
    motor.step(diff);
    head_pos = pos;
}

/// Presure Sensor Helper
double Bookscanner::read_pressure_sensor() {
    char ms = bmp180.startTemperature();
    if (ms == 0) {
        DEBUG_LOG("startTemp failed", 0)
    } else {
        delay(ms);
    }
    double t;
    bmp180.getTemperature(t);
    ms = bmp180.startPressure(0);
    if (ms == 0) {
        DEBUG_LOG("startPressure failed", 0)
    } else {
        delay(ms);
    }
    double p;
    bmp180.getPressure(p, t);
    return p;
}

void Bookscanner::set_drivers(bool state) {
    digitalWrite(PWM_A, state);
    digitalWrite(PWM_B, state);
    drivers = state;
    if (state == false) {
        head_pos = 0;
    }
}

Response Bookscanner::new_response(Error code) {
    return {
        code,
        0,
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };
}

/// Run a Complete Page flip cycle autonomously
/// @param page_size: Size of the page we are flipping in mm
Response Bookscanner::flip_page(uint8_t page_size) {
    float bs = STEPS_PER_MM * page_size;
    // Position Table
    int pos_0 = 0;
    int pos_1 = bs * 0.5;
    int pos_2 = bs * 0.4;
    int pos_3 = bs * 0.5;
    int pos_4 = bs * 1.05;
    DEBUG_LOG("POS1", pos_1);
    DEBUG_LOG("POS2", pos_2);
    DEBUG_LOG("POS3", pos_3);
    DEBUG_LOG("POS4", pos_4);

    // Here we actually don't care about the track length.
    // We have a solid 0 point, all we care is that we abort if
    // We collide with the limit switch

    // assert that the drivers are off:
    if (drivers == true) {
        // if not error out here:
        return new_response(ERROR_INVALID_STATE);
    }

    // Enable the drivers
    head_pos = 0;
    set_drivers(true);

    //int p_amb = read_pressure_sensor();
    set_fan(true);
    set_vac_pump(true);
    move_to(pos_1);
    DEBUG_LOG("MOVED TO POSITION 1", 1);
    
    //int p_pickup = read_pressure_sensor();
    //if (p_pickup < p_amb) {
        set_fan(false);
        move_to(pos_2);
     DEBUG_LOG("MOVED TO POSITION 2", 1);
        move_to(pos_3);
     DEBUG_LOG("MOVED TO POSITION 3", 1);
              //  set_blow_pump(true);
        move_to(pos_4);

     DEBUG_LOG("MOVED TO POSITION 4", 1);
        set_vac_pump(false);
        move_to(pos_3);
     DEBUG_LOG("MOVED TO POSITION 3", 1);
        set_blow_pump(false);
        move_to(pos_0);
     DEBUG_LOG("MOVED TO POSITION 4", 1);
    //} else {
    //    set_vac_pump(false);
    //    set_blow_pump(false);
    //    set_fan(false);
    //}
    // this is dumb cos 0 moved by a few steps when the page turned
    move_to(pos_0);
    DEBUG_LOG("MOVED TO POSITION 0", 1);
    // disable the drivers, safing the box.
    set_drivers(false);
    return new_response(ERROR_OK);
}
