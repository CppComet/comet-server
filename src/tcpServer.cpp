// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   tcpServer.cpp
 * Author: levha
 *
 * Created on 24 Апрель 2011 г., 18:42
 */

#include "main.h"
#include "tcpServer.h"
#include "user_index.h"
#include "connection.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <errno.h>

#include <unordered_map> // @todo Перейти с std::map на std::unordered_map
 
using namespace std;
 
#ifndef TCPSERVER_CPP
#define	TCPSERVER_CPP
 
template< class connectionType >
tcpServer<connectionType> * tcpServer<connectionType>::inst = NULL;
 
template< class connectionType >
void * tcpServer<connectionType>::add_th_loop(void* arg)
{
    struct tcpServer_loop_data<connectionType> *tinfo = (struct tcpServer_loop_data<connectionType> *) arg;
    TagLoger::debug(Log_tcpServer, 0, "\x1b[32mS%d:A new thread has been created for processing messages\x1b[0m\n",tinfo->parent_server_obj->id);

    tinfo->parent_server_obj->loop(tinfo);
    TagLoger::debug(Log_tcpServer, 0, "\x1b[32mS%d:Terminated thread for processing messages\x1b[0m\n",tinfo->parent_server_obj->id);
 
    return 0;
}

template< class connectionType >
tcpServer<connectionType>::tcpServer()
{
    bzero(&gev, sizeof(gev));
    init();
}

template< class connectionType >
tcpServer<connectionType>::tcpServer(int Epoll_size, int Epoll_timeout):epoll_size(Epoll_size),epoll_timeout(Epoll_timeout)
{ 
    bzero(&gev, sizeof(gev));
    init();
}

/**
 * @return количество потоков для обработкии соединений и сообщений
 */
template< class connectionType >
int tcpServer<connectionType>::get_thread_num()
{
    return thread_num;
}

/**
 * Позволяет изменить количество потоков обработки только до вызова функции start()
 * @param Size новое количество потоков обработки
 * @return true если новое значение установлено, иначе false
 */
template< class connectionType >
bool tcpServer<connectionType>::set_thread_num(int Size)
{
    if(Size < 1)
    {
        Size = 1;
    }

    if(th == NULL)
    {
        thread_num = Size+1;
        return true;
    }
    return false;
}

template< class connectionType >
tcpServer<connectionType>::~tcpServer()
{
    if(host != NULL)
    {
        delete[] host;
    }

    if(th != NULL)
    {
        delete[] th;
    } 
    
    
    for(int i=0; i<map_index_size+1; i++ )
    {
        pthread_mutex_destroy(&request_mutex[i]);
    }

    delete[] request_mutex;

    for(int i=0; i<map_index_size+1; i++ )
    {
        auto it = map_index[i].begin();
        while(it != map_index[i].end())
        {
            delete it->second;
        }

        map_index[i].clear();
    }

    delete[] map_index;
}

template< class connectionType >
void tcpServer<connectionType>::init()
{
    try
    {
        map_index = new std::map<int,CP<connectionType>>[map_index_size+1];
    }
    catch (std::bad_alloc& ba)
    {
      TagLoger::error(Log_UserIndex, 0, "\x1b[31mbad_alloc caught\x1b[0m\n" ); 
    }

    request_mutex = new pthread_mutex_t[map_index_size+1];

    for(int i=0; i<map_index_size+1; i++ )
    {
        pthread_mutex_init(&request_mutex[i],NULL);
    }
}

