// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#ifndef THREAD_DATA_H
#define	THREAD_DATA_H

class thread_data;
class tcpServer_benchmark;
#include "intervalLoop.h"

#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <exception>

#include "main.h"
#include "appConf.h"

#include "dbLink.h"

#include "CometQL.h"
  
#include "freeswitch-esl/include/esl.h"

#include "tcpServer_benchmark.h"

#ifndef ARRAY_BUFFER_SIZE
    #define ARRAY_BUFFER_SIZE 32
#endif

/**
 * Класс для работы с памятью предназначеной для буферизации принимаемых и отправляемых данных.
 *
 * @todo Заменить поддержку работы так чтоб можно было не задавать начальный размер [opt1 -600*ws_online]
 */
class bufferController
{
    char* buf = NULL;
    int buf_size = 0;
    int data_size = 0;
    bool inUse_buf = false;

private:

    /**
     * Запрещаем копирование для объектов донного класса
     */
    bufferController(const bufferController& cpy) = delete;

    /**
     * Запрещаем копирование для объектов донного класса
     */
    void operator=( const bufferController& cpy) = delete;

public:

    /*bufferController()
    {
    }*/

    bufferController(int size)
    {
        setSize(size);
    }

    /**
     * Устанавливает минимальный объём памяти и очищает содержимое буфера
     * @param size
     */
    void setSize(int size)
    {
        data_size = size;
        if(buf == NULL)
        {
            // Выделим память если её нет
            buf_size = size;
            buf = new char[buf_size];
            bzero(buf, buf_size);
        }

        if(buf_size < size)
        {
            // Удалим данные и выдилим побольше памяти
            delete[] buf;
            buf_size = size;
            buf = new char[buf_size];
            bzero(buf, buf_size);
        }
        else
        {
            // Если памяти и так достаточно то просто очистим её.
            bzero(buf, buf_size);
        }
    }

    /**
     * @return Вернёт номинальный объём памяти, то есть тот что был последний раз передан в setSize. А не тот который реально выделен.
     */
    int getSize()
    {
        return data_size;
    }

    int getMemSize()
    {
        return buf_size;
    }

    ~bufferController()
    {
        if(buf != NULL)
        {
            delete[] buf;
        }
    }

    char* getData()
    {
        return buf;
    }
    /**
     * Вернёт buf без проверки блокирования
     */
    operator char* () const
    {
        return buf;
    }

    operator const unsigned char*() const
    {
        return (unsigned char*)buf;
    }

    /**
     * Вернёт true если буфер заблокирован
     */
    operator bool () const
    {
        return inUse_buf;
    }

    /**
     * Вернёт true если буфер заблокирован
     */
    int operator == (bool)
    {
        return inUse_buf;
    }

    char& operator[] (int key)
    {
        return buf[key];
    }

    bool isLock()
    {
        return inUse_buf;
    }

    /**
     * Функция для контроля доступа.
     * Делает заметку о том что память занята.
     */
    char* lock()
    {
        if(inUse_buf)
        {
            TagLoger::trace(Log_Any, LogColorRed, "get inUse_buf != false");
        }

        if(buf == NULL)
        {
            TagLoger::warn(Log_Any, LogColorRed, "get buf == NULL");
        }

        inUse_buf = true;
        bzero(buf, buf_size);
        return buf;
    }

    /**
     * Функция для контроля доступа.
     * Делает заметку о том что память свободна.
     */
    void unlock()
    {
        inUse_buf = false;
    }

    /**
     * Функция для контроля доступа.
     * Делает заметку о том что память свободна.
     */
    char* getAndUnlock()
    {
        inUse_buf = false;
        return buf;
    }
};


class fs_esl{
    esl_handle_t handle = {{0}};
    
    std::string name;
    std::string host;
    std::string id;
    int port = 0;
    std::string password; 
    bool inited = false;
    bool conected = false;
    int timeout = 1000;
    
    std::string WShost;
    int WSport = 7443;
    
    std::string StreamServer;
public:
    
    fs_esl(std::string Name){
        name = Name;
    }
    
    std::string getWSHost(){
        return WShost;
    }
    
    std::string getStreamServer(){
        return StreamServer;
    }
    
    int getWSPort(){
        return WSport;
    }
    
    std::string getName(){
        return name;
    }
    
    std::string getId(){
        return id;
    }
    
    bool init(std::string connection);
    
    esl_handle_t getHandle(); 
    void connect();
    void disconnect();
    void exec(const char* command);
    
