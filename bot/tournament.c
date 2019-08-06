#include <stdlib.h>

#include "tournament.h"
#include "hashmap.h"
#include "strb.h"

hashmap_t* matches;

static void tourney_free(void* data)
{
    if (data == NULL)
        return;

    tourney_t* t = (tourney_t*)data;
    free(t->acronym);
    free(t->blue_team);
    free(t->red_team);

    t = NULL;
    free(data);
}

//-------------------------------------------------------------------------------

tourney_t* tourney_new(char* channel, char* name)
{
    if (matches == NULL)
        matches = hashmap_new(tourney_free);

    tourney_t* t = (tourney_t*)malloc(sizeof(tourney_t));
    if (t == NULL)
        return NULL;

    t->acronym = substr(name, 0, indexof_char(name, ':'));
    t->blue_team = substr(name, indexof_char(name, '(') + 1, indexof_char(name, ')'));
    t->red_team = substr(name, last_indexof_char(name, '(') + 1, last_indexof_char(name, ')'));

    hashmap_put(matches, channel, t);
    return t;
}
