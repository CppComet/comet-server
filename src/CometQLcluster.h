// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * Author: levha
 *
 * Created on 24 Ноября 2015 г., 12:37
 */

#include <string.h>

#ifndef COMETQLCLUSTER_H
#define	COMETQLCLUSTER_H

#include <mysql.h>

#include "appConf.h"

class CometQLcluster;

/**
 * Класс для хранения информации о запросах поступающих от систем репликации
 */
class CometQLClusterQueryData
{
    friend class CometQLcluster;

protected:
    /**
     * Номер ноды
     * @note Нода дожна иметь номер больше нуля
     */
    unsigned char node;

    /**
     * Время запуска
     */
    unsigned long startTime;

    /**
     * Сщётчик запросов
     */
    unsigned long queryCount;

    /**
     * Идентификатор пользователя отправившего запрос
     */
    unsigned int dev_id;

    bool isQueryFromCluster()
    {
        return node != 0;
    }

    /**
     * Читает данные из строки запроса queryString и заполняет ими свои поля.
     * @param queryString
     * @return Количество байт которое занимал заголовок запроса
     */
    int read(char* queryString);
    int wright(char* queryString);

public:
    int getDevId()
    {
        return dev_id;
    }
};


/**
 * Для репликации запросов которые необходимы для поддержания на дополнительных нодах горячего бекапа
 */
#define REPLICATION_BACKUP          1

/**
 * Для репликации запросов для работы в режиме кластера высокой доступности
 * Когда запись идёт на любую из доступных нод, а js подключается ко всем нодам сразу
 */
#define REPLICATION_HIGH_AVALIBLE   2

/**
 * Для работы в режиме кластера высокой нагрузки, кода репликация идёт на все ноды
 * запросы на запись и чтение идут к любой из нод
 * js подключается к одной из нод а не ко всем сразу
 */
#define REPLICATION_HIGH_LOAD       4



class CometQLcluster
{ 
public:
    MYSQL mysql[MAX_HL_CLUSTER_SIZE];
    CometQLClusterQueryData queryData;
    char* query = NULL; 
    int queryLen = 0;

    CometQLcluster()
    {
        query = new char[appConf::instance()->get_int("main", "buf_size")];
        bzero(query, appConf::instance()->get_int("main", "buf_size"));
        start();
    }

    int read(char* queryString);

    bool isQueryFromCluster()
    {
        return queryData.isQueryFromCluster();
    }

    ~CometQLcluster()
    {
        if(query != NULL)
        {
            delete[] query;
        }
    }

    void start();

    bool isValidQueryFromCluster()
    {
        return true;
    }
 
    void replicateQuery();
     
};

#endif
