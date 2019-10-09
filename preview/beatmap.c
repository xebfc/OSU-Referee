#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "beatmap.h"

timing_t *getTimingFromPosition2(beatmap_t *p, int pos)
{
    int i = OSU_TIMING_MAX;
    while (--i >= 0)
    {
        timing_t *t = &p->TimingPoints[i];
        if (t->Inherited && t->Offset <= pos)
            return t;
    }
    return NULL;
}

timing_t *getTimingFromPosition(beatmap_t *p, timing_t *prev, int pos)
{
    if (prev == NULL)
        // 没必要使用getFirstTiming，循环查找的时间成本最大为O(n)，甚至还要再次处理NULL。
        // 多数情况下TimingPoints[0]就是第一条红线，即便是绿线也可以（虽然不推荐）。
        prev = &p->TimingPoints[0];

    int i = getTimingIndex(p, prev);
    timing_t *t;

    if (prev->Offset > pos)
    {
        while (--i >= 0)
        {
            t = &p->TimingPoints[i];
            if (t->Inherited && t->Offset <= pos)
                return t;
        }
        t = NULL; // 红线不存在
    }
    else // prev->Offset <= pos
    {
        t = getNextTiming(p, prev);
        if (t == NULL || t->Offset > pos)
        {
            if (prev->Inherited)
                return prev; // prev是红线

            // 当prev是绿线时
            while (--i >= 0 && !p->TimingPoints[i].Inherited);
            return i < 0 ? NULL : &p->TimingPoints[i];
        }

        t = getTimingFromPosition2(p, pos); // t的next不一定是边界
    }

    return t;
}

timing_t *getNextTiming(beatmap_t *p, timing_t *prev)
{
    if (prev == NULL)
        return getFirstTiming(p);

    int i = getTimingIndex(p, prev);
    while (++i < OSU_TIMING_MAX)
    {
        timing_t *t = &p->TimingPoints[i];
        if (t->Inherited)
            return t;
    }
    return NULL;
}

timing_t *getFirstTiming(beatmap_t *p)
{
    int i = 0; // TimingPoints[0]不一定是红线
    while (i < OSU_TIMING_MAX && !p->TimingPoints[i++].Inherited);
    return i < OSU_TIMING_MAX ? &p->TimingPoints[i] : NULL;
}

void loadBeatmap(beatmap_t *p, char *file)
{
    char *s = (char *)malloc(sizeof(char) * NSIZE_MAX) // section
        , l[OSU_COLUMN_MAX]     // line
        , c[OSU_COLUMN_MAX];    // column | cache

    GetPrivateProfileSection("General", s, NSIZE_MAX, file);
    getStringFromSection("AudioFilename", NULL, p->AudioFilename, s);
    p->AudioLeadIn = getIntFromSection("AudioLeadIn", 0, s);

    GetPrivateProfileSection("TimingPoints", s, NSIZE_MAX, file);
    int index = -1;
    while (getLineFromSection(++index, l, s) != -1)
    {
        timing_t *timing = &p->TimingPoints[index];
        timing->Offset = getIntFromLine(0, 0, l);
        getStringFromLine(1, NULL, c, l);
        timing->MillisecondsPerBeat = atof(c);
        timing->Meter = getIntFromLine(2, 0, l);
        timing->SampleSet = getIntFromLine(3, 0, l);
        timing->SampleIndex = getIntFromLine(4, 0, l);
        timing->Volume = getIntFromLine(5, 0, l);
        timing->Inherited = getIntFromLine(6, 0, l);
        timing->KiaiMode = getIntFromLine(7, 0, l);
    }

    GetPrivateProfileSection("Difficulty", s, NSIZE_MAX, file);
    getStringFromSection("SliderTickRate", NULL, c, s);
    p->SliderTickRate = atof(c);

    free(s);
}

int getLineFromSection(int index, char *returnedString, char *section)
{
    char *now = 0, *prev = 0;
    while (TRUE)
    {
        now = strchr(now ? now + 1 : section, '\0');
        // 缓冲区中填充了一个或多个以null终止的字符串；最后一个字符串后跟第二个空字符。
        if (prev && now - prev == 1)
            return -1; // 下标越界

        if (!index--)
            break;
        prev = now;
    }

    char *line = prev ? prev + 1 : section;
    if (returnedString != NULL)
        strcpy(returnedString, line);
    return now - line;
}

int getStringFromSection(char *key, char *defaultString, char *returnedString, char *section)
{
    int i = 0;
    char line[OSU_COLUMN_MAX];
    while (getLineFromSection(i++, line, section) != -1)
    {
        if (strstr(line, key) == NULL)
            continue;

        char *value = strchr(line, ':') + 1;
        while (*value == ' ')
            value++; // lTrim
        if (returnedString != NULL)
            strcpy(returnedString, value);
        return strlen(value);
    }

    if (returnedString != NULL)
        strcpy(returnedString, defaultString);
    return -1;
}

int getIntFromSection(char *key, int defaultInt, char *section)
{
    char num[INT_STR_LMAX];
    if (getStringFromSection(key, NULL, num, section) == -1)
        return defaultInt;
    return atoi(num);
}

int getStringFromLine(int index, char *defaultString, char *returnedString, char *line)
{
    while (TRUE)
    {
        char *part = strchr(line, ',');

        if (!index--)
        {
            if (part == NULL)
                part = strchr(line, '\0');

            int length = part - line;
            if (returnedString != NULL)
            {
                memcpy(returnedString, line, sizeof(char) * length);
                *(returnedString + length) = '\0';
            }
            return length;
        }

        if (part == NULL)
            break; // 下标越界
        line = part + 1;
    }

    if (returnedString != NULL)
        strcpy(returnedString, defaultString);
    return -1;
}

int getIntFromLine(int index, int defaultInt, char *line)
{
    char num[INT_STR_LMAX];
    if (getStringFromLine(index, NULL, num, line) == -1)
        return defaultInt;
    return atoi(num);
}