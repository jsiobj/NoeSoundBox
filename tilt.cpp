/*
    MIDI mode related code.
*/

#include <Keypad.h>
#include <Wire.h> 
#include <I2C.h>
#include <MMA8453_n0m1.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <avr/wdt.h>

#include <InteractingObjects_ButtonPad.h>

#include "misc.h"
#include "globals.h"
#include "midi.h"

int tiltVolume=127;

byte key,previousKey=32;
byte row=32,col=32,rowPrevious=32,colPrevious=32;
long int lastTimeMoved;
const int waitBetweenMove=100; // in ms

void initTilt() {
    Serial.println(F("Starting TILT Mode"));
    Serial3.begin(31250);

    midiSetChannelBank(0, VS1053_BANK_DRUMS1);
    midiSetInstrument(0, VS1053_BANK_DRUMS1);
    midiSetChannelVolume(0, tiltVolume);
}

//=================================================================================
// What to do in TILT mode
//=================================================================================
void loopTilt(bool tiltLoop) {
  
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
    
//      if(x<-100)                    { col++; col=col%ROWS; moved=true; }
//      if(x>100 && col>0)            { col--; moved=true; }
//      if(x>100 && col==0 && !moved) { col=ROWS-1; moved=true; }
      if(x>100)                    { col++; col=col%ROWS; moved=true; }
      if(x<-100 && col>0)            { col--; moved=true; }
      if(x<-100 && col==0 && !moved) { col=ROWS-1; moved=true; }
    }
    // Here, we stop on borders
    else {
      if(y>100 && row<ROWS-1)  { row++; moved=true; }
      if(y<-100 && row>0)      { row--; moved=true; }

//      if(x<-100 && col<COLS-1) { col++; moved=true; }
//      if(x>100 && col>0)       { col--; moved=true; }
      if(x<-100 && col>0)      { col--; moved=true; }
      if(x>100 && col<COLS-1)  { col++; moved=true; }
    }
    
    if(moved) {
      ledMatrix.ledSetOff(rowPrevious,colPrevious);
      ledMatrix.ledSetState(row,col,standardColors[COLOR_WHITE]);
      midiNoteOn(0,75,tiltVolume);
  
      lastTimeMoved=millis();

      Serial.print("x: ");  Serial.print(x);
      Serial.print(" y: "); Serial.print(y);
      Serial.print(" | ");
      Serial.print("row: "); Serial.print(row);
      Serial.print("col: "); Serial.print(col);
      Serial.println();
    }
    
  }
  

  // Reading keyboard
  char customKey=customKeypad.getKey();
  if (customKey){

    char hexKey[]= { '0', 'x', '0', 0 };
    hexKey[2]=customKey; 
    key = strtol(hexKey,0,16);
    row=key/ROWS; col=key%COLS;
    
    #ifdef DEBUG
    Serial.print("Hex:");Serial.print(hexKey);
    Serial.print("|Key:"); Serial.print(key);
    Serial.print("|Prv:"); Serial.print(previousKey);
    Serial.print("|Row:"); Serial.print(row);
    Serial.print("|Col:"); Serial.print(col);
    Serial.print("|Cnt:"); Serial.print(btnPress[row][col]%5);
    Serial.println();
    Serial.println("Led colors:"); dbgPrintLedColors();
    Serial.println("Led statuses:"); dbgPrintLedStatus();
    Serial.println("Button presses:"); dbgPrintBtnPress();
    #endif
 
    //ledResetMatrix();
    ledMatrix.ledSetOff(rowPrevious,colPrevious);
    ledMatrix.ledSetState(row,col,standardColors[COLOR_WHITE]);
    delay(waitBetweenMove);
    previousKey=key;
  }

 
   
}
