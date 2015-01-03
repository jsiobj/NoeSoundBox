/*
    PIANO mode related code.
*/
//#define DEBUG
#include "debug.h"

#include <Keypad.h>
#include <Wire.h> 
#include <I2C.h>
#include <MMA8453_n0m1.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <avr/wdt.h>
#include <ClickEncoder>
#include <TimerOne.h>

#include <InteractingObjects_ButtonPad.h>

#include "NoJuBo.h"
#include "midi.h"
#include "globals.h"
#include "misc.h"

int midiCurrentMap=-1;
int pianoVolume=127;
byte pianoOptionList[]={0,1,3,8,12}; // List of available button options for Piano mode, end with 255

// Channel mapping : which bank / instrument on which channel (up to 4 channels)
// Looks like 
// - DEFAUT and MELODY are the same...
// - DRUM1 & 2 are also the same...
// - When DRUMS, notes select instrument

int midiChannelMaps[MIDI_MAX_CHANNEL][2] = {
  /* Channel 0 */ {VS1053_BANK_DEFAULT, VS1053_GM1_ACOUSTIC_GUITAR_NYLON},
  /* Channel 1 */ {VS1053_BANK_DEFAULT, VS1053_GM1_ACOUSTIC_GRAND_PIANO},
  /* Channel 2 */ {VS1053_BANK_DRUMS1, VS1053_GM1_GM2_BASSDRUM1},
  /* Channel 3 */ {VS1053_BANK_DRUMS2, VS1053_GM1_GM2_TAMBOURINE},
  /* Channel 4 */ {VS1053_BANK_DEFAULT, VS1053_GM1_TELEPHONE_RING},
  /* Channel 5 */ {VS1053_BANK_DEFAULT, VS1053_GM1_ELECTRIC_BASS_FINGER},
  /* Channel 6 */ {VS1053_BANK_DEFAULT, VS1053_GM1_ACOUSTIC_BASS}
};
  
// Midi mappings for each key, 1st number is channel, 2nd number is note
int midiMaps[][16][2] =  {
  
  // 0 Acoustic Guitar
  { {0,77},{0,76},{0,74},{0,72},
    {0,84},{0,83},{0,81},{0,79},
    {0,91},{0,89},{0,88},{0,86},
    {0,98},{0,96},{0,95},{0,93}
  },

  // 1 Grand Piano
  { {1,77},{1,76},{1,74},{1,72},
    {1,84},{1,83},{1,81},{1,79},
    {1,91},{1,89},{1,88},{1,86},
    {1,98},{1,96},{1,95},{1,93}
  },

  // 2 ...
  {
  },
  
  // 3 Drums on 1st row, then Piano
  { {2,81},{2,57},{2,75},{2,72},
    {0,77},{0,76},{0,74},{0,72},
    {0,84},{0,83},{0,81},{0,79},
    {0,91},{0,89},{0,88},{0,86}
  },

  // 4 ...
  {
  },

  // 5 ...
  {
  },

  // 6 ...
  {
  },

  // 7 ...
  {
  },

  // 8 Drums only
  { {2,61},{2,60},{2,57},{2,49},
    {2,66},{2,65},{2,68},{2,67},
    {2,30},{2,29},{2,55},{2,52},
    {2,58},{2,34},{2,27},{2,74}
  },

  // 9 ...
  {
  },

  // 10 ...
  {
  },

  // 11 ...
  {
  },

  // 12 Comparing ACOUSTIC_GRAND_PIANO ASSDRUM1 ELECTRIC_BASS_FINGER ACOUSTIC_BASS
  { {0,77},{0,76},{0,74},{0,72},
    {1,77},{1,76},{1,74},{1,72},
    {5,77},{5,76},{5,74},{5,72},
    {6,77},{6,76},{6,74},{6,72}
  }

};

byte* pianoGetOptionList(int* size) {
  DEBUG_PRINT_ARRAY(pianoOptionList,"pianoOptionList",ARRAY_LENGTH(pianoOptionList));
  *size=ARRAY_LENGTH(pianoOptionList);
  return pianoOptionList;
}

void initPiano(int option) {
    
    DEBUG_PRINT("Setting VS1053 in MIDI mode");
    digitalWrite(LED_PIANO,HIGH);
    digitalWrite(BOOT_M0DE_PIN,HIGH); // Setting pin connected to GPIO1 and VS1053 to HIGH...
    VS1053.reset();                   // ... and resetting

    Serial3.begin(31250);
    midiSetChannelVolume(0, pianoVolume);

    analogWrite(ENCODER_RED,pianoVolume*2);
    analogWrite(ENCODER_GREEN,pianoVolume*2);
    analogWrite(ENCODER_BLUE,pianoVolume*2);
    
    for(int i=0;i<MIDI_MAX_CHANNEL;i++) {
      DEBUG_PRINTF3("Channel:%d|Bank:%d|Instrument:%d",i,midiChannelMaps[i][0],midiChannelMaps[i][1])
      midiSetChannelBank(i, midiChannelMaps[i][0]);
      midiSetInstrument(i, midiChannelMaps[i][1]);
    }

    midiCurrentMap=option;
    
    ledMatrix.matrixLedSetAllRandom();
    
    // Play a few notes, just check
    // testMidi();

}

