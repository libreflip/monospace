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

    /// Set the RGB status LED. r/g/b are host-facing 0..255 values where
    /// 0 = channel off and 255 = channel fully on. The LED is a common-anode
    /// ring (cathodes on D9/D10/D11), so this inverts internally — see
    /// set_led()'s definition; the inversion never leaks onto the wire.
    void set_led(uint8_t r, uint8_t g, uint8_t b);

    /// Poll the debounced button. Returns true exactly once per physical
    /// press (the idle->pressed edge); release is not reported. Must be
    /// called every loop iteration to debounce correctly.
    bool poll_button();

  private:
    SFE_BMP180 bmp180;

    float read_pressure_avg(char oversampling, int n);

    // Button debounce state (see poll_button()).
    int button_last_reading;
    unsigned long button_last_edge_ms;
};

void do_log(int line, const char *key, int val);

#define DEBUG_LOG(key, val) { do_log(__LINE__, key, val); }

#endif //LIBREFLIP_BOOKSCANNER_H
