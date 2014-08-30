/*
    Global definition in a header file so that it can be reused everywhere
*/

#ifndef GLOBALS_H
#define GLOBALS_H

#define ON 0x0
#define OFF 0x1

#define VS1053_MODE_PIN  13

#define VS_SHIELD_MCS    36      // VS1053 chip select pin (output)
#define VS_SHIELD_DCS    38      // VS1053 Data/command select pin (output)
#define VS_SHIELD_CCS    40      // Card chip select pin
#define VS_SHIELD_DRQ    18      // VS1053 Data request, DREQ should be an Int pin

#define VS1053_MODE_MIDI HIGH
#define VS1053_MODE_PLAYER LOW

// Standard colors
#define COLOR_RED   0
#define COLOR_GREEN 1
#define COLOR_BLUE  2
#define COLOR_WHITE 6

// Control led
extern byte pinLedBlue;
extern byte pinLedGreen;
extern byte pinLedYellow;

//#define COLORS 7
extern byte standardColors[][3];

extern int VS1053Mode;
extern int lastVS1053Mode;

extern Keypad customKeypad;
extern rgbLedMatrix ledMatrix;
extern MMA8453_n0m1 accel;

extern Adafruit_VS1053_FilePlayer musicPlayer;
extern Adafruit_VS1053 VS1053;

#endif
