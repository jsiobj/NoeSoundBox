#include <Arduino.h>

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

  Main sketch

*/

#define DEBUG
#include "debug.h"

#include <Keypad.h>           // For keypad management
#include <Wire.h>             // Needed for I2C
#include <I2C.h>              // I2C communications used by accelerometer
#include <MMA8453_n0m1.h>     // Accelerometer
#include <SPI.h>              // Needed for VS1053
#include <Adafruit_VS1053.h>  // VS1053 shield (music maker)
#include <SD.h>               // SD card library, where are stored music files
#include <avr/wdt.h>          // Watchdog library, used for soft resetting arduino
#include <TimerOne.h>

#define WITHOUT_BUTTON 1
#include <ClickEncoder.h>

#include <InteractingObjects_ButtonPad.h>  // Button pad / led management library

#include "NoJuBo.h"
#include "midi.h"        // Headers for Midi functions
#include "piano.h"       // Headers for piano mode
#include "player.h"      // Headers for player mode
#include "tilt.h"        // Headers for Tilt mode
#include "misc.h"        // Headers for misc. functions

byte standardColors[][3] =  {
  {255, 0  , 0  },
  {0  , 255, 0  },
  {0  , 0  , 255},
  {255, 255, 0  },
  {255, 0  , 255},
  {0  , 255, 255},
  {255, 255, 255},
  {255, 165, 0  }
};

boolean SDStarted=false; // Let know if SD already started
int VS1053Mode;          // Stores current mode (MIDI or PLAYER)
int lastVS1053Mode;      // Stores last mode, to detect when it changes

int boxMode=BOX_MODE_UNDEF;
int lastBoxMode=BOX_MODE_UNDEF;
int boxOption=-1;
int configured=0;
int* validOptionList;
int validOptionCount;
int playerStatus;

int playerLibrary=-1;
int playerBank=-1;

long volKnobOldPos  = 0;  //
int volume=100;           // From 0 to 100%

// Key mapping for keypad library. Key hexa number
char hexaKeys[ROWS][COLS] = {
  {'0', '1', '2', '3'},
  {'4', '5', '6', '7'},
  {'8', '9', 'A', 'B'},
  {'C', 'D', 'E', 'F'}
};

byte pinBtn[ROWS]       = {22,23,24,25}; // Buttons rows
byte pinBtnGnd[COLS]    = {26,28,30,32}; // Buttons cols
byte customPinLedRGB[ROWS][3] = {{11,9,10},{8,6,7},{5,2,3},{44,46,45}}; // LEDs rows
byte customPinLedGnd[COLS]    = {27,29,31,33}; // LEDs cols

// Creation of needed objects
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), pinBtn, pinBtnGnd, ROWS, COLS);    // Keypad
rgbLedMatrix ledMatrix = rgbLedMatrix(customPinLedRGB, customPinLedGnd, ROWS, COLS);  // LED matrix
MMA8453_n0m1 accel;                                                                   // Accelerometer

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(VS_SHIELD_MCS,VS_SHIELD_DCS,VS_SHIELD_DRQ,VS_SHIELD_CCS);  // Player
Adafruit_VS1053 VS1053 = Adafruit_VS1053(-1,VS_SHIELD_MCS,VS_SHIELD_DCS,VS_SHIELD_DRQ);  // VS1053

ClickEncoder *encoder;

//-------------------------------------------------------------------------------
// Encoder interrupt callback
//-------------------------------------------------------------------------------
void timerIsr() {
  encoder->service();
}

