#include <windows.h>

#include "sound.h"
#include "bass.h"

#define MAX_SOUNDS 8
#define NEW_STREAM(file, length) BASS_StreamCreateFile(TRUE, file, 0, length, 0);

typedef struct
{
    HSTREAM sounds[MAX_SOUNDS];
    int soundIndex;
    LPVOID soundFile;
    DWORD soundSize;
} sound_t;

HANDLE newSoundFromResource(HMODULE hModule, LPCSTR  lpName, LPCSTR  lpType)
{
    sound_t *p = (sound_t *)malloc(sizeof(sound_t));
    if (p == NULL)
        return p;

    HRSRC h = FindResource(hModule, lpName, lpType);
    p->soundFile = LockResource(LoadResource(hModule, h));
    if (p->soundFile == NULL)
    {
        free(p);
        return NULL;
    }

    memset(p->sounds, NULL, sizeof(HSTREAM) * MAX_SOUNDS);
    p->soundSize = SizeofResource(hModule, h);
    //p->sounds[0] = NEW_STREAM(p->soundFile, p->soundSize);
    p->sounds[0] = BASS_SampleLoad(TRUE, p->soundFile, 0, p->soundSize, 65535, BASS_SAMPLE_OVER_POS);
    p->soundIndex = 0;
    return p;
}

// Let's find a sound that isn't currently playing.
// @return The index of the sound or -1 if none is available.
static int findAvailableSound(sound_t *p)
{
    for (int i = 0; i < MAX_SOUNDS; i++)
    {
        int index = (p->soundIndex + i) % MAX_SOUNDS;
        if (p->sounds[index] == NULL || !BASS_ChannelIsActive(p->sounds[index]))
        {
            // point to the next likely free player
            p->soundIndex = (index + 1) % MAX_SOUNDS;

            // return the free player
            return index;
        }
    }

    // all are busy playing, stop the next sound in the queue and reuse it
    int index = p->soundIndex % MAX_SOUNDS;
    p->soundIndex = (index + 1) % MAX_SOUNDS;
    return index;
}

int playSound(HANDLE *h, float vol)
{
    sound_t *p = (sound_t *)h;
    /*
    int soundId = findAvailableSound(p);
    if (soundId >= 0)
    {
        HSTREAM sound = p->sounds[soundId];
        if (sound == NULL)
        {
            sound = NEW_STREAM(p->soundFile, p->soundSize);
            p->sounds[soundId] = sound;
        }
        BASS_ChannelSetAttribute(sound, BASS_ATTRIB_VOL, vol);
        BASS_ChannelPlay(sound, FALSE);
    }
    return soundId;
    */

    HCHANNEL c = BASS_SampleGetChannel(p->sounds[0], FALSE);
    BASS_ChannelSetAttribute(c, BASS_ATTRIB_VOL, vol);
    BASS_ChannelSetAttribute(c, BASS_ATTRIB_PAN, 0);
    BASS_ChannelFlags(c, 0, BASS_SAMPLE_LOOP);
    BASS_ChannelPlay(c, FALSE);
}

void stopSound(HANDLE *h, int i)
{
    sound_t *p = (sound_t *)h;
    if (i == -1)
    {
        while (++i < MAX_SOUNDS)
            BASS_ChannelStop(p->sounds[i]);
        return;
    }
    BASS_ChannelStop(p->sounds[i]);
}

void freeSound(HANDLE *h)
{
    sound_t *p = (sound_t *)h;
    /*
    for (int i = 0; i < MAX_SOUNDS; i++)
        BASS_StreamFree(p->sounds[i]);
    */
    BASS_SampleFree(p->sounds[0]);
    free(p);
}