// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* 
 * File:   Client_connection.h
 * Author: levha
 *
 * Created on 11 Май 2011 г., 23:58
 */

   #include <stdio.h>
   #include <cstdlib>
   #include <strings.h>
   #include <string.h>
   #include <sys/socket.h>
   #include <pthread.h>
   #include <semaphore.h> 

   #include "tcpServer.h"

#ifndef CONNECTION_H
#define	CONNECTION_H



/** 
 * Родительский класс для класов обработки входящих соединение.
 */
class connection: public CpClass
{
    friend class tcpServer<connection>;
protected:
     
    /**
     * Реальный файловый дескриптор
     * В один момент времени fd уникален.
     */
    int fd = 0; 
    
    /**
     *  epfd потока для удаления клиента
     */
    int epfd = 0; 
       
    /**
     * Содержит текущий статус соединения
     */
    bool isOnLine = false;
     
    /** 
     * Uptime соединения с клиентом, если 0 или -1 то соединение offline
     */
    time_t start_online_time = -1;
    
    char ip[16];
    
public:  
    struct epoll_event event;
    
    connection();  
    //connection(const connection& orig);
    virtual ~connection();
     
    void setIp(const char* ip);
    const char* getIp() const;
    
    /*bool operator>(connection v){return id>v.id; }
    bool operator<(connection v){return id<v.id; }
    bool operator==(connection v){return v.id==id; }
    bool operator!=(connection v){return v.id!=id; }
    
    bool operator>(int Id){return id>Id; }
    bool operator<(int Id){return id<Id; }
    bool operator==(int Id){return Id==id; }
    bool operator!=(int Id){return Id!=id; }*/
      
    /**
     * Устанавливает ствтус соединения
     */
    int set_online(thread_data* local_buf );
    int set_offline(thread_data* local_buf );
     
    /**
     * Возвращает время установления соединения
     */
    inline time_t get_online_time(){ return start_online_time;}
    
    /**
     * Возвращает Uptime соединения
     */
    long getUptime();
    
    
    inline void setfd(int Fd){ fd = Fd;}
    inline int getfd(){ return fd;}
    
    
    inline void setEpfd(int Epfd){ epfd = Epfd;}
    inline int getEpfd(){ return epfd;}
       
    int request(int client,int len, thread_data* local_buf);
    
    int web_read(thread_data* local_buf);
    
    int web_write(const char *msg);
    int web_write(const char *msg, int len);
      
    /**
     * Закрывает соединение fd
     * @return 
     */
    int web_close();
     
    
};

#endif	/* CONNECTION_H */
