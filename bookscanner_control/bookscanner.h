#ifndef LIBREFLIP_BOOKSCANNER_H
#define LIBREFLIP_BOOKSCANNER_H

#include <Arduino.h>
#include "SFE_BMP180.h"

class Bookscanner {
  public:
    Bookscanner();

    void begin();

    /// Set vacuum pump relay state
    void set_vacuum(bool state);

    /// Set page-separation ("flutter") fan relay state
    void set_fan(bool state);

    /// Set turn-blower ("positive pressure pump") relay state
    void set_blower(bool state);

    /// Set light relay state
    void set_light(bool state);

    /// Atomically de-energize vacuum, fan, and blower (light untouched)
    void all_off();

    /// Single-shot averaged pressure read (mbar) — accuracy-favoring
    float press_once();

    /// Single fast pressure sample (mbar) — rate-favoring, for streaming
    float press_stream_sample();

  private:
    SFE_BMP180 bmp180;

    float read_pressure_avg(char oversampling, int n);
};

void do_log(int line, const char *key, int val);

#define DEBUG_LOG(key, val) { do_log(__LINE__, key, val); }

#endif //LIBREFLIP_BOOKSCANNER_H
