/*
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
#define MAX_VOLUME 0              // Because smaller is louder

// Track name is build like this : TRACK_ROOT/<bank>/TRACK_PREFIX<track no>.TRACK_EXT
#define TRACK_PREFIX "track"
#define TRACK_EXT    ".mp3"
#define TRACK_ROOT   "/music/"

#define STRING_BUFFER_SIZE 128

byte playerCurrentTrackNo=-1;     // Track no currently playing. If -1, no file is playing
byte playerOptionList[COLS*ROWS];
byte playerTrackList[COLS*ROWS];

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
  /*DEBUG_PRINTF("Track name length:%d",STRING_BUFFER_SIZE);
  DEBUG_PRINTF("Track name       :%s",tmpTrackName);
  
  DEBUG_PRINTF("Track name:%s",trackName);*/
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

  DEBUG_PRINTF("Track root directory: %s",TRACK_ROOT);
  #ifdef DEBUG
  printDirectory(root, 0);
  #endif
  
  for(int bank=0;bank<ROWS*COLS;bank++) {
    buildBankPath(bank,bankPath);
    DEBUG_PRINTF("Bank path:%s",bankPath);
    if(SD.exists(bankPath)) {
      DEBUG_PRINT("----> FOUND !");
      playerOptionList[bankCount]=bank;
      bankCount++;
    }
  }
  root.close();
  DEBUG_PRINTF("  BankCount  :%d",bankCount);

  *size=bankCount;
  DEBUG_PRINT_ARRAY(playerOptionList,"playerOptionList",ARRAY_LENGTH(playerOptionList));
  return playerOptionList;  
}

//-------------------------------------------------------------------------
byte getBankFiles(byte bank) {
  char trackPath[STRING_BUFFER_SIZE];
  //char bankPath[STRING_BUFFER_SIZE];
  int trackCount=0;

  //buildBankPath(bank,bankPath);
  //DEBUG_PRINTF("Bank path: %s",bankPath);
  //DEBUG_PRINTF("RAM: %d",freeRam());

  File root = SD.open("/");
  printDirectory(root, 0);
  for(int track=0;track<ROWS*COLS;track++) {

    buildTrackPath(bank,track,trackPath);
    //buildTrackName(track,trackName);
    DEBUG_PRINTF("Track name: %s",trackPath);

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
  Serial.print("Player status | playerCurrentTrackNo       : "); Serial.println(playerCurrentTrackNo);
  Serial.print("              | Playing                    : "); Serial.println(musicPlayer.playingMusic);
  Serial.print("              | Paused                     : "); Serial.println(musicPlayer.paused());
}

//-------------------------------------------------------------------------
void initPlayer(int option) {
    
  DEBUG_PRINT("Setting VS1053 in PLAYER mode");
  digitalWrite(BOOT_M0DE_PIN,LOW);  // Setting pin connected to GPIO1 and VS1053 to LOW..
  VS1053.reset();                   // ... and resetting

  DEBUG_PRINT("Starting player");
  if (!musicPlayer.begin()) {
    Serial.println(F("Couldn't find VS1053"));
    while(1);
    //resetArduino();
  }

  // Initialising SD card and dumping file list
  // WARNING : This needs to be done AFTER musicPayer.begin but BEFORE configuring music player
  DEBUG_PRINT("Starting SD");
  if(SD.begin(VS_SHIELD_CCS)) {
    DEBUG_PRINT("SD started");
    SDStarted=true;
  }
  else {
    DEBUG_PRINT("Cannot start (or restart ?) SD");
  }
  
  // Configuring music player
  musicPlayer.setVolume(DEFAULT_VOLUME, DEFAULT_VOLUME);   // Left and right channel volume (lower number mean louder)
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);
}

//-------------------------------------------------------------------------
void onKeyPressed_Player(int keyCode, int bank) {
  byte row=KEY2ROW(keyCode), col=KEY2COL(keyCode);
  char trackName[STRING_BUFFER_SIZE];
  
  printPlayerStatus();
  
  if(musicPlayer.stopped()) {                                     // Nothing is currently playing
    ledMatrix.matrixLedSetState(row,col,standardColors[COLOR_GREEN]);
    buildTrackPath(bank,keyCode,trackName);
    Serial.print("Starting "); Serial.println(trackName);
    musicPlayer.startPlayingFile(trackName);
    digitalWrite(LED_PLAYER,HIGH);
  }
  else {
    if(playerCurrentTrackNo == keyCode && !musicPlayer.paused()) { // This very track is already playing
      ledMatrix.matrixLedSetState(row,col,standardColors[COLOR_BLUE]);
      Serial.println("Pausing...");
      musicPlayer.pausePlaying(true);
    }
    else {
      if(playerCurrentTrackNo == keyCode && musicPlayer.paused()) { // This very track is already paused
        Serial.println("Paused. Restarting...");
        ledMatrix.matrixLedSetState(row,col,standardColors[COLOR_GREEN]);
        musicPlayer.pausePlaying(false);
        digitalWrite(LED_PLAYER,HIGH);
      }
      else {
        if(playerCurrentTrackNo != keyCode) {                       // This is a new track that was chosen
          ledMatrix.matrixLedSetState(row,col,standardColors[COLOR_GREEN]);
          musicPlayer.stopPlaying();
          buildTrackPath(bank,keyCode,trackName);
          Serial.print("Stopping previous & starting "); Serial.println(trackName);
          musicPlayer.startPlayingFile(trackName);    
          digitalWrite(LED_PLAYER,HIGH);
        }
      }
    }
  }  
  
  playerCurrentTrackNo=keyCode;

  Serial.println("New status");
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

//=================================================================================
// What to do in Player mode
//=================================================================================
void loopPlayer(int option) {

  long lastToggle;
  int volume=DEFAULT_VOLUME;  // From 0 (the loudest) to 255 (no sound)

  DEBUG_PRINTF("Volume:%d",volume);

  getBankFiles(option);
  
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
      if(volume<0) volume=0; if(volume>254) volume=254;
      musicPlayer.setVolume(volume,volume);   // Left and right channel volume (lower number mean louder)
      DEBUG_PRINTF("Volume set to %d",volume);
    }

    // If player button pressed, pausing...
    if(digitalRead(BTN_PLAYER)==LOW) {
      onKeyPressed_Player(playerCurrentTrackNo,option);
      while(digitalRead(BTN_PLAYER)==LOW) 1;
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
            switch (customKeypad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                  case PRESSED:  onKeyPressed_Player(customKeypad.key[i].kcode,option);  break;
                  case HOLD:     onKeyHold_Player(customKeypad.key[i].kcode); break;
                  case RELEASED: onKeyReleased_Player(customKeypad.key[i].kcode); break;
                  case IDLE:     break;
            }
          }
       }
    }
    ledMatrix.matrixLedRefresh();  
  }

  musicPlayer.stopPlaying();

}

