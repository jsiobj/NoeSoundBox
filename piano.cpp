/*
    PIANO mode related code.
*/
#define DEBUG
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
byte pianoOptionList[]={0,1,2,3,4,5,6,7,8,9}; // List of available button options for Piano mode, end with 255

// Channel mapping : which bank / instrument on which channel (up to 4 channels)
// Looks like 
// - DEFAUT and MELODY are the same...
// - DRUM1 & 2 are also the same...
// - When DRUMS, notes select instrument

int midiChannelMaps[MIDI_MAX_CHANNEL][2] = {
  /* Channel 0 */ {VS1053_BANK_MELODY, VS1053_GM1_ELECTRIC_GUITAR_JAZZ},
  /* Channel 1 */ {VS1053_BANK_DEFAULT, VS1053_GM1_ACOUSTIC_GRAND_PIANO},
  /* Channel 2 */ {VS1053_BANK_DRUMS1, VS1053_GM1_GM2_BASSDRUM1},
  /* Channel 3 */ {VS1053_BANK_DRUMS2, VS1053_GM1_GM2_TAMBOURINE},
  /* Channel 4 */ {VS1053_BANK_DEFAULT, VS1053_GM1_TELEPHONE_RING}
};
  
// Midi mappings for each key, 1st number is channel, 2nd number is note
int midiMaps[][16][2] =  {
  
  // 0 : Simple notes on channel 1
  { {0,72},{0,74},{0,76},{0,77},
    {0,79},{0,81},{0,83},{0,84},
    {0,86},{0,88},{0,89},{0,91},
    {0,93},{0,95},{0,96},{0,98}
  },

  // 1 : Simple notes on channel 1
  { {1,72},{1,74},{1,76},{1,77},
    {1,79},{1,81},{1,83},{1,84},
    {1,86},{1,88},{1,89},{1,91},
    {1,93},{1,95},{1,96},{1,98}
  },

  // 2 : Simple notes on channel 2
  { {2,72},{2,74},{2,76},{2,77},
    {2,79},{2,81},{2,83},{2,84},
    {2,86},{2,88},{2,89},{2,91},
    {2,93},{2,95},{2,96},{2,98}
  },

  // 3 : Simple notes on channel 3
  { {3,72},{3,74},{3,76},{3,77},
    {3,79},{3,81},{3,83},{3,84},
    {3,86},{3,88},{3,89},{3,91},
    {3,93},{3,95},{3,96},{3,98}
  },

  // 4 : Simple notes on channel 4
  { {4,72},{4,74},{4,76},{4,77},
    {4,79},{4,81},{4,83},{4,84},
    {4,86},{4,88},{4,89},{4,91},
    {4,93},{4,95},{4,96},{4,98}
  },

  // 5 : Drums on 1st row, then Piano
  { {2,81},{2,57},{2,75},{2,72},
    {1,72},{1,74},{1,76},{1,77},
    {1,79},{1,81},{1,83},{1,84},
    {1,86},{1,88},{1,89},{1,91}
  },

  // 6 : Drums only
  { {2,27},{2,28},{2,29},{2,30},
    {2,31},{2,32},{2,33},{2,34},
    {2,35},{2,36},{2,37},{2,38},
    {2,39},{2,40},{2,41},{2,42}
  },

  // 7 : Drums only
  { {2,43},{2,44},{2,45},{2,46},
    {2,47},{2,48},{2,49},{2,50},
    {2,51},{2,52},{2,53},{2,54},
    {2,55},{2,56},{2,57},{2,58}
  },
  
  // 8 : Drums only
  { {2,59},{2,60},{2,61},{2,62},
    {2,63},{2,64},{2,65},{2,66},
    {2,67},{2,68},{2,69},{2,70},
    {2,71},{2,72},{2,73},{2,74}
  },

  // 9 : Drums only
  { {2,75},{2,76},{2,77},{2,78},
    {2,79},{2,80},{2,81},{2,82},
    {2,83},{2,84},{2,85},{2,86},
    {2,87},{2,27},{2,27},{2,27}
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
    
    for(int i=0;i<MIDI_MAX_CHANNEL;i++) {
      Serial.print("Channel:");Serial.print(i);
      Serial.print("|Bank:");Serial.print(midiChannelMaps[i][0]);
      Serial.print("|Instrument:");Serial.println(midiChannelMaps[i][1]);
    
      midiSetChannelBank(i, midiChannelMaps[i][0]);
      midiSetInstrument(i, midiChannelMaps[i][1]);
    }

    midiCurrentMap=option;
    
    // Play a few notes, just check
    // testMidi();

}

//-------------------------------------------------------------------------
void onKeyPressedPiano(int keyCode) {
  byte row=keyCode/4;
  byte col=keyCode%4;

  Serial.print("Map:"); Serial.print(midiCurrentMap);Serial.print("|Key:");Serial.print(keyCode);
  Serial.print("|Bank:");Serial.print(midiMaps[midiCurrentMap][keyCode][0]);
  Serial.print("|Note:");Serial.println(midiMaps[midiCurrentMap][keyCode][1]);
  
  ledMatrix.matrixLedSetRandom(row,col);  
  midiNoteOn(midiMaps[midiCurrentMap][keyCode][0], midiMaps[midiCurrentMap][keyCode][1], pianoVolume);
}

//-------------------------------------------------------------------------
void onKeyHoldPiano(int keyCode) {
  byte row=keyCode/4;
  byte col=keyCode%4;

  if(ledMatrix.matrixLedGetState(row,col)==ON) ledMatrix.matrixLedLock(row,col);  

}

//-------------------------------------------------------------------------
void onKeyReleasedPiano(int keyCode) {
  byte row=keyCode/4;
  byte col=keyCode%4;

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
      //volKnobOldPos = volKnobNewPos;
      // TODO : set master volume
      for(int i=0;i<MIDI_MAX_CHANNEL;i++) midiSetChannelVolume(i, pianoVolume);
      DEBUG_PRINTF("Volume set to %d",pianoVolume);
      
      int red,green,blue;
      
      if(pianoVolume>64) {
        red=256-(pianoVolume-64)*4;
        green=(pianoVolume-64)*4;
        blue=255;
      }
      else {
        red=255;
        green=(64-pianoVolume)*4;
        blue=256-(64-pianoVolume)*4;
      }
      DEBUG_PRINTF3("RGB:%3d,%3d,%3d",red,green,blue);
      analogWrite(ENCODER_RED,red);
      analogWrite(ENCODER_GREEN,green);
      analogWrite(ENCODER_BLUE,blue);
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
