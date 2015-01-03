/*
DebugUtils.h - Simple debugging utilities.

Ideas taken from:
http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1271517197
and more specifically Fabio Varesano code

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#include <WProgram.h>

#ifdef DEBUG
  
      //Serial.print('|');                    \
      //Serial.print(__FILE__);               \
      //Serial.print(':');                    \
      //Serial.println(__LINE__);             \

  #define DEBUG_PRINTF(str,val)          \
      Serial.print(millis());            \
      Serial.print("|");                 \
      Serial.print(__PRETTY_FUNCTION__); \
      Serial.print('|');                 \
      Serial.printf(str,val);            \
      Serial.println();
  
  #define DEBUG_PRINTF3(str,val1,val2,val3) \
      Serial.print(millis());               \
      Serial.print("|");                    \
      Serial.print(__PRETTY_FUNCTION__);    \
      Serial.print('|');                    \
      Serial.printf(str,val1,val2,val3);    \
      Serial.println();

  #define DEBUG_PRINTF2(str,val1,val2)   \
      Serial.print(millis());            \
      Serial.print("|");                 \
      Serial.print(__PRETTY_FUNCTION__); \
      Serial.print('|');                 \
      Serial.printf(str,val1,val2);      \
      Serial.println();

  #define DEBUG_PRINT(str)               \
      Serial.print(millis());            \
      Serial.print("|");                 \
      Serial.print(__PRETTY_FUNCTION__); \
      Serial.print('|');                 \
      Serial.print(str);                 \
      Serial.println();
      
  #define DEBUG_PRINT_ARRAY(array,arrayName,size)      \
      Serial.printf("Array:%s|size:%d",arrayName,size); \
      Serial.println();                                 \
      for(int z=0;z<size;z++)  {                        \
        Serial.print("    "); Serial.print(z);          \
        Serial.print(":"); Serial.println(array[z]);    \
      }                                                    
}
   
#else
  #define DEBUG_PRINT(str)
  #define DEBUG_PRINTF(str,val)
  #define DEBUG_PRINTF2(str,val1,val2)
  #define DEBUG_PRINTF3(str,val1,val2,val3)
  #define DEBUG_PRINT_ARRAY(array,arrayName,arraySize)
#endif

#endif