//-------------------------------------------------------------------------------
// At startup, select mode, ie PLAYER, PIANO or TILT
//-------------------------------------------------------------------------------
int selectMode() {
  int selectedMode=BOX_MODE_UNDEF;
  long lastToggle=millis();

  DEBUG_PRINT("Mode selection...");

  while(selectedMode==BOX_MODE_UNDEF) {

    // Blinking all status leds if mode not yet selected
    if(millis()-lastToggle>MODE_LED_BLINK) {
      if(playerStatus == PLAYER_OK) digitalWrite(LED_PLAYER,!digitalRead(LED_PLAYER));
      if(playerStatus != PLAYER_ERR) digitalWrite(LED_PIANO,!digitalRead(LED_PIANO));
      digitalWrite(LED_TILT,!digitalRead(LED_TILT));
      lastToggle=millis();
    }

    // Reading button (last wins)
    if(digitalRead(BTN_PLAYER)==LOW && playerStatus == PLAYER_OK) {
      selectedMode=BOX_MODE_PLAYER;
      digitalWrite(LED_PLAYER,HIGH);
      digitalWrite(LED_TILT,LOW);
      digitalWrite(LED_PIANO,LOW);
    }
    if(digitalRead(BTN_TILT)==LOW) {
      selectedMode=BOX_MODE_TILT;
      digitalWrite(LED_PLAYER,LOW);
      digitalWrite(LED_TILT,HIGH);
      digitalWrite(LED_PIANO,LOW);
    }
    if(digitalRead(BTN_PIANO)==LOW && playerStatus != PLAYER_ERR) {
      selectedMode=BOX_MODE_PIANO;
      digitalWrite(LED_PLAYER,LOW);
      digitalWrite(LED_TILT,LOW);
      digitalWrite(LED_PIANO,HIGH);
    }
  }

  delay(500); // Just wait a little bit so the button is properly released
  DEBUG_PRINTF("Selected mode",selectedMode);

  // Know we know the mode, building option list
  switch(selectedMode) {
    case BOX_MODE_PIANO:
      DEBUG_PRINT("Piano mode: Getting valid options list")
      validOptionList=pianoGetOptionList(&validOptionCount);
      break;

    case BOX_MODE_PLAYER:
      DEBUG_PRINT("Player mode: Asking for music library");
      playerSelectLibrary();

      break;

    case BOX_MODE_TILT:
      DEBUG_PRINT("Tilt mode: Getting valid options list");
      validOptionList=tiltGetOptionList(&validOptionCount);
      break;

    default:
      DEBUG_PRINT("Ooops... unknown mode ! Resetting");
      resetArduino();
      break;
  }

  return selectedMode;
}

//-------------------------------------------------------------------------------
// At startup, using the keypad to let the user choose one key (among the 16)
// so that we can select different options.
// For MIDI mode, an option will change the intrument and note mapping
// For PLAYER mode, an option will change the music "bank" to play from
// For TILT mode, selects bounce / no bounce
//-------------------------------------------------------------------------------
int selectOption(int* optionList,int optionCount) {

  int selectedOption=-1;            // Selected option

  long lastToggle=millis();         // used for led blinking
  byte curOptIdx=0;                 // Current option index for led blinking

  boolean validOption=false;

  DEBUG_PRINT("Option selection...");

  ledMatrix.matrixLedSetAllOff();
  for(int option=0;option<optionCount;option++) ledMatrix.matrixLedSetState(optionList[option],standardColors[COLOR_WHITE]);


  while(selectedOption==-1) {

    // If any button pressed, restarting player on the selected library...
    if(boxMode == BOX_MODE_PLAYER) {
      int btn=readButton();
      if(btn!=-1) {
        while(digitalRead(btn)==LOW) 1;
        musicPlayer.stopPlaying();
        boxOption=-1;
        playerBank=-1;
        playerLibrary=button2Library(btn);
        digitalWrite(LED_PIANO, LOW);
        digitalWrite(LED_PLAYER, LOW);
        digitalWrite(LED_TILT, LOW);
        digitalWrite(library2Led(playerLibrary), HIGH);
        DEBUG_PRINTF("New library selected",playerLibrary);
        return -1;
      }
    }

    // Make the led matrix blink
    if(millis()-lastToggle>OPTION_LED_BLINK) {
       ledMatrix.matrixLedSetState(optionList[curOptIdx],standardColors[COLOR_WHITE]);
       curOptIdx=(curOptIdx+1)%optionCount;
       ledMatrix.matrixLedSetState(optionList[curOptIdx],standardColors[COLOR_BLUE]);
       lastToggle=millis();
    }

    char kchar = customKeypad.getKey();
    if (kchar) {
      int kcode=hex2dec(kchar);
      DEBUG_PRINTF("Option selected",kcode);

      // Checking if button pressed is in the option list.
      for(int j=0;j<optionCount;j++)
        if(kcode==optionList[j])
          validOption=true;

      // If it is, setting the option
      if(validOption) {
        DEBUG_PRINTF("Option is valid",kcode);
        ledMatrix.matrixLedSetState(optionList[curOptIdx],standardColors[COLOR_WHITE]);
        //ledMatrix.matrixLedSetAllOff();
        ledMatrix.matrixLedSetState(kcode,standardColors[COLOR_GREEN]);
        long now=millis();
        while(millis()-now<500) ledMatrix.matrixLedRefresh();
        ledMatrix.matrixLedSetAllOff();
        selectedOption = kcode;
      }
      else {
        DEBUG_PRINTF("Option is not valid",kcode);
        ledMatrix.matrixLedSetState(optionList[curOptIdx],standardColors[COLOR_WHITE]);
        ledMatrix.matrixLedSetState(kcode,standardColors[COLOR_RED]);
        long now=millis();
        while(millis()-now<500) ledMatrix.matrixLedRefresh();
        ledMatrix.matrixLedSetOff(kcode);
        ledMatrix.matrixLedSetState(optionList[curOptIdx],standardColors[COLOR_WHITE]);
      }
    }
    ledMatrix.matrixLedRefresh();
  }

  DEBUG_PRINTF("Seletected option",selectedOption);
  return selectedOption;
}


