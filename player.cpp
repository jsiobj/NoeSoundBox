/*
    Code for Player mode
*/
#include <Keypad.h>
#include <Wire.h> // Must include Wire library for I2C
#include <I2C.h>
#include <MMA8453_n0m1.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <avr/wdt.h>

#include <InteractingObjects_ButtonPad.h>

byte playerCurrentTrackNo=-1;     // Track no currently playing. If -1, no file is playing

#include "misc.h"
#include "globals.h"

void printPlayerStatus() {
  Serial.print("Player status | playerCurrentTrackNo       : "); Serial.println(playerCurrentTrackNo);
  Serial.print("              | Playing                    : "); Serial.println(musicPlayer.playingMusic);
  Serial.print("              | Paused                     : "); Serial.println(musicPlayer.paused());
}

void initPlayer(int option) {
    
  Serial.println(F("VS1053 in Player mode"));
  
    if (!musicPlayer.begin()) {
      Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
      resetArduino();
    }

    // Initialising SD card and dumping file list
    // WARNING : This needs to be done AFTER musicPayer.begin but BEFORE configuring music player
    SD.begin(VS_SHIELD_CCS);    // initialise the SD card
    File root = SD.open("/");
    printDirectory(root, 0);

    // Configuring music player
    musicPlayer.setVolume(20, 20);   // Left and right channel volume (lower number mean louder)
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);

}

void getTrackName(int bank, int track,char *trackName) {

  String playerCurrentTrackName;    // Track name (9.3 : TRACK0001.WAV for instance)
 
  if(bank<10) playerCurrentTrackName="TRK0";
  else        playerCurrentTrackName="TRK";
  playerCurrentTrackName.concat(String(bank));
  
  if(track<10) playerCurrentTrackName.concat("0");
  playerCurrentTrackName.concat(String(track));
  playerCurrentTrackName.concat(".MP3");

  playerCurrentTrackName.toCharArray(trackName,14);
  Serial.print("Track name: "); Serial.println(trackName);

}

//-------------------------------------------------------------------------
void onKeyPressed_Player(int keyCode, int bank) {
  byte row=keyCode/4;
  byte col=keyCode%4;
  char trackName[14];
  
  Serial.println("Current status");
  printPlayerStatus();
  
  
  if(musicPlayer.stopped()) {                                     // Nothing is currently playing
    ledMatrix.matrixLedSetRandom(row,col);
    getTrackName(bank,keyCode,trackName);
    Serial.print("Starting "); Serial.println(trackName);
    musicPlayer.startPlayingFile(trackName);
  }
  else {
    if(playerCurrentTrackNo == keyCode && !musicPlayer.paused()) { // This very track is already playing
      Serial.println("Pausing...");
      musicPlayer.pausePlaying(true);
    }
    else {
      if(playerCurrentTrackNo == keyCode && musicPlayer.paused()) { // This very track is already paused
        Serial.println("Paused. Restarting...");
        musicPlayer.pausePlaying(false);
      }
      else {
        if(playerCurrentTrackNo != keyCode) {                       // This is a new track that was chosen
          ledMatrix.matrixLedSetRandom(row,col);
          musicPlayer.stopPlaying();
          getTrackName(bank,keyCode,trackName);
          Serial.print("Stopping previous & starting "); Serial.println(trackName);
          musicPlayer.startPlayingFile(trackName);    
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
  byte row=keyCode/4;
  byte col=keyCode%4;

  //if(ledMatrix.matrixLedGetState(row,col)==ON) ledMatrix.matrixLedLock(row,col);  

}

//-------------------------------------------------------------------------
void onKeyReleased_Player(int keyCode) {
  byte row=keyCode/4;
  byte col=keyCode%4;
  
  if(ledMatrix.matrixLedGetLockState(row,col)==OFF && ledMatrix.matrixLedGetState(row,col)==ON) {
    ledMatrix.matrixLedToggleState(row,col,standardColors[4]); 
  }
}

//=================================================================================
// What to do in Player mode
//=================================================================================
void loopPlayer(int option) {
  
  // At least one key state changed i.e. a key was pressed or released
  if (customKeypad.getKeys()){

    for (int i=0; i<LIST_MAX; i++)  {
        if ( customKeypad.key[i].stateChanged ) {

          switch (customKeypad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                case PRESSED:
                  onKeyPressed_Player(customKeypad.key[i].kcode,option);
                  break;
                
                case HOLD:
                  onKeyHold_Player(customKeypad.key[i].kcode);
                  break;
                
                case RELEASED:
                  onKeyReleased_Player(customKeypad.key[i].kcode);
                  break;
                
                case IDLE:
                  break;
          }
        }
     }
  }

  ledMatrix.matrixLedRefresh();  
}