template< class connectionType >
bool tcpServer<connectionType>::start(const char* Host,const int Port, const char* server_name)
{
    name = server_name;
    if(benchmark) bm.start(this->id,thread_num, server_name);
  
    #ifdef EPOL_CTL_DISABLE
        TagLoger::debug(Log_tcpServer, 0, "\x1b[1;33mUsing EPOL_CTL_DISABLE\x1b[0m\n");
    #else
        TagLoger::debug(Log_tcpServer, 0, "\x1b[1;33mUsing EPOLL_CTL_DEL\x1b[0m\n");
    #endif

    host= new char[200];
    strncpy(host,Host, 200);
    port=Port;

        // define ip & ports for server(addr)
        //     and incoming client ip & ports(their_addr)
        struct sockaddr_in addr;
        bzero(&addr, sizeof(addr));
        //     configure ip & port for listen

        // Структура sockaddr_in описывает сокет для работы с протоколами IP.
        addr.sin_family = PF_INET;  //  для сетевого протокола IPv4,  PF_INET6 для IPv6
        addr.sin_port = htons(port); //  Поле sin_port содержит номер порта который намерен занять процесс. Если значение этого поля равно нулю, то операционная система сама выделит свободный номер порта для сокета.

        // inet_addr - Для перевода адреса в целое число из строкового представления
        addr.sin_addr.s_addr = inet_addr(host); // содержит IP адрес к которому будет привязан сокет

       /**
        * Элемент events является набором битов, созданном с помощью следующих возможных типов событий:
        * EPOLLIN  - Ассоциированный файл доступен для операций read
        * EPOLLOUT - Ассоциированный файл доступен для операций write
        * EPOLLPRI - Нет срочных данных, доступных для операций read
        * EPOLLERR - Произошла ошибка на ассоциированном описателе файлов.
        * EPOLLHUP - На ассоциированном описателе файлов произошло зависание.
        * EPOLLET  - Устанавливает поведение Edge Triggered для ассоциированного описателя файлов. (По умолчанию Level Triggered)
        * EPOLLRDHUP - Одна из сторон потокового сокета закрыла соединение, или выключила записывающую часть соединения. (Этот флаг особенно полезен при написании простого кода для обнаружения отключения стороны с помощью слежения Edge Triggered.)
        * EPOLLONESHOT - Установить однократное получение для связанного файлового дескриптора.
        *                Это означает, что после извлечения события с помощью epoll_wait(2) со связанным дескриптором приём отключается и о других событиях интерфейс epoll сообщать не будет.
        *                Пользователь должен вызвать epoll_ctl() с операцией EPOLL_CTL_MOD для переустановки новой маски событий для файлового дескриптора.
        *
        *
        * Режимы работы. (http://ru.wikipedia.org/wiki/Epoll)
        * epoll позволяет работать в двух режимах:
        * edge-triggered — файловый дескриптор с событием возвращается только если с момента последнего возврата произошли новые события (например, пришли новые данные)
        * level-triggered — файловый дескриптор возвращается, если остались непрочитанные/записанные данные
        * Если приложение прочитало из файлового дескриптора только часть доступных для чтения данных, то при следующем вызове:
        * в edge-triggered файловый дескриптор не будет возвращён до тех пор, пока в дескрипторе не появятся новые данные;
        * в level-triggerd файловый дескриптор будет возвращаться до тех пор, пока не прочитаны все «старые» данные (и новые, если таковые придут)
        */

        //     event template for epoll_ctl(ev)
        //     storage array for incoming events from epoll_wait(events)
        //     and maximum events count could be epoll_size

        // struct epoll_event ev;
        //     watch just incoming(EPOLLIN)
        //     and Edge Trigged(EPOLLET) events
 
        gev.events = EPOLLHUP | EPOLLIN | EPOLLERR | EPOLLRDHUP; // !|EPOLLET  !|EPOLLRDHUP  --  |EPOLLOUT | EPOLLRDNORM |EPOLLRDBAND|EPOLLWRNORM | EPOLLWRNORM | EPOLLWRBAND |EPOLLMSG |EPOLLERR|EPOLLHUP|EPOLLRDHUP|EPOLLONESHOT



       /**
        * socket() создает конечную точку соединения и возвращает дескриптор. socket() принимает три аргумента:
        * domain, указывающий семейство протоколов создаваемого сокета. Например:
        * PF_INET для сетевого протокола IPv4 или
        * PF_INET6 для IPv6.
        * PF_UNIX для локальных сокетов (используя файл).
        * type (тип) один из:
        * SOCK_STREAM (надежная потокоориентированная служба (сервис) или потоковый сокет)
        * SOCK_DGRAM (служба датаграмм или датаграммный сокет)
        * SOCK_SEQPACKET (надежная служба последовательных пакетов) или
        * SOCK_RAW (сырой протокол поверх сетевого уровня).
        * protocol определяет используемый транспортный протокол. 0 - для выбора протокола по умолчанию из указанного семейства (domain) и типа (type).
        * Функция возвращает −1 в случае ошибки. Иначе, она возвращает целое число, представляющее присвоенный дескриптор.
        */
        listener = socket(PF_INET, SOCK_STREAM, 0);
        if(listener < 0)
        {
            printf("\x1b[1;31mService:%s host:%s port:%d\x1b[0m\n",server_name, Host, Port);
            perror("eval");
            exit(-1);
        }



        TagLoger::log(Log_tcpServer, 0, "S%d:Main listener(fd=%d) created\n", id,listener);

        // Устанавливает часть флагов, относящихся к состоянию файла, согласно значению, указанному в 3 аргументе
        // setup nonblocking socket
        if( fcntl(listener, F_SETFL, fcntl(listener, F_GETFD, 0)|O_NONBLOCK ) < 0 ) // | SO_REUSEADDR  | SOCK_CLOEXEC | SO_REUSEPORT - https://habrahabr.ru/post/259403/
        {
            printf("\x1b[1;31mService:%s host:%s port:%d\x1b[0m\n",server_name, Host, Port);
            perror("eval");
            exit(-1);
        }
 
        int yes = 1;
        // Устанавливает возможность запуска при ещё не освободившемя сокете, уменьшая время старта после падения. Устраняет проблему "Address Already in Use"
        if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        {
            printf("\x1b[1;31mCannot set SO_REUSEADDR option on listen socket (%s)\x1b[0m\n", strerror(errno));
        }
 
        if(setsockopt(listener, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(yes)) < 0)
        {
            printf("\x1b[1;31mCannot set TCP_NODELAY option on listen socket (%s)\x1b[0m\n", strerror(errno));
        }
        
        //    bind listener to address(addr)
        /**
         * Связывает сокет с конкретным адресом
         * bind() принимает три аргумента:
         * sockfd — дескриптор, представляющий сокет при привязке
         * serv_addr — указатель на структуру sockaddr, представляющую адрес, к которому привязываем.
         * addrlen — поле socklen_t, представляющее длину структуры sockaddr.
         * Возвращает 0 при успехе и −1 при возникновении ошибки.
         */

        while(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) != 0 )
        {
           printf("\x1b[1;31mService:%s host:%s port:%d\x1b[0m\n",server_name, Host, Port);
           perror("eval");
           //exit(-1);
           sleep(1);
        }

       TagLoger::log(Log_tcpServer, 0, "S%d:Listener binded to: %s\n", id, host);

        //    start to listen connections
        /**
         * Подготавливает привязываемый сокет к принятию входящих соединений
         * sockfd — корректный дескриптор сокета.
         * backlog — целое число, означающее число установленных соединений, которые могут быть обработаны в любой момент времени
         */
        if(listen(listener, backlog) != 0 )
        { 
            printf("\x1b[1;31mService:%s host:%s port:%d\x1b[0m\n",server_name, Host, Port);
            perror("eval");
            exit(-1); 
        }
        TagLoger::log(Log_tcpServer, 0, "Start to listen: %s\n", host);

        //     set listener to event template
        gev.data.fd = listener;


        // *** Setup epoll
        //     create epoll descriptor
        //     and backup store for EPOLL_SIZE of socket events

        int epfd = epoll_create(epoll_size);
        if(epfd < 0 )
        {
            printf("\x1b[1;31mService:%s host:%s port:%d\x1b[0m\n",server_name, Host, Port);
            perror("eval"); exit(-1);
        }

        TagLoger::log(Log_tcpServer, 0, "Epoll(fd=%d) created\n", epfd);

        //     add listener to epoll
        //     epoll_ctl - интерфейс управления описателями epoll
        /**
         * int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
         * Корректными значениями для параметра op являются:
         * EPOLL_CTL_ADD - Добавить целевой описатель файла fd в описатель epoll - epfd и ассоциировать событие event с внутренним файлом, связанном с fd.
         * EPOLL_CTL_MOD - Изменить событие event связанное с целевым описателем файла fd.
         * EPOLL_CTL_DEL - Убрать целевой описатель файла fd из описателя epoll - epfd.
         */
        if(epoll_ctl(epfd, EPOLL_CTL_ADD, listener, &gev) != 0 )
        {
            printf("\x1b[1;31mService:%s host:%s port:%d\x1b[0m\n",server_name, Host, Port);
            perror("eval"); exit(-1);
        }

        TagLoger::log(Log_tcpServer, 0, "Main listener(%d) added to epoll\n", epfd);
         
        if(appConf::instance()->get_int(name, "uptimeTestInterval") > 0)
        {
            intervalLoop::instance()->add((intervalLoopObject*)this);
        }
        
        // Создаём потоки обработки соединений и сообщений
        th = new tcpServer_loop_data<connectionType>[thread_num];
        for(int i=0; i<thread_num; i++)
        {
            if(i == 0)
            {
                // Поток номер 0 занимается обработкой входящих соединений
                th[i].epfd = epfd;
            }
            else
            {
                // Остальные потоки занимаются обработкой входящих сообщений
                int c_epfd = epoll_create(epoll_size);
                if(c_epfd < 0 )
                {
                    printf("\x1b[1;31mService:%s host:%s port:%d\x1b[0m\n",server_name, Host, Port);
                    perror("eval"); exit(-1);
                }
                TagLoger::log(Log_tcpServer, 0, "Epoll(fd=%d) created\n", c_epfd);
                th[i].epfd = c_epfd;
            }

            th[i].thread_id = i;
            th[i].ev = gev;
            th[i].listener = listener;
            th[i].parent_server_obj=this;
 
            pthread_create(&th[i].th_id, NULL, add_th_loop , (void *)&th[i]);
        }

        // Создаёт поток для удаления соединений у которых слишком большой uptime
        //tcpServer_loop_data<connectionType> th_OldConnectionLoop;
        //th_OldConnectionLoop.parent_server_obj=this;
        //pthread_create(&th_OldConnectionLoop.th_id, NULL, removeOldConnectionLoop , (void *)&th_OldConnectionLoop);

        /*
        for(int i=0; i<thread_num; i++)
        {
             TagLoger::log(Log_tcpServer, 0, "pthread_join %d\n",i);
             pthread_join(th[i].th_id, NULL);
             TagLoger::log(Log_tcpServer, 0, " pthread_join %d\n\n",i);
        }
        close(listener);
        close(epfd);*/

        return true;
}

