// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <stdio.h>
#include <sys/time.h>

#include <map>
#include <string>

#ifndef TAG_TIMER_H
#define	TAG_TIMER_H

#define mTime long long

#define Time_start 1 
#define Time_db_query 2 
 
class statItem{
public:
    mTime time;
    int count;
     
};

class TagTimer
{ 
    static std::map<std::string, statItem> timeMap;  
    
    static bool on;
public:
    static mTime lastReadTime;
    
    static void setStatus(bool status)
    {
        on = status;
    }
    
    static void init(); 
    static void add(const char* key, mTime t); 
    static mTime mtime();
    
    static void lock(); 
    static void unlock(); 
    static std::map<std::string, statItem> get()
    {
        return timeMap;
    }
    
};

#endif	/* TAG_TIMER_H */