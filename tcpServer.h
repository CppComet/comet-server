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
#include "thread_data.h"
#include "appConf.h"
#include "intervalLoop.h"

template< class connectionType >
class tcpServer; 
 
#ifndef TCPSERVER_H
#define	TCPSERVER_H

/**
 * Класс учёта статистики по подключениям к серверу и сохранения этой статистики в redis и вывода её в stdout
 */
class tcpServer_benchmark: public intervalLoopObject
{
    int addClient = 0;
    int deleteClient = 0;
    
    /**
     * Количество входящих сообщений
     */
    int handle_message = 0;

    float ps_addClient = 0;
    float ps_deleteClient = 0;
    float ps_handle_message = 0;

    int conections = 0;

    float tps_addClient = 0;
    float tps_deleteClient = 0;
    float tps_handle_message = 0;

    double tps_work_time = 0;
    double ps_work_time = 0;
    double work_time = 0;

    int server_id = 0; 

    int cprint_count = 0;
    char name[300];
    char file_name[310];

    char* th_status = NULL;
    int* th_status_count = NULL;
    float* th_ps_status_count = NULL;
    int* th_tps_status_count = NULL;
    int th_num = 0;
 
    pthread_t th_id = 0;
  
    static time_t start_time;

    FILE* fp = NULL;
    int fileCountLines = 0;
public:
    
    /**
     * Интервал между сбрасыванием статистики в редис
     * Вывод статистики в stdout осуществляется в 3 раза реже
     */
    int stat_interval = 1;

    tcpServer_benchmark()
    {   
        bzero(name, 300);
        bzero(file_name, 310);
    }

    ~tcpServer_benchmark()
    {
        if(th_status != NULL)
        {
            delete[] th_status;
        }
        
        if(th_status_count != NULL)
        {
            delete[] th_status_count;
            delete[] th_ps_status_count;
            delete[] th_tps_status_count;
        }
        
        pthread_exit(&th_id);
    }
  
    /**
     * Печать статистики в stdout
     */
    void cprint() const
    {
        printf("----------------server_id(%d)%s----------------\n",server_id, name);
        printf("Соединений сейчас\t%d\n",conections);
        printf("addClient\t\t%d\t\t%.2f\n",addClient,ps_addClient);
        printf("deleteClient\t\t%d\t\t%.2f\n",deleteClient,ps_deleteClient);
        printf("handle_message\t\t%d\t\t%.2f\n",handle_message,ps_handle_message);
        printf("work_time\t\t%d\n",cprint_count);
        
        printf("[S]\tC\tPsC\n");
        for(int i = 0; i< th_num; i++)
        {
            printf("[%c]\t%d\t%.2f\n",th_status[i],th_status_count[i], th_ps_status_count[i]);
        }
        printf("---------------------------------------------\n");
    }
    
    void filePrint()
    {
        if(fp == NULL)
        {
            int len = strlen(name);
            memcpy(file_name, name, len);
            memcpy(file_name+len, ".csv", 4);
            
            fp = fopen(file_name, "w");
            fprintf(fp, "work_time,conections,addClient,deleteClient,handle_message,ps_addClient,ps_deleteClient,ps_handle_message,loadavg_1,loadavg_2,loadavg_3,running_processes,total_processes" );
            for(int i = 0; i< th_num; i++)
            {
                fprintf(fp, ",th_status_count%d,th_ps_status_count%d", i, i);
            }
            fprintf(fp, "\n");
        }
        
        FILE* loadavgFp = fopen("/proc/loadavg", "r");
        float loadavg_1, loadavg_2, loadavg_3;
        int running_processes, total_processes;
        
        fscanf(loadavgFp, "%5f %5f %5f %5d/%5d", &loadavg_1, &loadavg_2, &loadavg_3, &running_processes, &total_processes);
        fclose(loadavgFp);
        
        fprintf(fp, "%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d",
                cprint_count, conections, addClient, deleteClient, handle_message, ps_addClient, ps_deleteClient, ps_handle_message, loadavg_1, loadavg_2, loadavg_3, running_processes, total_processes);
         
        for(int i = 0; i< th_num; i++)
        {
            fprintf(fp, ",%d,%.2f",th_status_count[i], th_ps_status_count[i]);
        }
        fprintf(fp, "\n");
        fileCountLines++;
        
        if(fileCountLines > appConf::instance()->benchmark_file_save) 
        {
            fileCountLines = 0;
            fclose(fp);
            fp = fopen(file_name, "a");
        }
    }
    
    int getAddClient(){ return addClient; }
    float getPsAddClient(){ return ps_addClient; }
    
    int getDeleteClient(){ return deleteClient; }    
    float getPsDeleteClient(){ return ps_deleteClient; }
    
    int getHandleMessage(){ return handle_message; }
    float getPsHandleMessage(){ return ps_handle_message; }

    int getServiceId(){ return server_id; }
    const char* getServiceName(){ return name; }
    
    int getConections(){ return conections; }
    
