#include "bookscanner.h"

#include <Arduino.h>
#include <SPI.h>

#define STEPS 200
#define STEPS_PER_MM 34 //6.5
#define US_PER_STEP 2000

// Motor Directions
#define UP 1
#define DOWN 0

// 35V4 Pins
#define CSPIN 9
#define SLEEPPIN 8
#define FAULTPIN 3
#define STALLPIN 2

// Relay Pins
#define FAN 7
#define VAC_PUMP 4
#define PES_PUMP 5
#define LAMP 6

// Limit Switch
#define LIM_SW A0 // TBC

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
    motor(),
    bmp180()
{
}

void Bookscanner::begin() {
    drivers = false;
    pinMode(SLEEPPIN, OUTPUT);
    digitalWrite(SLEEPPIN, HIGH);
    SPI.begin();
    motor.setChipSelectPin(CSPIN);
    delay(1);
    motor.resetSettings();
    motor.clearStatus();

    // Select auto mixed decay.  TI's DRV8711 documentation recommends this mode
    // for most applications, and we find that it usually works well.
    motor.setDecayMode(HPSDDecayMode::AutoMixed);

    // Set the current limit. You should change the number here to an appropriate
    // value for your particular system.
    motor.setCurrentMilliamps36v4(3000);
    motor.setStepMode(HPSDStepMode::MicroStep1);
    
    //set_drivers(false);
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

    motor.enableDriver();
   motor.setDirection(UP);
    for(unsigned int x = 0; x < 1000; x++)
    {
      motor.step();
     delayMicroseconds(US_PER_STEP);
   }
    
}

bool Bookscanner::read_lim() {
    // Normally Open Switch to ground with pullup
    return digitalRead(LIM_SW);
}

Response Bookscanner::raise_box() {
    DEBUG_LOG("RAISING", 1);
    set_drivers(true);
    motor.setDirection(UP);
    while (!read_lim()) {
        motor.step();
        delayMicroseconds(US_PER_STEP); 
    }
    DEBUG_LOG("LIM_HIT", 1);
    head_pos = 32768; //Max 16Bit int
    set_drivers(false);
    return new_response(ERROR_OK);
}

Response Bookscanner::lower_box() {
    DEBUG_LOG("LOWERING", 1);
    //set_drivers(true);
    motor.setDirection(DOWN);
    while (digitalRead(STALLPIN)) {
        motor.step();
        delayMicroseconds(US_PER_STEP); 
    }
    //set_drivers(false);
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
    if (diff < 0) {
        motor.setDirection(DOWN);
        diff *= -1;
    } else {
        motor.setDirection(UP);
    }
    for (int i = 0; i < diff; i++) {
      motor.step();
      delayMicroseconds(US_PER_STEP); 
    }
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
    drivers = state;
    if (state == false) {
        motor.disableDriver();
        head_pos = 0;
    } else {
        motor.enableDriver();
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
    //set_drivers(true);

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
    //set_drivers(false);
    return new_response(ERROR_OK);
}
