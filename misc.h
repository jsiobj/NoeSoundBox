/*
    Misc. funtions header
*/
#ifndef MISC_H
#define MISC_H

int freeRam(); 
void resetArduino();
void printDirectory(File dir, int numTabs);
void setPWMFreq();
int hex2dec(char hex);

#define ARRAY_LENGTH(x) sizeof(x)/sizeof(typeof(*x))

#endif
