#include "bookscanner.h"

#include <Arduino.h>

/// Move Head to specified position.
/// Blocks while moving
void move_to(int pos) {
    
}

/// Presure Sensor Helper
int read_pressure_sensor() {
    
}

/// Flutter Fan Helper
void set_fan(bool state) {
  
}

/// Vaccum Pump Helper
void set_vac_pump(bool state) {
  
}

/// Positive Pressure Pump Helper
void set_blow_pump(bool state) {
  
}

/// Run a Complete Page flip cycle autonomously
/// @param page_size: Size of the page we are flipping in mm
Response flip_page(uint8_t page_size) {
    //TODO: Get starting Position
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
}