//=================================================================================
// SETUP
//=================================================================================
void setup() {

  Serial.begin(115200);

  DEBUG_PRINT("Setting pins for buttons and leds");

  // VS boot pin mode (Player / Midi)
  pinMode(BOOT_M0DE_PIN,OUTPUT);

  // Status LEDs settings and testing
  pinMode(LED_TILT, OUTPUT);
  pinMode(LED_PIANO, OUTPUT);
  pinMode(LED_PLAYER, OUTPUT);

  // Mode selection buttons
  pinMode(BTN_PLAYER,INPUT);
  pinMode(BTN_TILT,INPUT);
  pinMode(BTN_PIANO,INPUT);

  digitalWrite(BTN_PLAYER,HIGH);  // Enabling pullup resistor
  digitalWrite(BTN_TILT,HIGH);
  digitalWrite(BTN_PIANO,HIGH);

  // Encoder & push button
  pinMode(ENCODER_BTN,INPUT);
  pinMode(ENCODER_RED,OUTPUT);
  pinMode(ENCODER_GREEN,OUTPUT);
  pinMode(ENCODER_BLUE,OUTPUT);

  analogWrite(ENCODER_RED,255);
  analogWrite(ENCODER_GREEN,255);
  analogWrite(ENCODER_BLUE,255);

  DEBUG_PRINT("Starting up...");

  digitalWrite(LED_TILT,HIGH);
  digitalWrite(LED_PIANO,HIGH);
  digitalWrite(LED_PLAYER,HIGH);

  DEBUG_PRINT("Setting highest possible PWM frequency");
  setPWMFreq();

  DEBUG_PRINT("Setting encoder");
  encoder = new ClickEncoder(ENCODER_A,ENCODER_B);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  // Encoder button was pressed upon startup... launching test mode
  if(digitalRead(ENCODER_BTN)==HIGH) {
    DEBUG_PRINT("Entering check mode...");
    boxMode=BOX_MODE_CHECK;
    return; // Exit setup before initializing other stuff (just want to test buttons and led Matrix
  }

  DEBUG_PRINT("Starting accelerometer");
  accel.setI2CAddr(0x1D);
  accel.dataMode(true, 2); //enable highRes 10bit, 2g range [2g,4g,8g]

  DEBUG_PRINT("Starting VS1053");
  VS1053.begin();

  DEBUG_PRINT("Starting VS1053 Player");
  playerStatus=initPlayer();
  switch (playerStatus) {
    case PLAYER_ERR :
      DEBUG_PRINT("VS1053 error");
      digitalWrite(LED_PIANO,LOW);
      digitalWrite(LED_PLAYER,LOW);
      break;

    case PLAYER_NOSD :
      DEBUG_PRINT("SD error");
      digitalWrite(LED_PLAYER,LOW);
      break;
  }

  DEBUG_PRINT("Testing led matrix");
  ledMatrix.ledTestAll(standardColors[COLOR_WHITE]);

  DEBUG_PRINT("Getting user mode...");
  boxMode=selectMode();

  /*DEBUG_PRINT("Switching status leds off");
  digitalWrite(LED_TILT,LOW);
  digitalWrite(LED_PIANO,LOW);
  digitalWrite(LED_PLAYER,LOW);*/
}

