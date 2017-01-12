// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* 
 * File:   Freeswitch_connection.h
 * Author: levha
 *
 * Created on 31 Октября 2016 
 */

#ifndef FREESWITCH_CONNECTION_H
#define	FREESWITCH_CONNECTION_H

   #include <stdio.h>
   #include <cstdlib>
   #include <strings.h>
   #include <string.h>
   #include <sys/socket.h>
   #include <vector>
   #include <map>

   #include "main.h"
   #include "CP.h"
   #include "connection.h"

class Freeswitch_connection;
   #include "user_index.h"
 
#define	REQUEST_NULL            0
#define	REQUEST_OPTIONS         1
#define	REQUEST_GET             2
#define	REQUEST_LONG_POLLING    3
#define	REQUEST_POST            3 // Все LONG_POLLING отправляются как POST запросы
#define	REQUEST_WS              4

/**
 * Обрабатывает сообщения от браузеров
 */
class Freeswitch_connection:public connection
{
    friend class tcpServer<Freeswitch_connection>;
       
    /**
     * Для хранения фрагментов сообщений которые пришли не полностью.
     */
    bufferController fragment_buf;
    bool hasFragmentInBuffer = false;
      
    int options(int client, int len, thread_data* local_buf);
    int get_request(int client, int len, thread_data* local_buf); 
         
    int get_directory_request(int client, int len, thread_data* local_buf);
    int connection_type = REQUEST_NULL;
     
  public:
           
    Freeswitch_connection();  
    virtual ~Freeswitch_connection();
     
    int request(int client, int len, thread_data* local_buf); 
    
    /**
     * Устанавливает ствтус соединения
     */
    int set_online(thread_data* local_buf );
    int set_offline(thread_data* local_buf );
     
};


#endif	/* FREESWITCH_CONNECTION_H */

