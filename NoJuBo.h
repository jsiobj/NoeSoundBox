/*
     Generic defines. Includes pin definitions.
*/

#ifndef NOESOUNDBOX_H
#define NOESOUNDBOX_H

//-----------------------------------------------------------------------------------
// VS1053 shield pins
//-----------------------------------------------------------------------------------
#define VS_SHIELD_MCS    36      // VS1053 chip select pin (output)
#define VS_SHIELD_DCS    39      // VS1053 Data/command select pin (output)
#define VS_SHIELD_CCS    37      // VS1053 shield SD card chip select pin
#define VS_SHIELD_DRQ    18      // VS1053 Data request (int pin)

#define BOOT_M0DE_PIN    47      // Boot mode / GPIO1 : if high => MIDI

#define VS1053_MODE_MIDI HIGH
#define VS1053_MODE_PLAYER LOW

//-----------------------------------------------------------------------------------
// Pin interrupt Accel
//-----------------------------------------------------------------------------------
// Not used yet

//-----------------------------------------------------------------------------------
// Mode & options selection pins for buttons & leds
//-----------------------------------------------------------------------------------
#define BTN_PLAYER     48
#define BTN_PIANO      34
#define BTN_TILT       17

#define LED_PLAYER     49
#define LED_PIANO      35
#define LED_TILT       16

//-----------------------------------------------------------------------------------
// Encoder
//-----------------------------------------------------------------------------------
#define ENCODER_A      43
#define ENCODER_B      41
#define ENCODER_BTN    40
#define ENCODER_RED    4
#define ENCODER_GREEN  13 
#define ENCODER_BLUE   12

//-----------------------------------------------------------------------------------
// Led states
//-----------------------------------------------------------------------------------
#define ON 0x0
#define OFF 0x1

//-----------------------------------------------------------------------------------
// Box modes
//-----------------------------------------------------------------------------------
#define BOX_MODE_UNDEF   0
#define BOX_MODE_PLAYER  1
#define BOX_MODE_PIANO   2
#define BOX_MODE_TILT    3
#define BOX_MODE_CHECK   9


//-----------------------------------------------------------------------------------
// Misc
//-----------------------------------------------------------------------------------

// Led bliking period
#define MODE_LED_BLINK       100 // Blinking period in ms for mode selection
#define OPTION_LED_BLINK     300 // Matrix leds blinking period for option selection

// Standard colors
#define COLOR_RED   0
#define COLOR_GREEN 1
#define COLOR_BLUE  2
#define COLOR_WHITE 6

#endif
