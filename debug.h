/*
  debug.h - Simple debugging utilities.

  Ideas taken from:
  http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1271517197
  and more specifically Fabio Varesano code

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

*/

#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#include <Arduino.h>

#ifdef DEBUG

  #define DEBUG_PRINTF(str,val)          \
      Serial.print(millis());            \
      Serial.print("|");                 \
      Serial.print(__PRETTY_FUNCTION__); \
      Serial.print('|');                 \
      Serial.print(str);                 \
      Serial.print('|');                 \
      Serial.print(val);                 \
      Serial.println();

  #define DEBUG_PRINTF3(str,val1,val2,val3) \
      Serial.print(millis());               \
      Serial.print("|");                    \
      Serial.print(__PRETTY_FUNCTION__);    \
      Serial.print('|');                    \
      Serial.print(str);                    \
      Serial.print('|');                    \
      Serial.print(val1);                   \
      Serial.print('|');                    \
      Serial.print(val2);                   \
      Serial.print('|');                    \
      Serial.print(val3);                   \
      Serial.println();

  #define DEBUG_PRINTF2(str,val1,val2)   \
      Serial.print(millis());            \
      Serial.print("|");                 \
      Serial.print(__PRETTY_FUNCTION__); \
      Serial.print('|');                 \
      Serial.print(str);                    \
      Serial.print('|');                    \
      Serial.print(val1);                   \
      Serial.print('|');                    \
      Serial.print(val2);                   \
      Serial.println();

  #define DEBUG_PRINT(str)               \
      Serial.print(millis());            \
      Serial.print("|");                 \
      Serial.print(__PRETTY_FUNCTION__); \
      Serial.print('|');                 \
      Serial.print(str);                 \
      Serial.println();

  #define DEBUG_PRINT_ARRAY(array,arrayName,size)       \
      Serial.print("Array:");                           \
      Serial.print(arrayName);                          \
      Serial.print("|size:");                           \
      Serial.print(size);                               \
      Serial.println();                                 \
      for(byte z=0;z<size;z++)  {                       \
        Serial.print("    "); Serial.print(z);          \
        Serial.print(":"); Serial.println(array[z]);    \
      }

#else
  #define DEBUG_PRINT(str)
  #define DEBUG_PRINTF(str,val)
  #define DEBUG_PRINTF2(str,val1,val2)
  #define DEBUG_PRINTF3(str,val1,val2,val3)
  #define DEBUG_PRINT_ARRAY(array,arrayName,arraySize)
#endif

#endif
