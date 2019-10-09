#ifndef BEAT_H
#define BEAT_H

#include "beatmap.h"

typedef struct
{
    double time;    // µ•Œª£∫∫¡√Î
    int type;       // -1.NULL 0.bass 1.clap
} beat_t;

beat_t getNextBeatMillisecondByPosition(beatmap_t *p, timing_t *nowTiming, int pos);

#endif