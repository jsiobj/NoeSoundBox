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

  Code for Player mode
*/
#define DEBUG
#include "debug.h"

#include <Keypad.h>
#include <Wire.h> // Must include Wire library for I2C
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
#include "player.h"

#define DEFAULT_VOLUME 60
#define MAX_VOLUME     40              // Because smaller is louder
#define MIN_VOLUME     140
#define MID_VOLUME     110

// Track name is build like this : TRACK_ROOT/<bank>/TRACK_PREFIX<track no>.TRACK_EXT
#define TRACK_PREFIX "track"
#define TRACK_EXT    ".mp3"
#define TRACK_ROOT   "/music/"

#define PLAYER_MODE_SINGLE 1   // Play selected file
#define PLAYER_MODE_RANDOM 2   // Not implemented... yet
#define PLAYER_MODE_BANK   3   // Play all files in selected bank

#define STRING_BUFFER_SIZE 128

byte playerCurrentTrackNo=-1;     // Track no currently playing. If -1, no file is playing
byte playerOptionList[COLS*ROWS];
byte playerTrackList[COLS*ROWS];
byte playerMode=PLAYER_MODE_SINGLE;

//-------------------------------------------------------------------------
void buildTrackPath(int bank, int track,char *trackPath) {
  String tmpTrackPath;    

  tmpTrackPath=TRACK_ROOT;
  if(bank<10) tmpTrackPath.concat("0");
  tmpTrackPath.concat(String(bank));
  tmpTrackPath.concat("/");
  tmpTrackPath.concat(TRACK_PREFIX);
  if(track<10) tmpTrackPath.concat("0");
  tmpTrackPath.concat(String(track));
  tmpTrackPath.concat(TRACK_EXT);

  tmpTrackPath.toCharArray(trackPath,STRING_BUFFER_SIZE);
}

//-------------------------------------------------------------------------
void buildTrackName(int track,char *trackName) {
  
  String tmpTrackName;    

  tmpTrackName.concat(TRACK_PREFIX);
  if(track<10) tmpTrackName.concat("0");
  tmpTrackName.concat(String(track));
  tmpTrackName.concat(TRACK_EXT);

  tmpTrackName.toCharArray(trackName,STRING_BUFFER_SIZE);
}

//-------------------------------------------------------------------------
void buildBankPath(int bank,char *bankPath) {

  String tmpBankPath;    

  tmpBankPath=TRACK_ROOT;
  if(bank<10) tmpBankPath.concat("0");
  tmpBankPath.concat(String(bank));
  //tmpBankPath.concat("/");

  tmpBankPath.toCharArray(bankPath,STRING_BUFFER_SIZE);
}

//-------------------------------------------------------------------------
byte* playerGetOptionList(int* size) {
  char bankPath[STRING_BUFFER_SIZE];
  int bankCount=0;

  File root = SD.open(TRACK_ROOT);

  DEBUG_PRINTF("Track root directory",TRACK_ROOT);
  #ifdef DEBUG
  printDirectory(root, 0);
  #endif
  
  for(int bank=0;bank<ROWS*COLS;bank++) {
    buildBankPath(bank,bankPath);
    DEBUG_PRINTF("Bank path",bankPath);
    if(SD.exists(bankPath)) {
      DEBUG_PRINT("----> FOUND !");
      playerOptionList[bankCount]=bank;
      bankCount++;
    }
  }
  root.close();
  DEBUG_PRINTF("  BankCount  ",bankCount);

  *size=bankCount;
  DEBUG_PRINT_ARRAY(playerOptionList,"playerOptionList",ARRAY_LENGTH(playerOptionList));
  return playerOptionList;  
}

//-------------------------------------------------------------------------
byte getBankFiles(byte bank) {
  char trackPath[STRING_BUFFER_SIZE];
  int trackCount=0;

  File root = SD.open("/");
  printDirectory(root, 0);
  for(int track=0;track<ROWS*COLS;track++) {

    buildTrackPath(bank,track,trackPath);
    DEBUG_PRINTF("Track name",trackPath);

    if(SD.exists(trackPath)) {
      DEBUG_PRINT("----> FOUND !");
      ledMatrix.matrixLedSetState(track,standardColors[COLOR_WHITE]);
      trackCount++;
    }
  }
  root.close();
  return trackCount;
}

