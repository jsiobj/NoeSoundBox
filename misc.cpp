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

  Misc. functions and tools

*/

#include <SD.h>
#include <avr/wdt.h>

#include <InteractingObjects_ButtonPad.h>
//#include "globals.h"
#include "NoJuBo.h"
#include "debug.h"

int freeRam()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void resetArduino() {
  wdt_enable(WDTO_15MS);
  while(1);
}

void printDirectory(File dir, int numTabs) {
   while(true) {

     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

void setPWMFreq() {

  // TCCR0B = TCCR0B & B11111000 | B00000011;    // set timer 0 divisor to    64 for PWM frequency of   976.56 Hz
  TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  TCCR3B = TCCR3B & B11111000 | B00000001;    // set timer 3 divisor to     1 for PWM frequency of 31372.55 Hz
  TCCR4B = TCCR4B & B11111000 | B00000001;    // set timer 4 divisor to     1 for PWM frequency of 31372.55 Hz
  TCCR5B = TCCR5B & B11111000 | B00000001;    // set timer 5 divisor to     1 for PWM frequency of 31372.55 Hz

}

int hex2dec(char hex) {
    char hexKey[]= { '0', 'x', '0', 0 };
    hexKey[2]=hex;                         // Building an hex number...
    return strtol(hexKey,0,16);            // ... so it can be converted to an int
}

int readButton() {

  // Reading button (last wins)
  if(digitalRead(BTN_PLAYER)==LOW) {
    DEBUG_PRINT("Button 1 pressed...");
    return BTN_PLAYER;
  }
  if(digitalRead(BTN_TILT)==LOW) {
    DEBUG_PRINT("Button 2 pressed...");
    return BTN_TILT;
  }
  if(digitalRead(BTN_PIANO)==LOW) {
    DEBUG_PRINT("Button 3 pressed...");
    return BTN_PIANO;
  }

  return -1; // no button pressed
}

int button2Led(int btn) {
  if(btn==BTN_PLAYER) {return LED_PLAYER; }
  if(btn==BTN_TILT) {return LED_TILT; }
  if(btn==BTN_PIANO) {return LED_PIANO; }
  return -1;
}

int button2Library(int btn) {
  if(btn==BTN_PLAYER) {return 1; }
  if(btn==BTN_TILT) {return 3; }
  if(btn==BTN_PIANO) {return 2; }
  return -1;
}

int library2Led(int library) {
  if(library==1) {return LED_PLAYER; }
  if(library==3) {return LED_TILT; }
  if(library==2) {return LED_PIANO; }
  return -1;
}
