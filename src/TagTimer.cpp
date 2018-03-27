// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "TagTimer.h"
 
std::map<std::string, statItem> TagTimer::timeMap; 
bool TagTimer::on = true;
mTime TagTimer::lastReadTime = 0;

pthread_mutex_t TagTimer_mutex;
 
mTime TagTimer::mtime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    long long mt = (long long)t.tv_sec * 1000 + t.tv_usec / 1000;
    return mt;
}

void TagTimer::init()
{
    pthread_mutex_init(&TagTimer_mutex,NULL);
}
 
void TagTimer::add(const char* key, mTime t)
{
    if(!on)
    {
        return;
    }
    
    pthread_mutex_lock(&TagTimer_mutex);
    
    // добавить блокировки для атомарности
    statItem item = timeMap[std::string(key)];
    item.count++; 
    item.time += mtime() - t;
    
    timeMap[std::string(key)] = item; 
    pthread_mutex_unlock(&TagTimer_mutex);
}
 
void TagTimer::lock()
{
    pthread_mutex_lock(&TagTimer_mutex);
    
}

void TagTimer::unlock()
{
    pthread_mutex_unlock(&TagTimer_mutex);
    
}