//-------------------------------------------------------------------------
void printPlayerStatus() {
  //DEBUG_PRINTF("Player status | playerCurrentTrackNo       : ",playerCurrentTrackNo);
  //DEBUG_PRINTF("              | Playing                    : ",musicPlayer.playingMusic);
  //DEBUG_PRINTF("              | Paused                     : ",musicPlayer.paused());
}

//-------------------------------------------------------------------------
// Return player and SD state
// status = 0 if everything is ok
// status = 1 if can't init SD
// status = 2 if cant init VS1053 
int initPlayer(int option) {
    
  int status=PLAYER_OK;
  DEBUG_PRINT("Setting VS1053 in PLAYER mode");
  //DEBUG_PRINTF("Free RAM",freeRam());
  digitalWrite(BOOT_M0DE_PIN,LOW);  // Setting pin connected to GPIO1 and VS1053 to LOW..
  VS1053.reset();                   // ... and resetting

  DEBUG_PRINT("Starting player");
  if (!musicPlayer.begin()) {
    DEBUG_PRINT("Couldn't find VS1053");
    return PLAYER_ERR;
    //while(1);
    //resetArduino();
  }

  // Initialising SD card and dumping file list
  // WARNING : This needs to be done AFTER musicPlayer.begin but BEFORE configuring music player
  DEBUG_PRINT("Starting SD");
  if(SD.begin(VS_SHIELD_CCS)) {
    DEBUG_PRINT("SD started");
    SDStarted=true;
  }
  else {
    DEBUG_PRINT("Cannot start (or restart ?) SD");
    status=PLAYER_NOSD;
  }
  
  // Configuring music player
  //musicPlayer.setVolume(DEFAULT_VOLUME, DEFAULT_VOLUME);   // Left and right channel volume (lower number mean louder)
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);
  return status;
}

//-------------------------------------------------------------------------
void onKeyPressed_Player(int keyCode, int bank) {
  char trackName[STRING_BUFFER_SIZE];

  ledMatrix.ledSetAllOff();
  
  //DEBUG_PRINTF("Free RAM",freeRam());
  printPlayerStatus();
  
  if(ledMatrix.matrixLedGetState(keyCode)==OFF) {              // Nothing associated with this key
    ledMatrix.ledSetState(KEY2ROW(keyCode),KEY2COL(keyCode),standardColors[COLOR_RED]);
    delay(500);
    ledMatrix.ledSetOff(KEY2ROW(keyCode),KEY2COL(keyCode));
    return;
  }
    
  if(musicPlayer.stopped()) {                                     // Nothing is currently playing
    ledMatrix.matrixLedSetState(keyCode,standardColors[COLOR_GREEN]);
    buildTrackPath(bank,keyCode,trackName);
    DEBUG_PRINTF("Starting track",trackName);
    //musicPlayer.playFullFile(trackName);
    //VS1053.softReset();    // Try to fix "hanging"
    musicPlayer.startPlayingFile(trackName);
    digitalWrite(LED_PLAYER,HIGH);
  }
  else {
    if(playerCurrentTrackNo == keyCode && !musicPlayer.paused()) { // This very track is already playing
      ledMatrix.matrixLedSetState(keyCode,standardColors[COLOR_BLUE]);
      DEBUG_PRINT("Pausing...");
      musicPlayer.pausePlaying(true);
    }
    else {
      if(playerCurrentTrackNo == keyCode && musicPlayer.paused()) { // This very track is already paused
        DEBUG_PRINT("Paused. Restarting...");
        ledMatrix.matrixLedSetState(keyCode,standardColors[COLOR_GREEN]);
        musicPlayer.pausePlaying(false);
        digitalWrite(LED_PLAYER,HIGH);
      }
      else {
        if(playerCurrentTrackNo != keyCode) {                       // This is a new track that was chosen
          ledMatrix.matrixLedSetState(keyCode,standardColors[COLOR_GREEN]);
          ledMatrix.matrixLedSetState(playerCurrentTrackNo,standardColors[COLOR_ORANGE]);
          musicPlayer.stopPlaying();
          buildTrackPath(bank,keyCode,trackName);
          DEBUG_PRINTF("Stopping previous & starting track",trackName);
          //VS1053.softReset();    // Try to fix "hanging"
          musicPlayer.startPlayingFile(trackName);    
          digitalWrite(LED_PLAYER,HIGH);
        }
      }
    }
  }  
  
  playerCurrentTrackNo=keyCode;
  printPlayerStatus();
}