//=================================================================================
// LOOP
//=================================================================================
void loop() {

  // Check mode to test all leds and keyboard
  if(boxMode == BOX_MODE_CHECK) {
    // Test mode variable
    int encValue,encPos;
    byte red=0,blue=255,green=255;

    long int btnPress[ROWS][COLS] = { {0,0,0,0},
                                      {0,0,0,0},
                                      {0,0,0,0},
                                      {0,0,0,0} };

    // Reading button (last wins) and testing pad colors
    byte color_red[]={255,0,0};
    byte color_green[]={0,255,0};
    byte color_blue[]={0,0,255};
    /*if(checkedLed==0) {
      DEBUG_PRINT("Checking LEDs...");
      ledMatrix.ledTestMatrix(200);
      checkedLed=1;
    }*/

    // Testing keypad, buttons and encoder
    DEBUG_PRINT("Checking keyboard and encoder, press keys to test or turn encoder to test...");
    // Looping forever... until reset ! (check mode)
    while(1) {

      if(digitalRead(BTN_PLAYER)==LOW) {
        DEBUG_PRINT("Matrix set to Red");
        for(int row=0;row<ROWS;row++) {
          for(int col=0;col<COLS;col++) {
            ledMatrix.ledSetState(row,col,color_red);
          }
        }
      }
      if(digitalRead(BTN_TILT)==LOW) {
        DEBUG_PRINT("Matrix set to Green");
        for(int row=0;row<ROWS;row++) {
          for(int col=0;col<COLS;col++) {
            ledMatrix.ledSetState(row,col,color_green);
          }
        }
      }
      if(digitalRead(BTN_PIANO)==LOW) {
        DEBUG_PRINT("Matrix set to Blue");
        for(int row=0;row<ROWS;row++) {
          for(int col=0;col<COLS;col++) {
            ledMatrix.ledSetState(row,col,color_blue);
          }
        }
      }

      // Keyboard reading
      char customKey=customKeypad.getKey();
      if (customKey){
        char hexKey[]= { '0', 'x', '0', 0 };
        hexKey[2]=customKey;
        byte key = strtol(hexKey,0,16);
        byte row=KEY2ROW(key); byte col=KEY2COL(key);
        btnPress[row][col]++;

        DEBUG_PRINTF("-- Key",key);
        DEBUG_PRINTF2("   Logical coordinates R/C",row,col);
        DEBUG_PRINTF2("   Physical coordinates R/C",ROW2ADDR(row),COL2ADDR(col));

        ledMatrix.ledSetAllOff();
        ledMatrix.ledSetState(row,col,standardColors[btnPress[row][col]%4]);
      }

      // Encoder reading
      encValue = encoder->getValue();
      encPos += encValue;
      if (abs(encPos)>3) {
        encPos=0;
        DEBUG_PRINTF("Encoder pos",encPos);
        DEBUG_PRINTF("Encoder Value",encValue);
        DEBUG_PRINTF3("R/G/B",red,green,blue);

        if(red==0) { red=255; green=0; blue=255; }
        else if(green==0) { red=255; green=255; blue=0; }
        else if(blue==0) { red=0; green=255; blue=255; }

        analogWrite(ENCODER_RED,red);
        analogWrite(ENCODER_GREEN,green);
        analogWrite(ENCODER_BLUE,blue);
      }
    }
  }
  // Not it test mode, lets run real code
  else {
    // If not done yet, choosing the right option
    while(boxOption==-1) {
      switch (boxMode) {
        case BOX_MODE_PIANO:
          boxOption=selectOption(validOptionList,validOptionCount);
          DEBUG_PRINTF("Piano mode: boxOption",boxOption);
          initPiano(boxOption);
          break;

        case BOX_MODE_TILT:
          boxOption=selectOption(validOptionList,validOptionCount);
          DEBUG_PRINTF("Tilt mode: boxOption",boxOption);
          initTilt();
          break;

        case BOX_MODE_PLAYER:
          DEBUG_PRINT("Player mode: Building bank list");
          validOptionList=playerGetOptionList(playerLibrary,&validOptionCount);
          DEBUG_PRINT("Player mode: Selecting bank")
          boxOption=selectOption(validOptionList,validOptionCount)+1; // bank numbered from 1 instead of 0
          if(boxOption!=0) {
            playerBank=boxOption;
            DEBUG_PRINTF("Player mode: boxOption",boxOption);
            initPlayer();
          }
          else {
            boxOption = -1;
          }
          break;
      }
    }

    // Now, starting the right loop, depending on mode
    switch (boxMode) {
      case BOX_MODE_PIANO:
        loopPiano();
        break;

      case BOX_MODE_TILT:
        loopTilt(boxOption);
        break;

      case BOX_MODE_PLAYER:
        loopPlayer();
        break;
    }
  }
}
