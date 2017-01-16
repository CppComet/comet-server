// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <stdio.h>
#include <sys/time.h>

#ifndef TAG_TIMER_H
#define	TAG_TIMER_H

#define mTime long long

#define Time_start 1 
 
class TagTimer
{
    static mTime tagTimer[100];
    static mTime tagTimerStart[100];

public:
    
    static void end(int tag);
    
    static void start(int tag);

    static void add(int tag, mTime t);
    
    static mTime mtime();
    
};

#endif	/* TAG_TIMER_H */