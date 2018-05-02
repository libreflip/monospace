#include "bookscanner.h"

#include <Arduino.h>

#define STEPS 3200
#define STEPS_PER_MM 1

// Motor Shield pins
#define DIR_A 12
#define DIR_B 13
#define PWM_A 3
#define PWM_B 11
#define BRK_A 9
#define BRK_B 8

// Relay Pins
#define FAN 1 // TBC
#define VAC_PUMP 2 // TBC
#define PES_PUMP 4 // TBC
#define LAMP 5 // TBC

// Limit Switch
#define LIM_SW 10 // TBC

Bookscanner::Bookscanner():
    motor(STEPS, DIR_A, DIR_B)
    {
    // Set up extended Stepper control
    pinMode(PWM_A, OUTPUT);
    pinMode(PWM_B, OUTPUT);
    pinMode(BRK_A, OUTPUT);
    pinMode(BRK_B, OUTPUT);
    // Brakes off; Drivers off%
    digitalWrite(BRK_A, 0);
    digitalWrite(BRK_B, 0);
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
    digitalWrite(FAN, 0);
    digitalWrite(VAC_PUMP, 0);
    digitalWrite(PES_PUMP, 0);
    digitalWrite(LAMP, 0);
    // Box is set up
}

Bookscanner::~Bookscanner() {
  
}

Bookscanner::raise_box() {
    set_drivers(true);
    while (!read_lim()) {
        motor.step(1);
    }
    head_pos = 32768 //Max 16Bit int
}

Bookscanner::lower_box() {
    set_drivers(false);
    head_pos = 0;
}

/// Move Head to specified position.
/// Blocks while moving
bool Bookscanner::move_to(int pos) {
    int diff = pos - head_pos;
    motor.step(diff);
    head_pos = pos;
}

/// Presure Sensor Helper
int read_pressure_sensor() {
    
}

/// Flutter Fan Helper
void set_fan(bool state) {
    digitalWrite(FAN, state);
}

/// Vaccum Pump Helper
void set_vac_pump(bool state) {
    digitalWrite(VAC_PUMP, state);
}

/// Positive Pressure Pump Helper
void set_blow_pump(bool state) {
    digitalWrite(PES_PUMP, state);
}

void Bookscanner::set_drivers(bool state) {
    digitalWrite(PWM_A, state);
    digitalWrite(PWM_A, state);
}

/// Run a Complete Page flip cycle autonomously
/// @param page_size: Size of the page we are flipping in mm
Response Bookscanner::flip_page(uint8_t page_size) {
    int bs = STEPS_PER_MM * page_size;
    // Position Table
    int pos_0 = 0;
    int pos_1 = bs * 0.5;
    int pos_2 = bs * 0.4;
    int pos_3 = bs * 0.8;
    int pos_4 = bs * 1.05;

    // Here we actually don't care about the track length.
    // We have a solid 0 point, all we care is that we abort if
    // We collide with the limit switch

    // TODO: make sure the box is at the bottom first

    // Enable the drivers
    set_drivers(true);
    
    int p_amb = read_pressure_sensor();
    set_fan(true);
    set_vac_pump(true);
    move_to(pos_1);
    int p_pickup = read_pressure_sensor();
    if (p_pickup < p_amb) {
        set_fan(false);
        move_to(pos_2);
        move_to(pos_3);
        set_blow_pump(true);
        move_to(pos_4);
        set_vac_pump(false);
        move_to(pos_3);
        set_blow_pump(false);
        move_to(pos_0);
    } else {
        set_vac_pump(false);
        set_blow_pump(false);
        set_fan(false);
        
    }
    // this is dumb cos 0 moved by a few steps when the page turned
    move_to(pos_0);
    // disable the drivers, safing the box.
    set_drivers(false);
}