//-------------------------------------------------------------------------
void onKeyHold_Player(int keyCode) {
  byte row=KEY2ROW(keyCode), col=KEY2COL(keyCode);

  //if(ledMatrix.matrixLedGetState(row,col)==ON) ledMatrix.matrixLedLock(row,col);  

}

//-------------------------------------------------------------------------
void onKeyReleased_Player(int keyCode) {
  byte row=KEY2ROW(keyCode), col=KEY2COL(keyCode);
  
}

//-------------------------------------------------------------------------
void setVolume(int volume) {

  int redValue,greenValue,blueValue;
  
  if(volume<MAX_VOLUME) volume=MAX_VOLUME; if(volume>MIN_VOLUME) volume=MIN_VOLUME;
  musicPlayer.setVolume(volume,volume);   // Left and right channel volume (lower number mean louder)
  DEBUG_PRINTF("Volume set to",volume);
  
  if(volume<MID_VOLUME) {
    redValue=0;
    greenValue=((MID_VOLUME-volume)*255.0)/(MID_VOLUME-MAX_VOLUME);
    blueValue=((MID_VOLUME-volume)*255.0)/(MID_VOLUME-MAX_VOLUME);
  }
  else {
    blueValue=0;
    redValue=(255-((MIN_VOLUME-volume)*255.0)/(MIN_VOLUME-MID_VOLUME));
    greenValue=(255-((MIN_VOLUME-volume)*255.0)/(MIN_VOLUME-MID_VOLUME));
  }
  
  DEBUG_PRINTF3("RGB",redValue,greenValue,blueValue);
  analogWrite(ENCODER_RED,redValue);
  analogWrite(ENCODER_GREEN,greenValue);
  analogWrite(ENCODER_BLUE,blueValue);

}

//=================================================================================
// What to do in Player mode
//=================================================================================
void loopPlayer(int option) {

  long lastToggle;
  int volume=DEFAULT_VOLUME;  // From 0 (the loudest) to 255 (no sound)
  setVolume(volume);

  DEBUG_PRINTF("Before Piano loop. Volume",volume);

  //DEBUG_PRINTF("Free RAM: %d",freeRam());
  getBankFiles(option);
  //DEBUG_PRINTF("Free RAM: %d",freeRam());
  
  while(1) {

    // If player button pressed, restarting player...
    if(digitalRead(BTN_PLAYER)==LOW) {
      while(digitalRead(BTN_PLAYER)==LOW) 1;
      DEBUG_PRINT("Restarting player");
      boxOption=-1;
      break;
    }
    
    // Controlling volume  
    long volKnobMove = encoder->getValue();
    if (volKnobMove) {
      volume=volume-volKnobMove;
      setVolume(volume);      
    }

    // If player paused, make player led blink
    if(musicPlayer.paused() && millis()-lastToggle>MODE_LED_BLINK) {
      digitalWrite(LED_PLAYER,!digitalRead(LED_PLAYER));
      lastToggle=millis();
    } 
    
    // At least one key state changed i.e. a key was pressed or released
    if (customKeypad.getKeys()){
      for (int i=0; i<LIST_MAX; i++)  {
          if ( customKeypad.key[i].stateChanged ) {
            //DEBUG_PRINTF("Free RAM: %d",freeRam());
            switch (customKeypad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                  case PRESSED:  onKeyPressed_Player(customKeypad.key[i].kcode,option);  break;
                  case HOLD:     onKeyHold_Player(customKeypad.key[i].kcode); break;
                  case RELEASED: onKeyReleased_Player(customKeypad.key[i].kcode); break;
                  case IDLE:     break;
            }
            //DEBUG_PRINTF("Free RAM: %d",freeRam());
          }
       }
    }
    
    if(musicPlayer.paused() || musicPlayer.stopped()) { 
        ledMatrix.matrixLedRefresh();
    }
    else { 
        ledMatrix.ledSetState(KEY2ROW(playerCurrentTrackNo),KEY2COL(playerCurrentTrackNo),standardColors[COLOR_GREEN]);
    }
  }  
  musicPlayer.stopPlaying();
}

