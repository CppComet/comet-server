

#ifndef TCPSERVER_BENCHMARK_H
#define	TCPSERVER_BENCHMARK_H

class tcpServer_benchmark;
#include <cstdlib>
#include "appConf.h"
#include "intervalLoop.h"

/**
 * Класс учёта статистики по подключениям к серверу и сохранения этой статистики в redis и вывода её в stdout
 */
class tcpServer_benchmark: public intervalLoopObject
{
    int addClient = 0;
    int deleteClient = 0;
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
        printf("Online now\t\t%d\n",conections);
        printf("Add clients\t\t%d\t\t%.2f\n",addClient,ps_addClient);
        printf("Delete clients\t\t%d\t\t%.2f\n",deleteClient,ps_deleteClient);
        printf("Handle messages\t\t%d\t\t%.2f\n",handle_message,ps_handle_message);
        printf("work time\t\t%d\n",cprint_count);
        
        printf("[S]\tC\tPsC\n");
        for(int i = 0; i< th_num; i++)
        {
            if(th_status[i] == '0')
            {
                printf("\x1b[32m[%c]\x1b[0m\t%d\t%.2f\n",th_status[i],th_status_count[i], th_ps_status_count[i]);
            }
            else if(th_status[i] == '1')
            {
                printf("\x1b[33m[%c]\x1b[0m\t%d\t%.2f\n",th_status[i],th_status_count[i], th_ps_status_count[i]);
            }
            else
            { 
                printf("\x1b[31m[%c]\x1b[0m\t%d\t%.2f\n",th_status[i],th_status_count[i], th_ps_status_count[i]);
            }
        }
        printf("---------------------------------------------\n");
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
    void start(int Servewr_id, int th_Num, const char* server_name); 
    
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
     * Для корректировки числа людей online, так как у сщётчика накапливается погрешность (кажется из за того что операции инкремента и декремента не атомарны).
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

        if(appConf::instance()->get_bool("statistics", "to_log")) cprint();   
    } 
};
 
class usage_statistics: public intervalLoopObject
{
public:
    
    /**
     * Отправка данных статистики через curl
     * @param data данные
     * @return 
     */
    static bool send(std::string data);
  
    void static start();
};
 
#endif	/* TCPSERVER_BENCHMARK_H */