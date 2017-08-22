// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "TagLoger.h"
#include "appConf.h"

#include <cstdlib>
#include <iostream> 
#include <unistd.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h> 
#include <string.h>
#include <execinfo.h>
   
// Имена тегов.
const char* tagsNames[] = {
    "Any",
    "TagLoger",
    "appConf",
    "pipeCommands",
    "statistics",
    "removeOldConnections",
    "MySqlServer",
    "UserIndex",
    "UserItem",
    "ClientServer",
    "ServerServer",
    "SimpleRedisClient",
    "tcpServer",
    "CometQLCluster",
    "devManager",
    "FreeswitchServer",
    "dbLink"
};

// Общее количество тегов
const int tagsNamesLength = sizeof(tagsNames)/ sizeof(*tagsNames);

#define LOG_EVRY -1

int TagLoger::TagsLogLevelSize = tagsNamesLength;
int TagLoger::GloballogLevel = TAGLOG_ALL;

int TagLoger::TagsLogLevel[tagsNamesLength];
 
    const char** TagLoger::getTagsNames()
    {
        return tagsNames;
    }

    void TagLoger::setLogLevel(int level)
    {
        msg(Log_TagLoger, LOG_EVRY, "setGlobalLogLevel %d\n", level);
        GloballogLevel = level;
    }
    
    int TagLoger::getLogLevel(){
        return GloballogLevel;
    }
    
    
    bool TagLoger::setTagLevel(int tag, int level)
    {
        if(tag >= TagsLogLevelSize || tag < 0)
        {
            msg(Log_TagLoger, TAGLOG_BACKTRACE, "TagLogerError tag=%s, TagsLogLevelSize=%d\n", tagsNames[tag], TagsLogLevelSize);
            return false;
        }
        //printf("setTagLevel %d for tag=%d is %d\n", level, tag, tagsNamesLength);
        //msg(Log_TagLoger, LOG_EVRY, "setTagLevel %d for tag=%d is %d [%s]\n", level, tag, tagsNamesLength, tagsNames[tag]);
        TagsLogLevel[tag] = level;
    }
    
    void TagLoger::setAllTagLevel(int level)
    {
        msg(Log_TagLoger, LOG_EVRY, "setAllTagLevel=%d\n", level);
        for(int i =0; i< tagsNamesLength; i++)
        {
            TagsLogLevel[i] = level;
        }
    }
    
    void TagLoger::initTagLevels()
    { 
        setLogLevel(appConf::instance()->get_int("log", "level"));
        for(int i =0; i< tagsNamesLength; i++)
        {
            setTagLevel(i, appConf::instance()->get_int("log", tagsNames[i])); 
        }
    }
    
    int TagLoger::getTagLevel(int tag)
    { 
        if(tag >= TagsLogLevelSize && tag < 0)
        {
            msg(Log_TagLoger, TAGLOG_BACKTRACE, "TagLogerError tag=%s, TagsLogLevelSize=%d\n", tagsNames[tag], TagsLogLevelSize);
            return -1;
        }
        
        return TagsLogLevel[tag];
    }
     
    bool TagLoger::isLog(int tag, int level)
    { 
        if(tag >= TagsLogLevelSize && tag < 0)
        {
            msg(Log_TagLoger, TAGLOG_BACKTRACE, "TagLogerError tag=%d, TagsLogLevelSize=%d\n", tag, TagsLogLevelSize);
            return false;
        }
         
        if(level == LOG_EVRY)
        {
            return true; 
        }
        
        if(GloballogLevel < level)
        {
            return false; 
        }
        
        if(GloballogLevel == TAGLOG_ALL)
        {
            return true; 
        }
        
        // 0 - все сообщения попадут в лог 
        if(GloballogLevel % 100 != 0)
        {
            if(rand() % 100 >= GloballogLevel % 100 )
            {
                // К примеру GloballogLevel == 103, это значит что в 3% случаев сообщение попадёт в лог.
                return false; 
            }
        }
        
        if(TagsLogLevel[tag] < level)
        {
            return false; 
        }
        
        // 0 - все сообщения попадут в лог 
        if(TagsLogLevel[tag] % 100 != 0)
        {
            if(rand() % 100 >= TagsLogLevel[tag] % 100 )
            {
                // К примеру TagsLogLevel[tag] == 103, это значит что в 3% случаев сообщение попадёт в лог.
                return false; 
            }
        }
        
        return true;
    }
    
    bool TagLoger::msg(int tag, int level,const char *format, ...)
    {
        if(!isLog(tag, level)) return false;
            
            
        if(level == TAGLOG_BACKTRACE)
        {
            void *bt[1024];
            int bt_size;
            char **bt_syms;
            int i;

            bt_size = backtrace(bt, 1024);
            bt_syms = backtrace_symbols(bt, bt_size);
            
            printf("\x1b[1;31m--------- TagLoger-BACKTRACE ---------\x1b[0m\n");
            
            int buffer_size = 2000;
            char buf[2000];
            bzero(buf, buffer_size);
            va_list ap;
            va_start(ap, format);
            vsnprintf(buf, buffer_size, format, ap);
            if(buf[0] == '!')
            {
                printf( "%s", buf+1); 
            }
            else if(buf[strlen(buf)-1] == '\n')
            {
                printf( "[Tag=%s, Level=%d]%s", tagsNames[tag], level, buf); 
            }
            else
            {
                printf( "[Tag=%s, Level=%d]%s\n", tagsNames[tag], level, buf); 
            }
            va_end(ap); 
            
            for (i = 1; i < bt_size; i++) 
            {
                    size_t len = strlen(bt_syms[i]);
                    full_write(STDERR_FILENO, bt_syms[i], len); 
                    printf("\n");
            }
            printf("\x1b[1;31m--------------------------------------\x1b[0m\n");
            free(bt_syms); 
            return true;
        }
         
        
        int buffer_size = 2000;
        char buf[2000];
        bzero(buf, buffer_size);
        va_list ap;
        va_start(ap, format);
        vsnprintf(buf, buffer_size, format, ap);
        if(buf[0] == '!')
        {
            printf( "%s", buf+1); 
        }
        else if(buf[strlen(buf)-1] == '\n')
        {
            printf( "[Tag=%s, Level=%d]%s", tagsNames[tag], level, buf); 
        }
        else
        {
            printf( "[Tag=%s, Level=%d]%s\n", tagsNames[tag], level, buf); 
        }
        va_end(ap); 
        return true;
    }
     
    
    
    bool TagLoger::trace(int tag, int color, const char *format, ...)
    {
        int level = TAGLOG_BACKTRACE; 
        if(!isLog(tag, level))
        {
            return false;
        }
        
        void *bt[1024];
        int bt_size;
        char **bt_syms;
        int i;

        bt_size = backtrace(bt, 1024);
        bt_syms = backtrace_symbols(bt, bt_size);

        printf("\x1b[1;31m--------- TagLoger-BACKTRACE ---------\x1b[0m\n");

        int buffer_size = 2000;
        char buf[2000];
        bzero(buf, buffer_size);
        va_list ap;
        va_start(ap, format);
        vsnprintf(buf, buffer_size, format, ap);
        if(buf[0] == '!')
        {
            printf( "%s", buf+1); 
        }
        else if(buf[strlen(buf)-1] == '\n')
        {
            printf( "[Tag=%s, Level=%d]%s", tagsNames[tag], level, buf); 
        }
        else
        {
            printf( "[Tag=%s, Level=%d]%s\n", tagsNames[tag], level, buf); 
        }
        va_end(ap); 

        for (i = 1; i < bt_size; i++) 
        {
                size_t len = strlen(bt_syms[i]);
                full_write(STDERR_FILENO, bt_syms[i], len); 
                printf("\n");
        }
        printf("\x1b[1;31m--------------------------------------\x1b[0m\n");
        free(bt_syms); 
        return true;
    }
     
    bool TagLoger::error(int tag, int color, const char *format, ...)
    {
        int level = TAGLOG_ERROR; 
        if(!isLog(tag, level))
        {
            return false;
        }
        
        
        int buffer_size = 2000;
        char buf[2000];
        bzero(buf, buffer_size);
        va_list ap;
        va_start(ap, format);
        vsnprintf(buf, buffer_size, format, ap);
        if(buf[0] == '!')
        {
            printf( "%s", buf+1); 
        }
        else if(buf[strlen(buf)-1] == '\n')
        {
            printf( "[Tag=%s, Level=%d]%s", tagsNames[tag], level, buf); 
        }
        else
        {
            printf( "[Tag=%s, Level=%d]%s\n", tagsNames[tag], level, buf); 
        }
        va_end(ap); 
        return true;
    }
    
    bool TagLoger::warn(int tag, int color, const char *format, ...)
    {
        int level = TAGLOG_WARN; 
        if(!isLog(tag, level))
        {
            return false;
        }
        
        int buffer_size = 2000;
        char buf[2000];
        bzero(buf, buffer_size);
        va_list ap;
        va_start(ap, format);
        vsnprintf(buf, buffer_size, format, ap);
        if(buf[0] == '!')
        {
            printf( "%s", buf+1); 
        }
        else if(buf[strlen(buf)-1] == '\n')
        {
            printf( "[Tag=%s, Level=%d]%s", tagsNames[tag], level, buf); 
        }
        else
        {
            printf( "[Tag=%s, Level=%d]%s\n", tagsNames[tag], level, buf); 
        }
        va_end(ap); 
        return true;
    }
    
    bool TagLoger::log(int tag, int color, const char *format, ...)
    {
        int level = TAGLOG_LOG; 
        if(!isLog(tag, level))
        {
            return false;
        }
        
        int buffer_size = 2000;
        char buf[2000];
        bzero(buf, buffer_size);
        va_list ap;
        va_start(ap, format);
        vsnprintf(buf, buffer_size, format, ap);
        
        if(buf[0] == '!')
        {
            printf( "%s", buf+1); 
        }
        else if(buf[strlen(buf)-1] == '\n')
        {
            printf( "[Tag=%s, Level=%d]%s", tagsNames[tag], level, buf); 
        }
        else
        {
            printf( "[Tag=%s, Level=%d]%s\n", tagsNames[tag], level, buf); 
        }
        va_end(ap); 
        return true;
    }
    
    bool TagLoger::debug(int tag, int color, const char *format, ...)
    {
        int level = TAGLOG_DEBUG; 
        if(!isLog(tag, level))
        {
            return false;
        }
        
        int buffer_size = 2000;
        char buf[2000];
        bzero(buf, buffer_size);
        va_list ap;
        va_start(ap, format);
        vsnprintf(buf, buffer_size, format, ap);
        
        if(buf[0] == '!')
        {
            printf( "%s", buf+1); 
        }
        else if(buf[strlen(buf)-1] == '\n')
        {
            printf( "[Tag=%s, Level=%d]%s", tagsNames[tag], level, buf); 
        }
        else
        {
            printf( "[Tag=%s, Level=%d]%s\n", tagsNames[tag], level, buf); 
        }
        va_end(ap); 
        return true;
    }
    
    