template< class connectionType >
void tcpServer<connectionType>::step(int uptime, thread_data* local_buf)
{
    if(uptime % appConf::instance()->get_int(name, "uptimeTestInterval") != 0)
    {
        // выполнять один раз в 120 секунд
        return;
    }

    int maxUptime = appConf::instance()->get_int(name, "maxUptime");
    
    int onlineSum = 0;
    // Пробегает по всем конектам и проверяет не порали отключить по истичению uptime
    for(int i =0; i< map_index_size; i++)
    {
        if(maxUptime > 0)
        {
            pthread_mutex_lock(&request_mutex[i]);
            auto it = map_index[i].begin(); 
            while( it != map_index[i].end())
            { 
                CP<connectionType> clientObj = it->second;
                if(!clientObj.isNULL())
                {
                    if(clientObj->getUptime() > maxUptime)
                    {
                        it = map_index[i].erase(it);
                        closeClientConnection(clientObj, local_buf);
                    }
                    else
                    {
                        it++;
                    }
                }
                else
                {
                    it++;
                } 
            }
            pthread_mutex_unlock(&request_mutex[i]);
        } 
        onlineSum+= map_index[i].size();
    } 
    
    // Правим значение колва онлайн так как сщётчик мог накопить погрешность.
    if(benchmark) bm.setConections(onlineSum);
}

