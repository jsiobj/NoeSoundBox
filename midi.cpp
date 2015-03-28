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

  MIDI mode related code.
  Inspired from Adafruit's example code for Music Maker
  (https://github.com/adafruit/Adafruit_VS1053_Library)

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

#include "midi.h"
#include "misc.h"
//#include "globals.h"

void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --; // page 32 has instruments starting with 1 not 0 :(
  if (inst > 127) return;
  
  Serial3.write(MIDI_CHAN_PROGRAM | chan);  
  Serial3.write(inst);
}

void midiSetChannelVolume(uint8_t chan, uint8_t vol) {
  if (chan > 15) return;
  if (vol > 127) return;
  
  Serial3.write(MIDI_CHAN_MSG | chan);
  Serial3.write(MIDI_CHAN_VOLUME);
  Serial3.write(vol);
}

void midiSetChannelBank(uint8_t chan, uint8_t bank) {
  if (chan > 15) return;
  if (bank > 127) return;
  
  Serial3.write(MIDI_CHAN_MSG | chan);
  Serial3.write((uint8_t)MIDI_CHAN_BANK);
  Serial3.write(bank);
}

void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  Serial3.write(MIDI_NOTE_ON | chan);
  Serial3.write(n);
  Serial3.write(vel);
}

void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  Serial3.write(MIDI_NOTE_OFF | chan);
  Serial3.write(n);
  Serial3.write(vel);
}

void testMidi() {
  for (uint8_t i = 60; i < 62; i++) {
    midiNoteOn(0, i, 127); delay(100); midiNoteOff(0, i, 127);
  }  
}
