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

  Midi mode header

*/
#ifndef MIDI_H
#define MIDI_H MIDI_H

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 31
#define VS1053_BANK_DEFAULT 0x00
#define VS1053_BANK_DRUMS1 0x78
#define VS1053_BANK_DRUMS2 0x7F
#define VS1053_BANK_MELODY 0x79

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 32 for more!
#define VS1053_GM1_ACOUSTIC_GRAND_PIANO 1
#define VS1053_GM1_BRIGHT_ACOUSTIC_PIANO 2
#define VS1053_GM1_ACOUSTIC_GUITAR_NYLON 25
#define VS1053_GM1_ELECTRIC_GUITAR_JAZZ 27
#define VS1053_GM1_ACOUSTIC_BASS 27
#define VS1053_GM1_ELECTRIC_BASS_FINGER 34
#define VS1053_GM1_VIOLIN 41
#define VS1053_GM1_OCARINA 80
#define VS1053_GM1_TELEPHONE_RING 125
#define VS1053_GM1_HELICOPTER 126
#define VS1053_GM1_APPLAUSE 127

#define VS1053_GM1_GM2_BASSDRUM1 36
#define VS1053_GM1_GM2_TAMBOURINE 54

#define MIDI_NOTE_ON  0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_CHAN_MSG 0xB0
#define MIDI_CHAN_BANK 0x00
#define MIDI_CHAN_VOLUME 0x07
#define MIDI_CHAN_PROGRAM 0xC0

#define MIDI_MAX_CHANNEL 7

void midiSetInstrument(uint8_t chan, uint8_t inst);
void midiSetChannelVolume(uint8_t chan, uint8_t vol);
void midiSetChannelBank(uint8_t chan, uint8_t bank);
void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel);
void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel);

#endif
