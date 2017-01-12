// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <cstdlib>
#include <iostream> 
#include <unistd.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h> 
#include <string.h>
#include <execinfo.h>
  
#ifndef TAG_LOGER_H
#define	TAG_LOGER_H


// Номера тегов задаются здесь, а имена тегов и их количество задаётся в TagLoger.cpp
#define Log_Any 0
#define Log_TagLoger 1
#define Log_appConf 2
#define Log_pipeCommands 3
#define Log_benchmark 4
#define Log_removeOldConnectionsLoop 5
#define Log_MySqlServer 6
#define Log_UserIndex 7
#define Log_UserItem 8
#define Log_ClientServer 9
#define Log_ServerServer 10
#define Log_SimpleRedisClient 11
#define Log_tcpServer 12
#define Log_CometQLCluster 13
#define Log_devManager 14
#define Log_FreeswitchServer 15
#define Log_dbLink 16


// Уровни логирования
#define TAGLOG_NONE 0
#define TAGLOG_BACKTRACE 100
#define TAGLOG_ERROR 200
#define TAGLOG_WARN 300
#define TAGLOG_LOG 400
#define TAGLOG_DEBUG 500
#define TAGLOG_ALL 600

// Цвета логирования
#define LogColorBase 0
#define LogColorBlue 1
#define LogColorBBlue 2

#define LogColorBRed 3
#define LogColorRed 4

#define LogColorGreen 5
#define LogColorBGreen 6

#define ifLog(tag) if(TagLoger::isLog(tag, TAGLOG_LOG))
#define ifDebug(tag) if(TagLoger::isLog(tag, TAGLOG_DEBUG))
#define ifLog(tag) if(TagLoger::isLog(tag, TAGLOG_LOG))
#define ifLog(tag) if(TagLoger::isLog(tag, TAGLOG_LOG))
#define ifLog(tag) if(TagLoger::isLog(tag, TAGLOG_LOG))

class TagLoger
{
    static void full_write(int fd, const char *buf, size_t len)
    {
        while (len > 0)
        {
            ssize_t ret = write(fd, buf, len);

            if ((ret == -1) && (errno != EINTR))
                    break;

            buf += (size_t) ret;
            len -= (size_t) ret;
        }
    }
    
        
    TagLoger(){}
    TagLoger(const TagLoger& root);
    TagLoger& operator=(const TagLoger&);

    static int TagsLogLevelSize;
    static int TagsLogLevel[];
    
    static int GloballogLevel;
public:
     
    /**
     * Возвращает массив с текстовыми именами тегов
     * @return 
     */
    static const char** getTagsNames();
     
    /**
     * Возвращает размер массива текстовых имён тегов, тоесть вернёт общее количество тегов в проекте.
     * @return 
     */
    static int countTagsNames();
    
    /**
     * Устанавливает глобальный уровень логирования
     * @param level
     */
    static void setLogLevel(int level);    
        
    /**
     * Вернёт глобальный уровень логирования
     * @param level
     */
    static int getLogLevel();
    
    /**
     * Устанавливает уровень логирования для тега
     * @param tag
     * @param level
     */
    static bool setTagLevel(int tag, int level);  
    
    /**
     * Устанавливает уровень логирования для всех тегов
     * @param tag
     * @param level
     */
    static void setAllTagLevel(int level);      
    
    static int getTagLevel(int tag = Log_Any);
     
    static bool isLog(int tag = Log_Any, int level = TAGLOG_DEBUG);
        
    static bool trace(int tag, int color, const char *format, ...);    
    static bool error(int tag, int color, const char *format, ...);    
    static bool warn(int tag, int color, const char *format, ...);    
    static bool log(int tag, int color, const char *format, ...);    
    static bool debug(int tag, int color, const char *format, ...);
     
protected:
    static bool msg(int tag, int level,const char *format, ...);
};

void printHexMin(const char* data, int len, int logKeyId);
void printHex(const char* data, int len, int logKeyId);
void printHex(const char* data, int len);

#endif	/* TAG_LOGER_H */