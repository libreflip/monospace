# monospace

Arduino Uno firmware controlling the Libreflip bookscanner's relay board
(vacuum pump, page-separation fan, turn blower, light) and reading its
BMP180 air-pressure sensor. It's deliberately "dumb": it never decides
*when* to switch anything on — it receives one text command per line over
USB-serial from the Raspberry Pi, executes it immediately, and replies. All
process logic (when to engage vacuum, when a page has separated, when to
abort) lives on the Raspberry Pi side (`sans` repo).

**Not this board's concern:** the suction-box motor/lift mechanism and its
limit switch — that's a separate ESP32 FOC board, a different firmware
project entirely.

## Requirements

- Arduino Uno
- [`arduino-cli`](https://arduino.github.io/arduino-cli/) with the
  `arduino:avr` core installed
- The `pi`/host user in the `dialout` group (or root) for serial access

## Building & flashing

```sh
arduino-cli core install arduino:avr   # once
arduino-cli board list                 # identify the port - a second serial
                                        # device (the ESP32 FOC board) is
                                        # often attached too, don't guess
arduino-cli compile --fqbn arduino:avr:uno bookscanner_control
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno bookscanner_control
```

`SFE_BMP180.cpp`/`.h` (the vendored SparkFun BMP180 driver, "beerware"
license) live directly in `bookscanner_control/` next to the main `.ino` and
compile in automatically as part of the sketch — no separate library install
needed.

## Protocol

Text-based, one command per line, **115200 baud**, `\n`-terminated (an
optional preceding `\r` is tolerated), ASCII, **case-sensitive uppercase
commands only**. Every command gets exactly one reply line — `OK` on
success, `ERR <reason>` on failure — except while pressure streaming is
active, when unsolicited `PRESS <mbar>` lines are also emitted (told apart
by the `PRESS ` prefix; every other line is still the reply to whichever
command preceded it).

| Command | Reply | Effect |
|---|---|---|
| `VACUUM ON` / `VACUUM OFF` | `OK` | Energize/de-energize the vacuum pump relay |
| `FAN ON` / `FAN OFF` | `OK` | Energize/de-energize the page-separation fan relay |
| `BLOWER ON` / `BLOWER OFF` | `OK` | Energize/de-energize the turn-blower relay |
| `LIGHT ON` / `LIGHT OFF` | `OK` | Energize/de-energize the light relay |
| `ALL OFF` | `OK` | Atomically de-energizes vacuum, fan, and blower in one operation (light untouched) |
| `PRESS?` | `OK <mbar>` | Single-shot averaged pressure read (oversampling=3, 8 samples — favors accuracy) |
| `PRESS START` | `OK`, then unsolicited `PRESS <mbar>` lines | Begin continuous pressure streaming (oversampling=2, ~49Hz measured) |
| `PRESS STOP` | `OK` | Stop streaming |

Any other line: `ERR UNKNOWN_COMMAND`.

There is **no state-query command** — the host is expected to track what it
last commanded itself. The recovery pattern for reconnecting to a board that
might already be in some state from a previous session is to send `ALL OFF`
immediately after opening the connection, not to query anything.

## Safety notes

- All four relays are **active-low** (`digitalWrite(pin, LOW)` energizes).
  All four are forced HIGH (off) in `setup()`, before the serial interface
  starts accepting commands — with one accepted exception: the light can
  briefly glitch on during the Uno's own bootloader reset window, a harmless
  hardware quirk, not a bug.
- No automatic behavior of any kind: this firmware never times an actuator
  or sequences multiple actuators on its own — that's entirely the host's
  job. It also never times out on its own if the host disappears; the last
  commanded state persists until told otherwise.
- The AVR watchdog (8s timeout) is armed *before* sensor init, specifically
  because the BMP180 driver's I2C read has an unbounded wait loop with no
  timeout of its own — a watchdog reset re-runs the normal boot sequence,
  forcing everything off again.

## Testing

Flash, then sanity-check by hand with `arduino-cli monitor -p /dev/ttyACM0 -c
baudrate=115200` and typing a command. For anything more thorough — a
persistent interactive session, streamed pressure logging to CSV — use
`hw_diag` from the `sans` repo
([`sans-core/README.md`](https://github.com/libreflip/sans/blob/master/sans-core/README.md)),
which also has real measured pressure-drop test results under
`sans-core/test-results/`.

`arduinofucker/arduinofucker.py` is an older interactive Python test tool
predating `hw_diag` (built for the previous binary protocol) — kept as a
documented fallback, not the primary path.
