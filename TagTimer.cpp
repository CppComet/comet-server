// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "TagTimer.h"


mTime TagTimer::tagTimer[100];
mTime TagTimer::tagTimerStart[100];


void TagTimer::end(int tag)
{
   tagTimer[tag] +=  mtime() - tagTimerStart[tag];
   tagTimerStart[tag] = 0;
} 

void TagTimer::start(int tag)
{
    tagTimerStart[tag] = mtime();
}

void TagTimer::add(int tag, mTime t)
{
    tagTimer[tag] += t;
}

mTime TagTimer::mtime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    long long mt = (long long)t.tv_sec * 1000 + t.tv_usec / 1000;
    return mt;
}