    virtual ~fs_esl() {
        disconnect();
    }

};

/**
 * Класс хранящий соединение с редисом и память для записи туда данных на короткий период.
 * Не является потокобезопасным и поэтому подразумевается что в каждом потке иметтся своя копия класса.
 * Данные сохранямые сюда будут удалятся каждый раз перед началом обработки входящего сообщения.
 * В один момет времени с одним экземпляром класса должен работать только один поток.
 */
class thread_data
{
public:

    /**
     * Соединение с mysql сервером для хранения данных
     */
    dbLink db;

    std::vector<dbLink*> wsCluster; 
    std::vector<dbLink*> proxyCluster;
    
    std::vector<fs_esl*> fs_eslCluster; 
    
    stmMapper stm;

    tcpServer_benchmark* bm;
    int thread_id = 0;

    /**
     * Буфер для сообщения от пользователя.
     * Сюда записываются данные прочитаные из сокета.
     */
    bufferController buf;

    /**
     * Буфер для ответного сообщения для функции Client_connection::message, перед использованием его надо очищать.
     */
    bufferController messge_buf;

    /**
     * Буфер для ответного сообщения для пользователя
     * Сюда записываются данные перед отправкой, перед использованием его надо очищать.
     */
    bufferController answer_buf;

    /**
     * Для формирования заголовков описания таблицы ответа в mysql
     */
    mysqlAnswer sql;

    /**
     * Для хранания распарсеной информации о запросе
     */
    QueryData qInfo;

    /*char* tmp_bufdata[ARRAY_BUFFER_SIZE];
    int tmp_bufdataSize[ARRAY_BUFFER_SIZE];
    int tmp_bufdataPrt[ARRAY_BUFFER_SIZE];*/

    thread_data( appConf* app);

    void setThreadStatus(char c);

private:

    /**
     * Запрещаем копирование для объектов данного класса
     */
    thread_data(const thread_data& cpy) = delete;

    /**
     * Запрещаем копирование для объектов данного класса
     */
    void operator=( const thread_data& cpy) = delete;

public:

    bool isWSClusterActive()
    {
        return !wsCluster.empty();
    }
    
    bool wsClusterSize()
    {
        return wsCluster.size();
    }
    
    bool isProxyClusterActive()
    {
        return !proxyCluster.empty();
    }
    
    bool proxyClusterSize()
    {
        return proxyCluster.size();
    }
    
    void unlockAll()
    {
        buf.unlock();
        messge_buf.unlock();
        answer_buf.unlock();
    }

private:
    /**
     * Очищает всю ранее выделеную память.
     */
    /*void erase()
    {
        for(int i = 0; i< ARRAY_BUFFER_SIZE; i++)
        {
            if(tmp_bufdata[i] > 0 && tmp_bufdataSize[i] > 0)
            {
                TagLoger::log(Log_Any, 0, "erase bzero(tmp_bufdata[%d], %d)\n", i, tmp_bufdataSize[i]);
                bzero(tmp_bufdata[i], tmp_bufdataSize[i]);
            }
            else
            {
                TagLoger::log(Log_Any, 0, "break erase tmp_bufdata[%d]\n", i);
                break;
            }
        }

        bzero(tmp_bufdataPrt, ARRAY_BUFFER_SIZE);
    }*/

    /**
     * Распределитель памяти.
     * Выделяет блок памяти запрошенного размера.
     * Очистка памяти происходит сразу для всех выделеных блоков вызовом метода erase()
     * @param size
     * @return
     */
    /*char* getMem(int size)
    {
        for(int i =0; i< ARRAY_BUFFER_SIZE - 1; i++)
        {
            if(tmp_bufdataPrt[i] + size < tmp_bufdataSize[i])
            {
                char* prt = tmp_bufdata[i] + tmp_bufdataPrt[i];
                tmp_bufdataPrt[i] += size + 1;
                return prt;
            }
            else
            {
                if(tmp_bufdata[i+1] == NULL)
                {
                    tmp_bufdataSize[i+1] = tmp_bufdataSize[i]*2;
                    tmp_bufdata[i+1] = new char[tmp_bufdataSize[i+1]];
                }
            }
        }

        throw "getMem not enough memory!";
    }*/

public:

    ~thread_data()
    {
        /*for(int i =0; i< ARRAY_BUFFER_SIZE; i++)
        {
            if(tmp_bufdata[i] != NULL)
            {
                delete[] tmp_bufdata[i];
            }
        }*/
    }
};


#endif	/* THREAD_DATA_H */