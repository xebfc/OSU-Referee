#ifndef SOUND_H
#define SOUND_H

#include <windows.h>

#include "bass.h"

HANDLE newSoundFromResource(HMODULE hModule, LPCSTR  lpName, LPCSTR  lpType);
int playSound(HANDLE *h, float vol);
void stopSound(HANDLE *h, int i);
void freeSound(HANDLE *h);

#endif