template< class connectionType >
bool tcpServer<connectionType>::crash_stop()
{
    TagLoger::log(Log_tcpServer, 0, "\x1b[1;31mcrash stop\x1b[0m\n");
    close(listener);
    //close(epfd);
    return true;
}

void sighand(int s);

/**
 * Цикл приёма и обработки сооединений и сообщений
 * @param d
 * @return
 */
template< class connectionType >
void tcpServer<connectionType>::loop(const tcpServer_loop_data<connectionType>* d)
{
    bm.set_th_status(d->thread_id,'-');
    TagLoger::log(Log_tcpServer, 0, "S%d:A new thread has been started to process messages on port %d with the identifier %d\n",this->id,port,d->thread_id);

    thread_data* hm_thread_data = new thread_data(appConf::instance());
    hm_thread_data->bm = &bm;
    hm_thread_data->thread_id = d->thread_id;
    
    struct epoll_event loop_ev = d->ev;
    struct sockaddr_in their_addr;                      // define ip & ports for server(addr) and incoming client ip & ports(their_addr)
    socklen_t socklen = sizeof(struct sockaddr_in);     // size of address

    struct epoll_event* events = new epoll_event[epoll_size];

    int epfd = d->epfd;
    int next_epfd_index = 0;
    

    while(1)
    {
        TagLoger::log(Log_tcpServer, 0, "S%d:epoll_wait to epfd = %d, thread_id=%d\n",this->id,epfd, d->thread_id);
        bm.set_th_status(d->thread_id,'0');

            TagLoger::log(Log_tcpServer, 0, "S%d:Epoll epoll_wait start: %d\n", this->id, epfd);
            int epoll_events_count = epoll_wait(epfd, events, epoll_size, epoll_timeout);

            if(epoll_events_count < 0 )
            {
                TagLoger::error(Log_tcpServer, 0, "\x1b[31m[Critical error]epoll_events_count %d\x1b[0m\n",epoll_events_count);
                bm.set_th_status(d->thread_id,'X');
                perror("epoll_wait");
                continue;
            }
            else
            {
                bm.set_th_status(d->thread_id,'1');
            }

            TagLoger::log(Log_tcpServer, 0, "Epoll events count: %d, thread_id=%d\n", epoll_events_count, d->thread_id);

        bm.add_th_count(d->thread_id,epoll_events_count);
        
        for(int i = 0; i < epoll_events_count ; i++)
        {
            if(TagLoger::isLog(Log_tcpServer, TAGLOG_LOG) )
            {
                TagLoger::log(Log_tcpServer, 0, "events[%d].data.fd = %d\n", i, events[i].data.fd);
                TagLoger::log(Log_tcpServer, 0, "fd(%d), ev.events:", events[i].data.fd);

                if(events[i].events & EPOLLIN)
                        TagLoger::log(Log_tcpServer, 0, " \x1b[32mEPOLLIN\x1b[0m ");
                if(events[i].events & EPOLLOUT)
                        TagLoger::log(Log_tcpServer, 0, " \x1b[32mEPOLLOUT\x1b[0m ");
                if(events[i].events & EPOLLET)
                        TagLoger::log(Log_tcpServer, 0, " EPOLLET ");
                if(events[i].events & EPOLLPRI)
                        TagLoger::log(Log_tcpServer, 0, " EPOLLPRI ");
                if(events[i].events & EPOLLRDNORM)
                        TagLoger::log(Log_tcpServer, 0, " EPOLLRDNORM ");
                if(events[i].events & EPOLLRDBAND)
                        TagLoger::log(Log_tcpServer, 0, " EPOLLRDBAND ");
                if(events[i].events & EPOLLWRNORM)
                        TagLoger::log(Log_tcpServer, 0, " EPOLLRDNORM ");
                if(events[i].events & EPOLLWRBAND)
                        TagLoger::log(Log_tcpServer, 0, " EPOLLWRBAND ");
                if(events[i].events & EPOLLMSG)
                        TagLoger::log(Log_tcpServer, 0, " EPOLLMSG ");
                if(events[i].events & EPOLLONESHOT)
                        TagLoger::log(Log_tcpServer, 0, " EPOLLONESHOT ");
                if(events[i].events & EPOLLERR)
                    TagLoger::log(Log_tcpServer, 0, " \x1b[1;31mEPOLLERR\x1b[0m ");
                if(events[i].events & EPOLLHUP)
                    TagLoger::log(Log_tcpServer, 0, " \x1b[1;31mEPOLLHUP\x1b[0m "); 
            }
 
            if(events[i].events & EPOLLERR  )
            {
                TagLoger::log(Log_tcpServer, 0, " \x1b[32mdeleteClient EPOLLERR\x1b[0m\n");
                deleteClient(CP<connectionType>((connectionType*)events[i].data.ptr), hm_thread_data);
                continue;
            }

            if(events[i].events & EPOLLHUP  )
            {
                TagLoger::log(Log_tcpServer, 0, " \x1b[32mdeleteClient EPOLLHUP\x1b[0m\n");
                deleteClient(CP<connectionType>((connectionType*)events[i].data.ptr), hm_thread_data);
                continue;
            }

            if(events[i].events & EPOLLRDHUP)
            {
                TagLoger::log(Log_tcpServer, 0, " \x1b[31mdeleteClient EPOLLRDHUP\x1b[0m\n");
                deleteClient(CP<connectionType>((connectionType*)events[i].data.ptr), hm_thread_data);
                continue;
            }

            if(events[i].data.fd ==  listener)
            {
                // Входящие соединение

                TagLoger::log(Log_tcpServer, 0, "th(%d) events[i].data.fd[%d] == listener[%d]\n",d->thread_id, events[i].data.fd, listener);
                /**
                 * accept() используется для принятия запроса на установление соединения от удаленного хоста.
                 * Принимает следующие аргументы:
                 * sockfd — дескриптор слушающего сокета на принятие соединения.
                 * cliaddr — указатель на структуру sockaddr, для принятия информации об адресе клиента.
                 * addrlen — указатель на socklen_t, определяющее размер структуры, содержащей клиентский адрес и переданной в accept(). Когда accept() возвращает некоторое значение, socklen_t указывает сколько байт структуры cliaddr использовано в данный момент.
                 * Функция возвращает дескриптор сокета, связанный с принятым соединением, или −1 в случае возникновения ошибки.
                 *
                 * @note Дескриптор, который равен нулю в *nix системах, считается легальным, а в Windows - нет
                 */
                int client_id=accept( listener, (struct sockaddr *) &their_addr, &socklen);
                if(client_id < 0 )
                {
                    perror("Could not connect in accept in server loop"); 
                    TagLoger::error(Log_tcpServer, 0, "\x1b[31mS%d:Could not connect %d (errno=%d)\x1b[0m",this->id, client_id, errno); 
                    continue;
                }

                TagLoger::log(Log_tcpServer, 0, "connection from:%s:%d, socket assigned to:%d \n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port),  client_id);
                /*if( fcntl(client_id, F_SETFL, fcntl(client_id, F_GETFD, 0)|O_NONBLOCK) < 0 )
                {
                    TagLoger::error(Log_tcpServer, 0, "\x1b[31msetup nonblocking socket faile\x1b[31m");
                    close(client_id);
                    continue;
                }*/
 
                try{
                    CP<connectionType> clientObj;   
                    clientObj->event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP; // events[i].events;
                    clientObj->event.data.fd = client_id;  
                    clientObj->event.data.ptr = (void*)clientObj.get();

                    // Решает в какой из потоков обработки сообщений добавить соединение, раскидывает каждому потоку поровну входящих соединений
                    if(next_epfd_index < thread_num - 1)
                    {
                        next_epfd_index++;
                    }
                    else
                    {
                        next_epfd_index = 1;
                    } 
                    int c_epfd = th[next_epfd_index].epfd;

                    //addClient(client_id, client_id, hm_thread_data, c_epfd); 
                    TagLoger::debug(Log_tcpServer, 0, "S%d:A new client is created %d=%d...",this->id,client_id,clientObj->event.data.fd);

                    clientObj->setIp(inet_ntoa(their_addr.sin_addr)); 
                    clientObj->setEpfd(c_epfd);
                    clientObj->setfd(client_id);
                    
                    // Выполняется в контексте потока приёма входящих сообщений
                    clientObj->set_online(hm_thread_data);  

                    // Вставка объекта соединения в std::map чтоб объект можно было найти по его client_id
                    pthread_mutex_lock(&request_mutex[client_id%map_index_size]); 
                    getMapToUserId(client_id).insert(std::pair<int, CP<connectionType>>(client_id, clientObj));  
                    pthread_mutex_unlock(&request_mutex[client_id%map_index_size]);

                    if(benchmark) bm.increment_addClient();

                    TagLoger::log(Log_tcpServer, 0, "th(%d) epoll_ctl(c_epfd:%d, next_epfd_index:%d)\n",d->thread_id, c_epfd, next_epfd_index); 
                    if(epoll_ctl(c_epfd, EPOLL_CTL_ADD, client_id, &clientObj->event) != 0)
                    {
                        perror("Add new client to epoll - faile");
                        TagLoger::log(Log_tcpServer, 0, "\x1b[31madd new client to epoll - faile (errno=%d)\x1b[0m\n", errno);
                        close(client_id);
                        continue;
                    }
                }catch(...)
                {
                    TagLoger::warn(Log_tcpServer, 0, "S%d:Fail in establish new connection");
                    continue;
                } 
            }
            else
            {
                // Новое сообщение
                TagLoger::log(Log_tcpServer, 0, "th(%d) events[i].data.fd[%d] != listener[%d]\n", d->thread_id, events[i].data.fd, listener);
                if( handle_message(CP<connectionType>((connectionType*)events[i].data.ptr), d->thread_id, hm_thread_data) < 0 )
                {
                    TagLoger::log(Log_tcpServer, 0, "\x1b[31mhandle_message return -1\x1b[0m\n");
                }
            }
        }

        TagLoger::log(Log_tcpServer, 0, "------------------------------------------\n");
    }

    delete[] events;
}

    /**
     * Удаляет соединение из epoll
     */
template< class connectionType >
int tcpServer<connectionType>::epoll_detached(int fd, int epfd)
{
    #ifdef EPOL_CTL_DISABLE
        // EPOL_CTL_DISABLE (добавлена в Linux 3.7) — для безопасного отключения наблюдения за файловым дескриптором в многопоточных приложениях
        return epoll_ctl ( epfd, EPOL_CTL_DISABLE, fd, &gev );
    #else
        // EPOLL_CTL_DEL удаление файлового дескриптора из наблюдения
        return epoll_ctl ( epfd, EPOLL_CTL_DEL, fd, &gev );
    #endif
}
  
/**
 * Удаляет соединение из std::map<int,CP<connectionType>>
 * Таким образом если ссылок кроме как в std::map не останется то объект будет реально удалён из памяти
 * @param client_id
 * @param local_buf
 *  
 */
template< class connectionType >
void tcpServer<connectionType>::deleteClientFromMap(CP<connectionType> clientObj)
{  
    TagLoger::log(Log_tcpServer, 0, "S%d:tcpServer::delete-Client Epfd=%d, fd=%d\n",this->id, clientObj->getEpfd(), clientObj->getfd()); 
    pthread_mutex_lock(&request_mutex[clientObj->getfd()%map_index_size]);

    auto it = getMapToUserId(clientObj->getfd()).find(clientObj->getfd());
    if( it != getMapToUserId(clientObj->getfd()).end() )
    {
        getMapToUserId(clientObj->getfd()).erase(it);
    }
    else
    {
        TagLoger::error(Log_tcpServer, 0, "\x1b[31mS%d:Could not find user %d in map_index\x1b[0m\n",this->id, clientObj->getfd());
    }

    pthread_mutex_unlock(&request_mutex[clientObj->getfd()%map_index_size]); 
}

/**
 * Закрывает сетевое соединение клиента.
 * @param client_id
 * @param local_buf 
 */
template< class connectionType >
void tcpServer<connectionType>::closeClientConnection(CP<connectionType> clientObj, thread_data* local_buf)
{  
    TagLoger::log(Log_tcpServer, 0, "S%d:tcpServer::closeClientConnection Epfd=%d, fd=%d\n",this->id, clientObj->getEpfd(), clientObj->getfd()); 
    /**
     * Если не ошибаюсь, то после этой строчки событий от клиента в epoll не будет.
     * И поэтому будет можно безопасно удалить память на которую указывает events[i].data.ptr в событиях epoll
     */
    epoll_detached(clientObj->getfd(), clientObj->getEpfd());
    
    
    if(clientObj->set_offline(local_buf) < 0 )
    {
        TagLoger::error(Log_tcpServer, 0, "\x1b[31mS%d:Failed to disconnect client %d\x1b[0m\n",this->id, clientObj->getfd());
    }
    else
    {
        TagLoger::debug(Log_tcpServer, 0, "S%d:Succeeded set_online false\n",this->id);
    }
      
    /**
     * @fixme Если начнёт утекать память то надо проверить этот момент.
     */
    //delete clientObj; 
    if(benchmark) bm.increment_deleteClient();
    TagLoger::debug(Log_tcpServer, 0, "S%d:Succeeded disconnect client\n",this->id);
}

/**
 * Удаляет соединение
 * @note Не очищает память объекта а просто удаляет ссылку на него из std::map<int,CP<connectionType>>
 *       Таким образом если ссылок кроме как в std::map не останется то объект будет реально удалён из памяти
 * @param client_id
 * @param local_buf
 *  
 */
template< class connectionType >
void tcpServer<connectionType>::deleteClient(CP<connectionType> clientObj, thread_data* local_buf)
{ 
    TagLoger::debug(Log_tcpServer, 0, "S%d:call closeClientConnection from deleteClient\n",this->id);
    deleteClientFromMap(clientObj);
    closeClientConnection(clientObj, local_buf);
}

/**
 * Удаляет соединение
 * @note Не очищает память объекта а просто удаляет ссылку на него из std::map<int,CP<connectionType>>
 *       Таким образом если ссылок кроме как в std::map не останется то объект будет реально удалён из памяти
 * @param client_id
 * @param local_buf
 */
template< class connectionType >
void tcpServer<connectionType>::deleteClient(int client_id, thread_data* local_buf)
{
    pthread_mutex_lock(&request_mutex[client_id%map_index_size]); 
    auto it = getMapToUserId(client_id).find(client_id);
    if( it != getMapToUserId(client_id).end() )
    {
        CP<connectionType> clientObj = it->second;
        pthread_mutex_unlock(&request_mutex[client_id%map_index_size]);  
        
        TagLoger::debug(Log_tcpServer, 0, "S%d:call deleteClient client_id=%\n",this->id, client_id);
        deleteClient(clientObj, local_buf);
        return;
    }
    pthread_mutex_unlock(&request_mutex[client_id%map_index_size]);   
}

/**
 * Обрабатывает сообщения от подключённых клиентов
 * @todo simpleTask | отказывать в подключении если прислано слишком много данных | количество указывать в конфиге
 * @param client
 * @param thread_id
 * @return
 */
template< class connectionType >
    int tcpServer<connectionType>::handle_message(CP<connectionType> clientObj, int thread_id, thread_data* local_buf)
    {
        int len = 0;
 
        if(benchmark) bm.increment_handle_message();

        local_buf->unlockAll();
        int client = clientObj->getfd();
          
        bm.set_th_status(thread_id, 'R');
        
        
        TagLoger::log(Log_tcpServer, 0, "S%d:{thread_id:%d}Try to read from fd(%d)\n",this->id, thread_id, client);
        len = clientObj->web_read(local_buf);

        
        TagLoger::log(Log_tcpServer, 0, "S%d:{%d}clients[%d].web_read() = %d\n\n",this->id, thread_id, client, len);

        if(len < 0 )
        {
            bm.set_th_status(thread_id, 'D');
            TagLoger::warn(Log_tcpServer, 0, "\x1b[31mS%d:{%d}Failed to receive data from %d\x1b[0m\n",this->id,thread_id, client);
            deleteClient(clientObj, local_buf);
        }
        else if(len > 0)
        { 
            bm.set_th_status(thread_id, 'M');
            // Обработка сообщения от клиента
            if(clientObj->request(client, len, local_buf) == -1 )
            {
                bm.set_th_status(thread_id, 'D');
                TagLoger::warn(Log_tcpServer, 0, "\x1b[32m{%d} Processing a message from the client return -1 %d\x1b[0m\n",thread_id, client);
                deleteClient(clientObj, local_buf);
            }
        }
        else
        { 
            bm.set_th_status(thread_id, 'D');
            // zero size of len mean the client closed connection
             TagLoger::warn(Log_tcpServer, 0, "\x1b[32m{%d} zero size of len mean the client closed connection from %d\x1b[0m\n",thread_id, client);
             deleteClient(clientObj, local_buf);
        }
 
        bm.set_th_status(thread_id, 'E');
        return len;
    }


/**
 * Возвращает объект соединения по его идентификатору, если идентификатор не валидный то у возвращённого объекта функция getID вернёт -1.
 * @param client_id
 * @return
 */
template< class connectionType >
    CP<connectionType> tcpServer<connectionType>::get(int client_id)
    {
        pthread_mutex_lock(&request_mutex[client_id%map_index_size]);

        auto it = getMapToUserId(client_id).find(client_id);
        if( it != getMapToUserId(client_id).end() )
        {
            pthread_mutex_unlock(&request_mutex[client_id%map_index_size]);
            return it->second;
        }

        pthread_mutex_unlock(&request_mutex[client_id%map_index_size]);
        return NULL;
    }
 
#include "Client_connection.h" 
#include "MySql_connection.h"
#include "Freeswitch_connection.h"
#include "CometQLProxy_connection.h"

template class tcpServer<Client_connection>; 
template class tcpServer<MySql_connection>;
template class tcpServer<Freeswitch_connection>;
template class tcpServer<CometQLProxy_connection>;


#endif	/* TCPSERVER_CPP */ 



