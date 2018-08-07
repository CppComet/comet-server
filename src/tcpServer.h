// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   tcpServer.h
 * Author: levha
 *
 * Created on 24 Апрель 2011 г., 18:42
 */

#include <map>
#include <time.h>
#include <cstdlib>
#include <strings.h>
#include <string.h>

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include "CP.h"
  
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

  
#include "main.h"  

template< class connectionType >
class tcpServer; 

#include "tcpServer_benchmark.h"

#include "thread_data.h"
#include "intervalLoop.h"
#include "appConf.h"
 
#ifndef TCPSERVER_H
#define	TCPSERVER_H


template< class connectionType >
class tcpServer_loop_data
{
    friend class tcpServer<connectionType>;

    int thread_id = 0;
    pthread_t th_id;
    int epfd = 0;
    epoll_event ev;
    int listener = 0;
    tcpServer<connectionType>* parent_server_obj = NULL;
};
 
template< class connectionType >
class tcpServer: public intervalLoopObject
{
    //pthread_mutex_t addClient_mutex;
    //pthread_mutex_t deleteClient_mutex;

    static tcpServer<connectionType>* inst;
 
    const char* key_file = NULL;
    const char* pem_file = NULL;

    //int epfd = 0;     //epoll descriptor to watch events
    int listener = 0;
    struct epoll_event gev;
     
    /**
     * Запрещаем копирование для объектов донного класса
     */
    tcpServer(const tcpServer& ) = delete;
    
    /**
     * Запрещаем копирование для объектов донного класса
     */
    void operator=( const tcpServer& ) = delete;
    
    /**
     * Количество потоков обрабатывающих соединения
     */
    int thread_num = 4;
    
 public:

    void step(int step, thread_data* local_buf);
    
    const char* name = NULL;
    char* host = NULL;
    int port = 80;

    /**
     * Включает или выключает использование ssl
     */
    bool use_ssl = false;

    /**
     * Возможность epoll впервые была представлена в разрабатываемом ядре 2.5.44, а завер-шен интерфейс был в версии ядра 2.5.66.
     *
     *  Параметр size - это подсказка ядру относительно числа дескрипторов файла, за
     *  которыми вы собираетесь наблюдать; это не макси-мальное количество. Чем более
     *  точное значение вы передадите, тем выше будет производительность, но точное
     *  значение не требуется.
     */
    int epoll_size = 10000;
    int backlog = 1024;
    
    int epoll_timeout = -1;
 
    int id = 1; 
    int benchmark = 1;
    tcpServer_benchmark bm;
 
 private:

    /**
     * Указатель на объект обрабатывающий соединения.
     * Повторно используемый
     * Каждый экземпляр гарантировано в один момент времени работает только с одним клиентом.
     */ 
    std::map<int,CP<connectionType>>* map_index;
    /**
     * Чем больше Pool_index_size тем меньше вероятность того что один поток будет ожидать завершения выполнения другого потока в этой секции.
     */
    int map_index_size = 101;
    pthread_mutex_t* request_mutex;

    int map_index_lock(int client_id); 
    int map_index_unlock_by_key(int key);
    
    /**
     *  @todo Заменить std::map на std::unordered_map так как он быстрее http://forum.vingrad.ru/topic-341520.html
     */
    inline std::map<int,CP<connectionType>>& getMapToUserId(unsigned int client_id) const
    { 
        return map_index[client_id%map_index_size];
    }

    tcpServer_loop_data<connectionType> *th = NULL;

    void init();
    
 public:
    tcpServer();
    tcpServer(int Epoll_size, int Epoll_timeout);

    virtual ~tcpServer();

    /**
     * Возвращает объект соединения по его идентификатору, если идентификатор не валидный то у возвращённого объекта функция getID вернёт -1.
     * @param client_id
     * @return
     */
    CP<connectionType> get(int client_id);

    /**
     * Запуск обработки сообщений и пиёма соединений
     */
    bool start(const char* Host,const int Port, const char* server_name);
    bool crash_stop();

    /** 
     * @return количество потоков для обработкии соединений и сообщений
     */
    int get_thread_num();
    
    /**
     * Позволяет изменить количество потоков обработки только до вызова функции start()
     * @param Size новое количество потоков обработки
     * @return true если новое значение установлено, иначе false
     */
    bool set_thread_num(int Size);


    static tcpServer<connectionType>* instance()
    {
       if (tcpServer<connectionType>::inst == 0)
       {
             tcpServer<connectionType>::inst = new tcpServer<connectionType>();
       }
       return tcpServer<connectionType>::inst;
    }

    /**
     * delete new connection to list of clients
     * @param client_id
     */
    void deleteClient(int client_id, thread_data* local_buf);
  
 private:

    /**
     * delete new connection to list of clients
     * @param client_id
     */
    void deleteClient(CP<connectionType> clientObj, thread_data* local_buf);
    void deleteClientFromMap(CP<connectionType> clientObj);
    void closeClientConnection(CP<connectionType> clientObj, thread_data* local_buf);
    
    /**
     * Удаляет соединение из epoll
     */
    int epoll_detached(int fd, int epfd);
  
    int handle_message(CP<connectionType> clientObj,int thread_id, thread_data* local_buf);

    /**
     * Цикл приёма и обработки сооединений и сообщений
     * @param d
     * @return 
     */
    void loop(const tcpServer_loop_data<connectionType>* d);
    
    static void *add_th_loop(void* arg);
    
    
    //static void * removeOldConnectionLoop(void* arg);
    //void removeOldConnectionsLoop();
};
  
#endif	/* TCPSERVER_H */
