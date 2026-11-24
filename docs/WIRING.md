# Wiring — current NU40DK setup

| Module | Pin | NU40 |
|---|---|---|
| TFT | CS | P1.12 |
| TFT | DC | P0.03 |
| TFT | RST | P0.04 |
| Shared SPI | MOSI | P1.13 |
| Shared SPI | MISO | P1.14 |
| Shared SPI | SCK | P1.15 |
| Touch | CS | P1.03 |
| RC522 | SDA/SS | P0.26 |
| RC522 | RST | P0.27 |
| RC522 | VCC | 3.3V (never VBUS) |
| Play button | pin switched to GND | P1.05 |
| Volume + | pin switched to GND | P1.06 |
| Volume - | pin switched to GND | P1.07 |
| DFPlayer | RX via 1k series | P1.02 TX |
| DFPlayer | TX | P1.01 RX |
| DFPlayer | VCC | VBUS 5V in the existing USB-powered setup |
| Speaker | two wires | DFPlayer SPK1 / SPK2 |

All module grounds must connect to NU40 GND. TFT supply/backlight wiring must
follow the actual module's voltage specification; its definitive power wiring
was not preserved in earlier records. Battery charging/protection/regulation
has not been designed or verified here. Do not infer battery wiring from VBUS.
Touch calibration reads valid existing records and writes /lp-touch.bin.
Physical button wiring is normally-open to GND with internal pull-ups.