    /**
     * Устанавливает статус потоку для статистики
     * @param i номер потока
     * @param c статус
     * @link https://worldpage.ru/redmine/documents/19#Контроль-в-пеприод-работы
     */
    void set_th_status(int i, char c)
    {
        if(i >= 0 && i< th_num )
        {
            th_status[i] = c; 
        }
    }
    
    /**
     * Для учёта входящих сообщений обрабатываемых потоком
     * @param i
     * @param num
     */
    void add_th_count(int i, int num)
    {
        if(i >= 0 && i< th_num )
        { 
            th_status_count[i]+=num;
            th_tps_status_count[i]+=num;
        }
    }
    
    /**
     * Вернёт статус потока
     * @param i
     * @return 
     */
    char get_th_status(int i)
    {
        if(i >= 0 && i< th_num )
        {
            return th_status[i];
        }
        return -1;
    }
    
    /**
     * Вернёт количество сообщений обработаных потоком
     * @param i
     * @return 
     */
    int get_th_count(int i)
    {
        if(i >= 0 && i< th_num )
        {
            return th_status_count[i];
        }
        return -1;
    }
    
    /**
     * Вернёт количество сообщений обработаных потоком за секунду
     * @param i
     * @return 
     */
    int get_th_ps_count(int i)
    {
        if(i >= 0 && i< th_num )
        {
            return th_ps_status_count[i];
        }
        return -1;
    }
    
    inline int get_th_num()
    {
        return th_num;
    }

    /**
     * Запускает поток в котором будет производится вывод статистики и сохранение статистики.
     * @param Servewr_id
     * @param th_Num Количество потоков
     * @param server_name
     */
    void start(int Servewr_id, int th_Num, const char* server_name)
    {
        bzero(name, sizeof(char) * 299);
        memcpy(name, server_name, strlen(server_name));

        TagLoger::log(Log_benchmark, 0, "Запуск benchmark %d [%s]\n",Servewr_id, name);
        if(th_id == 0)
        {
            th_num = th_Num;
            th_status = new char[th_num];
            th_status_count = new int[th_num];
            th_ps_status_count = new float[th_num];
            th_tps_status_count = new int[th_num];

            for(int i = 0; i< th_num; i++)
            {
                th_status[i] = '-';
                th_status_count[i] = 0; 
                th_ps_status_count[i] = 0;
                th_tps_status_count[i] = 0; 
            }
            conections = 0;
            server_id = Servewr_id;
  
            //pthread_create(&th_id, NULL, stat_loop , (void *)this);
            intervalLoop::instance()->add((intervalLoopObject*)this);
        }
    }


    /**
     * Учесть новое входящие соединение
     */
    void increment_addClient()
    {
        addClient++;
        tps_addClient++;
        conections++;
    }

    /**
     * Учесть закрытие входящего соединения
     */
    void increment_deleteClient()
    {
        deleteClient++;
        tps_deleteClient++;
        conections--;
    }

    /**
     * Учесть новое сеебщение
     */
    void increment_handle_message()
    {
        handle_message++;
        tps_handle_message++;
    }

    /**
     * Для корректировки числа людей online, так как у сщётчика почемуто накапливается погрешность.
     * @param Conections
     */
    void setConections(int Conections)
    {
        conections = Conections;
    }
 
    long static get_uptime( )
    {
        return time(0) - start_time;
    }
    
    /**
     * Шаг съёма статистики и обнуления счётчиков
     * @param step
     * @return 
     */
    void step(int step, thread_data* local_buf)
    {
        if( stat_interval == 0)
        {
            return;
        }
        
        if( step%stat_interval != 0)
        {
            return;
        }
        
        if(server_id == 0)
        {
            return;
        }
        
        ps_addClient = tps_addClient/stat_interval;
        ps_deleteClient = tps_deleteClient/stat_interval;
        ps_handle_message = tps_handle_message/stat_interval;
        ps_work_time = tps_work_time;

        tps_addClient=0;
        tps_deleteClient=0;
        tps_handle_message=0;
        tps_work_time=0;

        for(int i = 0; i< th_num; i++)
        {
            th_ps_status_count[i] = (float)th_tps_status_count[i]/stat_interval;
            th_tps_status_count[i] = 0;
        }


        cprint_count+=stat_interval;

        if(appConf::instance()->benchmarkToLog) cprint();  
        if(appConf::instance()->benchmarkToFile) filePrint();   
    } 
};




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
class tcpServer
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

    /**
     *  @todo Заменить std::map на std::unordered_map так как он быстрее http://forum.vingrad.ru/topic-341520.html
     */
    inline std::map<int,CP<connectionType>>& getMapToUserId(unsigned int client_id) const
    { 
        return map_index[client_id%map_index_size];
    }

    tcpServer_loop_data<connectionType> *th = NULL;

    void init()
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
 
/**
 * Потом поправить ... в общем убираешь все лишние define а в конце tcpServer.cpp добавляешь строки
 * template class tcpServer<Client_connection>;
 * template class tcpServer<Server_connection>;
 * 
 *  http://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor
 */
//#include "tcpServer.cpp"
 
#endif	/* TCPSERVER_H */
