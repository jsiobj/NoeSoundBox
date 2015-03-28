/*
  Interacting Objects - Noe's Juke Box Project
  http://www.interactingobjects.com

  Project page : http://interactingobjects.com/category/projects/juke-box/

  ---------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2014 Interacting Objects (http://interactingobjects.com)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  ---------------------------------------------------------------------------

  Tilt mode related code.
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

#include <InteractingObjects_ButtonPad.h>

#include "NoJuBo.h"
#include "misc.h"
#include "globals.h"
#include "midi.h"

byte tiltOptionList[]={0,1}; // List of available button options for Piano mode, end with 255

int tiltVolume=127;

byte key,previousKey=32;
byte row=32,col=32,rowPrevious=32,colPrevious=32;
long int lastTimeMoved;
const int waitBetweenMove=100; // in ms

byte* tiltGetOptionList(int* size) {
  DEBUG_PRINT_ARRAY(tiltOptionList,"tiltOptionList",ARRAY_LENGTH(tiltOptionList));
  *size=ARRAY_LENGTH(tiltOptionList);
  return tiltOptionList;
}

void initTilt() {
    Serial.println(F("Starting TILT Mode"));
    digitalWrite(LED_TILT,HIGH);
    digitalWrite(BOOT_M0DE_PIN,HIGH); // Setting pin connected to GPIO1 and VS1053 to HIGH...
    Serial3.begin(31250);

    VS1053.reset();                   // ... and resetting

    midiSetChannelBank(0, VS1053_BANK_DRUMS1);
    midiSetInstrument(0, VS1053_BANK_DRUMS1);
    midiSetChannelVolume(0, tiltVolume);
}

//=================================================================================
// What to do in TILT mode
//=================================================================================
void loopTilt(bool tiltLoop) {
  
  //int tiltVolume;
  
  while(1) {

    // If player button pressed, restarting player...
    if(digitalRead(BTN_TILT)==LOW) {
      while(digitalRead(BTN_TILT)==LOW) 1;
      DEBUG_PRINT("Restarting tilt");
      boxOption=-1;
      break;
    }
  
    // Controlling volume  
    long volKnobMove = encoder->getValue();
    if (volKnobMove) {
      tiltVolume=tiltVolume+volKnobMove;
      if(tiltVolume<0) tiltVolume=0; if(tiltVolume>127) tiltVolume=127;
      midiSetChannelVolume(0, tiltVolume);
      DEBUG_PRINTF("Volume set to %d",tiltVolume);
    }

    int x,y;
  
    // If we did not move for a while, update accel data
    if(lastTimeMoved<millis()-waitBetweenMove) {
      accel.update();
      x=accel.x();y=accel.y();
    }
  
    if(row<32 && col<32 && lastTimeMoved<millis()-waitBetweenMove) {
  
      boolean moved=false;  // Not moved yet
      rowPrevious=row; colPrevious=col;
      
      // Here we loop around the square edges
      if(tiltLoop) {
        if(y>100)                      { row++; row=row%ROWS; moved=true; }
        if(y<-100 && row>0)            { row--; moved=true; }
        if(y<-100 && row==0 && !moved) { row=ROWS-1; moved=true; }
      
        if(x<-100)                    { col++; col=col%ROWS; moved=true; }
        if(x>100 && col>0)            { col--; moved=true; }
        if(x>100 && col==0 && !moved) { col=ROWS-1; moved=true; }
        //if(x>100)                    { col++; col=col%ROWS; moved=true; }
        //if(x<-100 && col>0)            { col--; moved=true; }
        //if(x<-100 && col==0 && !moved) { col=ROWS-1; moved=true; }
      }
      // Here, we stop on borders
      else {
        if(y>100 && row<ROWS-1)  { row++; moved=true; }
        if(y<-100 && row>0)      { row--; moved=true; }
  
        if(x<-100 && col<COLS-1) { col++; moved=true; }
        if(x>100 && col>0)       { col--; moved=true; }
        //if(x<-100 && col>0)      { col--; moved=true; }
        //if(x>100 && col<COLS-1)  { col++; moved=true; }
      }
      
      if(moved) {
        ledMatrix.ledSetOff(rowPrevious,colPrevious);
        ledMatrix.ledSetState(row,col,standardColors[COLOR_WHITE]);
        midiNoteOn(0,75,tiltVolume);
    
        lastTimeMoved=millis();
  
        DEBUG_PRINTF("Tilt volume:%d",tiltVolume);
        DEBUG_PRINT("Accelerometer data & light position");
        DEBUG_PRINTF("    x: %d",x);
        DEBUG_PRINTF("    y: %d",y);
        DEBUG_PRINTF("    row: %d",row);
        DEBUG_PRINTF("    col: %d",col);
      }
      
    }
    
  
    // Reading keyboard
    char customKey=customKeypad.getKey();
    if (customKey){
  
      char hexKey[]= { '0', 'x', '0', 0 };
      hexKey[2]=customKey; 
      key = strtol(hexKey,0,16);
      row=key/ROWS; col=key%COLS;
      
      //ledResetMatrix();
      ledMatrix.ledSetOff(rowPrevious,colPrevious);
      ledMatrix.ledSetState(row,col,standardColors[COLOR_WHITE]);
      delay(waitBetweenMove);
      previousKey=key;
    }
  }
}
