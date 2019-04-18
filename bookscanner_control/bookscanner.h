#ifndef bookscanner_h
#define bookscanner_h
#include <Arduino.h>
#include <Stepper.h>
#include "SFE_BMP180.h"

typedef char Error;
#define ERROR_OK 0
#define ERROR_INVALID_STATE 255

typedef struct {
    Error error;
    char payload_len;
    char payload[16];
} Response;

class Bookscanner {
  public:
    Bookscanner();

    void begin();

    /// Raises box to maximum position
    Response raise_box();

    /// Lowers box to minimum position and recalibrate 0 position.
    Response lower_box();

    /// Set box lighting
    /// @param state: new lighting state,
    Response set_lights(bool state);

    /// Run a Complete Page flip cycle autonomously
    /// @param page_size: Size of the page we are flipping in mm
    Response flip_page(uint8_t page_size);
  private:
    Stepper motor;
    SFE_BMP180 bmp180;

    // on an AVR these are 16Bit Values
    int head_pos; // Position of the head in steps from the book
    bool drivers; // current driver state

    bool move_to(int pos);
    void set_drivers(bool state);
    bool read_lim();
    double read_pressure_sensor();
    Response new_response(Error code);
};

void do_log(int line, const char *key, int val);

#define DEBUG_LOG(key, val) {}
//#ifdef DEBUG
#define DEBUG_LOG(key, val) { do_log(__LINE__, key, val); }
//#endif

#endif
