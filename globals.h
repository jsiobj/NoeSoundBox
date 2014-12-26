/*
    Global definition in a header file so that it can be reused everywhere
*/

#ifndef GLOBALS_H
#define GLOBALS_H

#include <ClickEncoder.h>

extern byte standardColors[][3];

extern int VS1053Mode;
extern int lastVS1053Mode;

extern Keypad customKeypad;
extern rgbLedMatrix ledMatrix;
extern MMA8453_n0m1 accel;

extern Adafruit_VS1053_FilePlayer musicPlayer;
extern Adafruit_VS1053 VS1053;

extern ClickEncoder *encoder;
extern boolean SDStarted;

extern int boxMode;
extern int boxOption;
extern byte* validOptionList;
extern int validOptionCount;

#endif
