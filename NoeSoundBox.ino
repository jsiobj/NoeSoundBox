
/*
  All In One test program
*/

#include <Keypad.h>           // For keypad management
#include <Wire.h>             // Needed for I2C
#include <I2C.h>              // I2C communications used by accelerometer
#include <MMA8453_n0m1.h>     // Accelerometer
#include <SPI.h>              // Needed for VS1053
#include <Adafruit_VS1053.h>  // VS1053 shield (music maker)
#include <SD.h>               // SD card library, where are stored music files
#include <avr/wdt.h>          // Watchdog library, used for soft resetting arduino
#include <Encoder.h>          // Volume knob


#include <InteractingObjects_ButtonPad.h>  // Button pad / led management library

#include "midi.h"     // Headers for Midi functions
#include "piano.h"    // Headers for piano mode
#include "player.h"   // Headers for player mode
#include "tilt.h"     // Headers for Tilt mode
#include "misc.h"     // Headers for misc. functions

#define ON 0x0        // LED status for matrix
#define OFF 0x1       // ibid.

#define VS1053_MODE_PIN    13    // Pin used to know whether in midi or player mode (replicate VS1053 GPIO 0 status
#define VS1053_MODE_MIDI   HIGH  // MIDI when pin is HIGH
#define VS1053_MODE_PLAYER LOW   // PLAYER if LOW

#define VS_SHIELD_MCS    36      // VS1053 chip select pin (output)
#define VS_SHIELD_DCS    38      // VS1053 Data/command select pin (output)
#define VS_SHIELD_CCS    40      // VS1053 shield SD card chip select pin
#define VS_SHIELD_DRQ    18      // VS1053 Data request (int pin)

// Pin interrupt Accel

// Encoder
#define ENCODER_A 48
#define ENCODER_B 49
#define ENCODER_BUTTON 51   

// Control leds
byte pinLedBlue   = 41;
byte pinLedGreen  = 39;
byte pinLedYellow = 37;

// Standard colors "helper"
#define COLOR_RED   0
#define COLOR_GREEN 1
#define COLOR_BLUE  2
#define COLOR_WHITE 6

byte standardColors[][3] =  {
  {255, 0  , 0  },
  {0  , 255, 0  },
  {0  , 0  , 255},
  {255, 255, 0  },
  {255, 0  , 255},
  {0  , 255, 255},
  {255, 255, 255},
  {150, 12 , 210}
};

int VS1053Mode;        // Stores current mode (MIDI or PLAYER)
int lastVS1053Mode;    // Stores last mode, to detect when it changes
int selectedOption;
long volKnobOldPos  = -999;

// Key mapping for keypad library. Key hexa number
char hexaKeys[ROWS][COLS] = {
  {'0', '1', '2', '3'},
  {'4', '5', '6', '7'},
  {'8', '9', 'A', 'B'},
  {'C', 'D', 'E', 'F'}
};

// Button & Led Matrix pins
byte pinBtn[ROWS]       = {22, 23, 24, 25}; // Buttons rows
byte pinBtnGnd[COLS]    = {26, 27, 28, 29}; // Buttons cols
//byte customPinLedRGB[4][3] = {{2, 3, 44}, {5,6,7}, {8,9,10}, {11,12,45}};
byte customPinLedRGB[4][3] = {{2,3,5}, {6,7,8}, {9,10,11}, {44,45,46}};
byte customPinLedGnd[4]    = {30, 31, 32, 33}; // LEDs cols

// Creation of needed objects
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), pinBtn, pinBtnGnd, ROWS, COLS);    // Keypad
rgbLedMatrix ledMatrix = rgbLedMatrix(customPinLedRGB, customPinLedGnd, ROWS, COLS);  // LED matrix
MMA8453_n0m1 accel;                                                                   // Accelerometer

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(VS_SHIELD_MCS,VS_SHIELD_DCS,VS_SHIELD_DRQ,VS_SHIELD_CCS);  // Player
Adafruit_VS1053 VS1053 = Adafruit_VS1053(-1,VS_SHIELD_MCS,VS_SHIELD_DCS,VS_SHIELD_DRQ);  // VS1053

Encoder myEnc(ENCODER_A, ENCODER_B);