//-------------------------------------------------------------------------
void onKeyPressedPiano(int keyCode) {
  byte row=KEY2ROW(keyCode);
  byte col=KEY2COL(keyCode);

  DEBUG_PRINTF3("-- Key:%2d| row:%2d| col:%2d",keyCode,row,col);
  //DEBUG_PRINTF2("   Addr2:  div:%2d|mod:%d",keyCode/ROWS,keyCode%COLS);
  //DEBUG_PRINTF2("   Addr3:  div:%2d|mod:%d",KEY2ROW(keyCode),KEY2COL(keyCode));
  DEBUG_PRINTF2("   Addr:   row:%2d|col:%d",ROW2ADDR(row),COL2ADDR(col));
  DEBUG_PRINTF3("   Map:%d|Bank:%d|Note:%d",midiCurrentMap,midiMaps[midiCurrentMap][keyCode][0],midiMaps[midiCurrentMap][keyCode][1]); 
  
  ledMatrix.matrixLedSetRandom(row,col);  
  midiNoteOn(midiMaps[midiCurrentMap][keyCode][0], midiMaps[midiCurrentMap][keyCode][1], pianoVolume);
}

//-------------------------------------------------------------------------
void onKeyHoldPiano(int keyCode) {
  byte row=KEY2ROW(keyCode);
  byte col=KEY2COL(keyCode);

  if(ledMatrix.matrixLedGetState(row,col)==ON) ledMatrix.matrixLedLock(row,col);  

}

//-------------------------------------------------------------------------
void onKeyReleasedPiano(int keyCode) {
  byte row=KEY2ROW(keyCode);
  byte col=KEY2COL(keyCode);

  //midiNoteOff(midiMaps[midiCurrentMap][keyCode][0], midiMaps[midiCurrentMap][keyCode][1], pianoVolume);
  
  if(ledMatrix.matrixLedGetLockState(row,col)==OFF && ledMatrix.matrixLedGetState(row,col)==ON) {
    ledMatrix.matrixLedToggleState(row,col,standardColors[4]); 
  }
}

//=================================================================================
// What to do in MIDI mode
//=================================================================================
void loopPiano() {

  long lastToggle;
//  int volKnobOldPos=0;
//  boolean restart=false;

  DEBUG_PRINTF("Volume:%d",pianoVolume);
  
  while(1) {
    
    // Controlling volume  
    long volKnobMove = encoder->getValue();
    if (volKnobMove) {
      pianoVolume=pianoVolume+volKnobMove;
      if(pianoVolume<0) pianoVolume=0; if(pianoVolume>127) pianoVolume=127;
      for(int i=0;i<MIDI_MAX_CHANNEL;i++) midiSetChannelVolume(i, pianoVolume);
      DEBUG_PRINTF("Volume set to %d",pianoVolume);
      
//      int red,green,blue;
      
      analogWrite(ENCODER_RED,pianoVolume*2);
      analogWrite(ENCODER_GREEN,pianoVolume*2);
      analogWrite(ENCODER_BLUE,pianoVolume*2);

//      if(pianoVolume>64) {
//        red=256-(pianoVolume-64)*4;
//        green=(pianoVolume-64)*4;
//        blue=255;
//      }
//      else {
//        red=255;
//        green=(64-pianoVolume)*4;
//        blue=256-(64-pianoVolume)*4;
//      }
      //DEBUG_PRINTF3("RGB:%3d,%3d,%3d",red,green,blue);
      //analogWrite(ENCODER_RED,red);
      //analogWrite(ENCODER_GREEN,green);
      //analogWrite(ENCODER_BLUE,blue);
    }      

    // If player button pressed, pausing...
    if(digitalRead(BTN_PIANO)==LOW) {
      while(digitalRead(BTN_PIANO)==LOW) 1;
      DEBUG_PRINT("Restarting piano");
      boxOption=-1;
      break;
    }
  
    // At least one key state changed i.e. a key was pressed or released
    if (customKeypad.getKeys()){
      
      for (int i=0; i<LIST_MAX; i++)  {
          if ( customKeypad.key[i].stateChanged ) {
  
            switch (customKeypad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                  case PRESSED:
                    onKeyPressedPiano(customKeypad.key[i].kcode);
                    break;
                  
                  case HOLD:
                    onKeyHoldPiano(customKeypad.key[i].kcode);
                    break;
                  
                  case RELEASED:
                    onKeyReleasedPiano(customKeypad.key[i].kcode);
                    break;
                  
                  case IDLE:
                    break;
            }
          }
       }
    }
  
    ledMatrix.matrixLedRefresh();
  }
 }
