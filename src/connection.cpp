// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   Client_connection.cpp
 * Author: levha
 *
 * Created on 11 Май 2011 г., 23:58
 */

#ifndef CONNECTION_CPP
#define	CONNECTION_CPP


#define SERVER "webserver/1.0"
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define PORT 80


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "connection.h"


void sighand(int s)
{
    TagLoger::log(Log_Any, 0, "::sighand %d::\n",s);
}

connection::connection():isOnLine(false)
{ 
    bzero(ip, 16);
    bzero(&event, sizeof(epoll_event));
}

connection::~connection()
{  
    web_close();
}


long connection::getUptime()
{
    if(start_online_time <= 0)
    {
        return -1;
    }
    
    return time(0) - start_online_time;
}

void connection::setIp(const char* Ip)
{
    strncpy(ip, Ip, 16);
}

const char* connection::getIp() const{
    return ip;
}
  
int connection::web_read(thread_data* local_buf)
{
    local_buf->buf.lock();
    TagLoger::log(Log_Any, 0, "web_read[fd=%d, buffSize=%d]", fd, local_buf->buf.getSize()); 
    int i = read(fd, local_buf->buf.getData(), local_buf->buf.getSize()); 
    return i;
}
 
int connection::web_write(const char *msg)
{ 
    return web_write(msg, strlen(msg)); 
}

int connection::web_write(const char *msg, int len)
{ 
    // https://linux.die.net/man/2/write
    // The adjustment of the file offset and the write operation are performed as an atomic step.
    return write(fd, msg, len);  
}
  
/**
 * Закрывает соединение fd
 * @return 
 */
int connection::web_close()
{
    if(fd < 0)
    {
        return 0;
    }
    
    int r = close(fd); 
    fd = -1; // Иначе мы могли бы ещё и попытаться туда что то отправить.
    return r;
}


#endif	/* CONNECTION_CPP */