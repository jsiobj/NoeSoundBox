/*
    Midi mode header
*/
#ifndef PIANO_H
#define PIANO_H PIANO_H

void initPiano(int option);
void onKeyPressedPiano(int keyCode);
void onKeyHoldPiano(int keyCode);
void onKeyReleasedPiano(int keyCode);
void loopPiano();

#endif
