#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "hashmap.h"

typedef struct
{
    char* acronym;              // ±ÈÈüËõĞ´
    char* blue_team;
    char* red_team;
} tourney_t;

extern hashmap_t* matches;

tourney_t* tourney_new(char* channel, char* name);

#endif