void printHexMin(const char* data, int len, int logKeyId)
{ 
    for(int i=0; i < len; i++)
    {
        if(i % 16 == 0)
        {
            TagLoger::log(logKeyId, 0, "!\n%d:\t", i);
        }
        else if(i % 8 == 0) 
        {
            TagLoger::log(logKeyId, 0, "!\t\t");
        }
        
        char c = (char)data[i];
        if(c < 32 ) c = '.';
        TagLoger::log(logKeyId, 0, "!%02x [%c] ",  (unsigned char)data[i], c );
    }
    TagLoger::log(logKeyId, 0, "!\n");
    
}

void printHex(const char* data, int len, int logKeyId)
{
    for(int i=0; i < len; i++)
    {
        if(i % 8 == 0)  TagLoger::log(logKeyId, 0, "!\n%d:\t", i);
        char c = (char)data[i];
        if(c < 32 ) c = '.';
        TagLoger::log(logKeyId, 0, "!%02x [%c] ",  (unsigned char)data[i], c );
    }
    TagLoger::log(logKeyId, 0, "!\n");
    
    for(int i=0; i < len; i++)
    {
        TagLoger::log(logKeyId, 0, "!%02x",  (unsigned char)data[i]);
    }
    TagLoger::log(logKeyId, 0, "!\n");
}

void printHex(const char* data, int len)
{
    return printHex(data, len, Log_Any);
}