//=================================================================================
// SETUP
//=================================================================================
void setup() {

  // Status LEDs settings and testing
  pinMode(pinLedBlue, OUTPUT);
  pinMode(pinLedGreen, OUTPUT);
  pinMode(pinLedYellow, OUTPUT);

  digitalWrite(pinLedBlue,HIGH);
  digitalWrite(pinLedGreen,HIGH);
  digitalWrite(pinLedYellow,HIGH);
  delay(300);
  digitalWrite(pinLedBlue,LOW);
  digitalWrite(pinLedGreen,LOW);
  
  pinMode(ENCODER_BUTTON,INPUT);
  digitalWrite(ENCODER_BUTTON,HIGH);  // Enabling pullup resistor
  
  Serial.begin(115200);

  setPWMFreq();                     // Setting high PWM freq.
    
  // Quickly check pad LEDs
  ledMatrix.ledTestAll(standardColors[COLOR_WHITE]);
  //ledMatrix.ledTestMatrix();

  // Setting accellerator
  accel.setI2CAddr(0x1D);
  accel.dataMode(true, 2); //enable highRes 10bit, 2g range [2g,4g,8g]

  // Starting basic VS1053 features
  VS1053.begin();
  
  // Reading pin saying saying if MIDI or Player mode and setting mode
  pinMode(VS1053_MODE_PIN,INPUT);
  VS1053Mode=digitalRead(VS1053_MODE_PIN);
  lastVS1053Mode=VS1053Mode;

  digitalWrite(pinLedYellow,LOW);

  // Let the user select an option (one out of 16)
  digitalWrite(pinLedBlue,HIGH);
  selectedOption=selectOption();
  digitalWrite(pinLedBlue,LOW);
  
  // Initialising VS1053 depending on selected mode (MIDI or Player)
  if(VS1053Mode==VS1053_MODE_MIDI) {
    if(selectedOption == 15 || selectedOption == 14) initTilt();
    else initPiano(selectedOption);
  }
  else initPlayer(selectedOption);

  digitalWrite(pinLedGreen,HIGH);
}

//=================================================================================
// LOOP
//=================================================================================
void loop() {

  // Reading mode in case user changed the switch...
  // And if he changed it, resetting the arduino
  VS1053Mode=digitalRead(VS1053_MODE_PIN);  
  if(VS1053Mode!=lastVS1053Mode) {
    Serial.println(F("Changing mode. Resetting !"));
    resetArduino();
  }

  long volKnobNewPos = myEnc.read();
  if (volKnobNewPos != volKnobOldPos) {
    volKnobOldPos = volKnobNewPos;
    Serial.print("Knob:"); Serial.println(volKnobNewPos);
  }

  if(digitalRead(ENCODER_BUTTON)==LOW) {
    Serial.println("Button was pressed");
  }

  // Now, starting the right loop, depending on mode
  if(VS1053Mode==VS1053_MODE_MIDI) {
    switch(selectedOption) {
      case 15: loopTilt(false); break;
      case 14: loopTilt(true); break;
      default: loopPiano(); break;
    }
  }

  if(VS1053Mode==VS1053_MODE_PLAYER) loopPlayer(selectedOption);
  
}

//-------------------------------------------------------------------------------
// At startup, using the keypad to let the user choose one key (among the 16)
// so that we can select different options.
// For MIDI mode, an option will change the intrument and note mapping
// For PLAYER mode, an option will change the music "bank" to play from
//-------------------------------------------------------------------------------
int selectOption() {
  
  int selectedOption=-1;
  
  Serial.println("Getting user input...");
  
  while(!customKeypad.getKeys() || selectedOption==-1){
    for (int i=0; i<LIST_MAX; i++)  {
      if ( customKeypad.key[i].stateChanged ) {
        switch (customKeypad.key[i].kstate) {
          case PRESSED:
            ledMatrix.ledSetState(customKeypad.key[i].kcode/4,customKeypad.key[i].kcode%4,standardColors[COLOR_WHITE]);
            break;
          
          case HOLD:
            break;
          
          case RELEASED:
            delay(500);
            ledMatrix.ledSetOff(customKeypad.key[i].kcode/4,customKeypad.key[i].kcode%4);
            selectedOption = customKeypad.key[i].kcode;
            break;
          
          case IDLE:
            break;
        }
      }
    }
  }
  
  Serial.print("User selected option "); Serial.println(selectedOption);
  return selectedOption;
}
