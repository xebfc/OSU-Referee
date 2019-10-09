#ifndef BEATMAP_H
#define BEATMAP_H

#include <windows.h>
#include <limits.h>

#define NSIZE_MAX 32767 // win32 GetPrivateProfileSection lpReturnedString 允许的最大字符数
#define INT_STR_LMAX 11 // int32 最大位数（int string length max）含符号位
#define OSU_COLUMN_MAX UCHAR_MAX
#define OSU_TIMING_MAX NSIZE_MAX

// beatmap,timing 均为指针
#define getTimingIndex(beatmap, timing) (timing - &beatmap->TimingPoints[0]) / sizeof(timing_t)

#define isEffectEnabled(k, e) ((k & e) != 0)

typedef enum
{
    Effect_None = 0,
    Effect_Kiai = 1,
    OmitFirstBarLine = 8,
} Effect;

typedef struct
{
    int Offset;
    double MillisecondsPerBeat;
    int Meter;
    int SampleSet;
    int SampleIndex;
    int Volume;
    BOOL Inherited;
    int KiaiMode;
} timing_t;

typedef struct
{
    // General
    char AudioFilename[MAX_PATH]; // (String) 音频文件相对于当前文件夹的位置。
    int AudioLeadIn; // (Integer, 毫秒) 延迟播放。对于立即开始的音频文件很有用。
    int PreviewTime; // (Integer, 毫秒) 在选歌菜单中音频文件的起始播放（预览）位置。
    int Countdown; // (Integer) "Are you ready? 3, 2, 1, GO!"倒计时。(0=No countdown, 1=Normal, 2=Half, 3=Double)
    char SampleSet[OSU_COLUMN_MAX]; // (String) 音效组。
    float StackLeniency; //(Decimal) hit objects堆叠频率，大于0时会进行适当的错位显示，仅在play时有效。
    int Mode; // (Integer) 游戏模式。(0=osu!, 1=Taiko, 2=Catch the Beat, 3=osu!mania)
    BOOL LetterboxInBreaks; // (Boolean) 在break期间是否出现上下黑条。
    BOOL StoryFireInFront; // (Boolean) 是否在combo fire（该功能已被废弃）前显示storyboard。
    char SkinPreference[OSU_COLUMN_MAX]; // (String) 首选外观。
    BOOL EpilepsyWarning; // (Boolean) 是否在开头显示"This beatmap contains scenes with rapidly flashing colours..."警告。
    int CountdownOffset; // (Integer) 指定倒计时开始前的节拍数。
    BOOL WidescreenStoryboard; // (Boolean) 指定storyboard是否应为宽屏。
    BOOL SpecialStyle; // (Boolean) 是否使用osu!mania的N+1特殊样式。
    BOOL UseSkinSprites; // (Boolean) 指定storyboard是否可以使用用户的皮肤资源。

    // Difficulty
    double SliderTickRate;

    // TimingPoints
    timing_t TimingPoints[OSU_TIMING_MAX];

} beatmap_t;

/**
* 获取当前位置所处的Timing，数字2表示该函数为两个参数的版本。
*
* @param beatmap_t* beatmap指针
* @param pos 当前播放位置，单位：毫秒
* @return pos所处的Timing，为NULL时表示当前位置不在红线内
*/
timing_t *getTimingFromPosition2(beatmap_t *p, int pos);
timing_t *getTimingFromPosition(beatmap_t *p, timing_t *prev, int pos);
timing_t *getNextTiming(beatmap_t *p, timing_t *prev);
timing_t *getFirstTiming(beatmap_t *p);
void loadBeatmap(beatmap_t *p, char *file);

int getLineFromSection(int index, char *returnedString, char *section);
int getStringFromSection(char *key, char *defaultString, char *returnedString, char *section);
int getIntFromSection(char *key, int defaultInt, char *section);

int getStringFromLine(int index, char *defaultString, char *returnedString, char *line);
int getIntFromLine(int index, int defaultInt, char *line);

#endif