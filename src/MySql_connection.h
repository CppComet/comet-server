// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   Client_connection.h
 * Author: levha
 *
 * Created on 01 Май 2015 г., 13:20
 */

#include <stdio.h>
#include <cstdlib>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>

#include "CP.h"
#include "connection.h"


#include "tcpServer.h"


#include <vector>
#include <map>

#ifndef MYSQL_CONNECTION_H
#define	MYSQL_CONNECTION_H
 
#include "MySqlProtocol.h"
#include "CometQL.h"
#include "mystring.h" 
 
/**
 * Коды ошибок
 */
#define  NO_ERROR 0
#define  AUTHORIZATION_ERROR -12
#define  ERROR_USER_OFLINE -5
#define  ERROR_USER_OFLINE_AND_OVERFLOW_QUEUE -10
#define  ERROR_USER_OFLINE_MORE_MAX_OFLINE_TIME -11

#define  ERROR_PROTOCOL_INCORRECT_HASH -13
#define  ERROR_PROTOCOL_INCORRECT -14

#define  ERROR_USER_CONECTION -6

#define  ERROR_UNDEFINED_EVENT -7
#define  ERROR_UNDEFINED_SERVER_EVENT -1


#define  ERROR_TOO_LARGE_MESSAGE -15
#define  ERROR_SERVER -500

/**
 * Максимальный размер сообщения
 * @todo вынести в конфигурационный файл  | а лучше и экономичнее выделять память по мере надобности
 */
#define  MAX_MESSAGE_SIZE 9000
#define  MAX_OFLINE_TIME 90  


/**
 * Максимальная длина очереди сообщений на пользователя
 * @todo simpleTask вынести в конфигурационный файл
 */
#define  MAX_QUEUE_LENGTH 100

/**
 * Максимальная длина лога сообщений в канале
 * @todo simpleTask вынести в конфигурационный файл
 */
#define  MAX_LOG_LENGTH 1000
  

/**
 * Обращение с номером больше чем максимальное количество подключёных пользователей (9)
 */
#define  ERROR_MORE_MAX_CONECTIONS -9


using namespace std;

class PipeLog
{
public:
    static int addToLog(thread_data* local_buf, unsigned int dev_id, const char* pipe_name, const char* event_name, unsigned int from_user_id, const char* msg, unsigned int msg_length);
};


class PipeSettings
{
    unsigned int log_length = 0;
public:
    char* pipe_name = 0;
    unsigned int dev_id = 0;

    /**
     *
     * @param Dev_id
     * @param Pipe_name Предполагается что имя канала проверено и соответсвует AZ09test
     */
    PipeSettings(unsigned int Dev_id, char* Pipe_name):dev_id(Dev_id), pipe_name(Pipe_name){}
    PipeSettings(unsigned int Dev_id, char* Pipe_name, unsigned int Log_length):dev_id(Dev_id), pipe_name(Pipe_name), log_length(Log_length){}

    void setLogLength(unsigned int length)
    {
        if(length > MAX_LOG_LENGTH)
        {
            length = MAX_LOG_LENGTH;
        }

        if(length <= 0)
        {
            length = 0;
        } 
        log_length = length;
    }
  
    unsigned int getLogLength()
    {
        return log_length;
    }

    /**
     * Сохраняет настройки канала
     * @param local_buf
     * @return 
     */
    int save(thread_data* local_buf)
    {
        if(!AZ09test(pipe_name,strlen(pipe_name)))
        {
            return -1;
        }
 
        // Сохраняем настройки канала в бд
        local_buf->db.query_format("REPLACE INTO `pipes_settings`(`dev_id`, `name`, `type`, `length`) VALUES (%d, '%s', '0', %d);", dev_id, pipe_name, log_length);
        //local_buf->db.query_format("DELETE FROM `pipe_messages` WHERE `dev_id` = %d and `pipe_name` = '%s' ORDER BY `time` DESC limit %d, 99999", dev_id, pipe_name, log_length);
        return 0;
    }

    bool load(thread_data* local_buf)
    {
        if(!AZ09test(pipe_name,strlen(pipe_name)))
        {
            return false;
        }

        local_buf->stm.pipes_settings_select->execute(dev_id, pipe_name);
        if(local_buf->stm.pipes_settings_select->fetch())
        {
            TagLoger::error(Log_UserItem, 0, "pipes_settings not found Error:pipes_settings_select [fetch-1] pipe_name = %s, dev_id = %d\n", pipe_name, dev_id);
            local_buf->stm.pipes_settings_select->free();
            return false;
        }

        log_length = local_buf->stm.pipes_settings_select->result_length;
        local_buf->stm.pipes_settings_select->free();
 
        return true;
    }

    bool remove(thread_data* local_buf)
    {
        if(!AZ09test(pipe_name,strlen(pipe_name)))
        {
            return false;
        }
   
        local_buf->db.query_format("DELETE FROM `pipe_messages` WHERE `dev_id` = %d, `name` = '%s' ;", dev_id, pipe_name);
        local_buf->db.query_format("DELETE FROM `pipes_settings` WHERE `dev_id` = %d and `name` = '%s' ;", dev_id, pipe_name); 
        
        return true;
    }
};

 
#define STATE_0 0
#define STATE_SEND_HANDSHAKE 1
#define STATE_RECEIVED_HANDSHAKE 2

/**
 * Количество таблиц в масиве MySql_connection::tables
 */
#define MYSQL_TABLES_COUNT 10

/**
 * Класс одного соединения с сервером
 * Содержит функции обработки запросов от бекенда
 */
class MySql_connection:public connection
{
    friend class tcpServer<MySql_connection>;

protected:
    
    /**
     * Идентификатор dev_id
     */
    int dev_id = 0;
    
    /**
     * Не 0 если сообщение пришло из кластера
     */
    int cometqlcluster = 0;
    
    /**
     * Указывает на то что у пользователя есть права супер пользователя
     */
    bool isAuthUser = false;

    int api_version = 0;

    long clientState = STATE_0;
    unsigned int PacketMaxSize = 0;
    unsigned int ClientFlags = 0;
    unsigned char ClientCharacterSet = 0;

    /**
     * Для хранения случайных 20 байт используемых при авторизации по MySQL Authentication::Native41
     * http://dev.mysql.com/doc/internals/en/secure-password-authentication.html#packet-Authentication::Native41
     */
    char random20bytes[20];

    /**
     * Для хранения фрагментов сообщений которые пришли не полностью.
     */
    bufferController fragment_buf;
    
    inline bool isFromQLCluster(){ return cometqlcluster != 0; }
    
  public:
      
    /*
     * https://www.opennet.ru/docs/RUS/bison_yacc/bison_6.html#SEC65
     * https://www.opennet.ru/docs/RUS/bison_yacc/bison_7.html#SEC74
     * В норме Bison генерирует не повторно входимый анализатор. 
     * В качестве альтернативы вы можете создать чистый, повторно входимый анализатор.
     * Объявление Bison %pure_parser говорит, что вы хотите получить повторно входимый анализатор. 
     * %pure_parser
     * 
     * В результате переменные взаимодействия yylval и yylloc становятся локальными переменными yyparse
     * и используются другие соглашения о вызове функции лексического анализатора yylex. 
     * См. раздел 5.2.4 Соглашения о вызове для чистых анализаторов, для прояснения деталей. 
     * Переменная yynerrs также становится локальной переменной yyparse (см. раздел 5.3 Функция сообщения об ошибках yyerror).
     * Соглашения о вызове самой функции yyparse не изменяются.
     * Будет ли анализатор чистым, никак не влияет на правила грамматики.
     * Вы можете создать как чистый, так и не повторно входимый анализатор из любой правильной грамматики.
     * 
     * @todo Надо бы и наш парсер переиначить в повторно входимый.
     */
    static pthread_mutex_t QLParsing_mutex;
 
    MySql_connection();
    virtual ~MySql_connection();

    int request(int client, int len, thread_data* local_buf);
    int one_query(int client, int len, thread_data* local_buf, char* data);

    int set_online(thread_data* local_buf);
    int set_offline(thread_data* local_buf);

    /**
     * Добавляет задачу в план задачь для переодической очистки бд от устаревших данных
     */
    static void addIntervalRoutine();

protected:

    virtual int query_router(thread_data* local_buf, int PacketNomber);
    /**
     *  Если версия api не выбрана то вернёт false и отправит информацию об ошибке клиенту.
     */
    bool test_api_version(thread_data* local_buf, unsigned int PacketNomber);

    bool sql_use_db(char* db_name);

    int sql_show_databases(thread_data* local_buf, unsigned int PacketNomber);
    int sql_show_tables(thread_data* local_buf, unsigned int PacketNomber);
    int sql_show_columns(thread_data* local_buf, unsigned int PacketNomber);
    int sql_show_table_status(thread_data* local_buf, unsigned int PacketNomber);

    /**
     * отображает список потоков
     */
    int sql_show_processlist(thread_data* local_buf, unsigned int PacketNomber);

    /**
     * SHOW STATUS https://dev.mysql.com/doc/refman/5.0/en/show-status.html
     */
    int sql_show_status(thread_data* local_buf, unsigned int PacketNomber);

    int sql_show_variables(thread_data* local_buf, unsigned int PacketNomber);
    /**
     * SHOW SLAVE HOSTS https://dev.mysql.com/doc/refman/5.0/en/show-slave-hosts.html
     * SHOW SLAVE STATUS https://dev.mysql.com/doc/refman/5.0/en/show-slave-status.html
     */

    int sql_set_value(thread_data* local_buf, unsigned int PacketNomber);
    
    
    int sql_select_from_profiler(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_database_name(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_systemvarible(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_from_users_auth(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_from_users_time(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_from_users_data(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_from_users_messages(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_from_pipes_messages(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_from_users_in_pipes(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_from_pipes(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_from_pipes_settings(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_from_conference(thread_data* local_buf, unsigned int PacketNomber);
    int sql_select_from_conference_members(thread_data* local_buf, unsigned int PacketNomber);

    int sql_insert_into_conference(thread_data* local_buf, unsigned int PacketNomber);
    int sql_insert_into_dialogs(thread_data* local_buf, unsigned int PacketNomber);
    
    int sql_insert_into_conference_members(thread_data* local_buf, unsigned int PacketNomber);
    
    /**
     * Эквивалент таблицы содержащей ключи авторизации пользователей
     * @param local_buf
     * @param PacketNomber
     * @return
     */
    int sql_insert_into_users_auth(thread_data* local_buf, unsigned int PacketNomber);
    int sql_insert_into_users_data(thread_data* local_buf, unsigned int PacketNomber);

    /**
     * Эквивалент таблицы содержащей ключи авторизации время последнего пребывания пользователей online
     * @param local_buf
     * @param PacketNomber
     * @return
     */
    int sql_insert_into_users_time(thread_data* local_buf, unsigned int PacketNomber);
    int sql_insert_into_users_messages(thread_data* local_buf, unsigned int PacketNomber);
    int sql_insert_into_pipes_messages(thread_data* local_buf, unsigned int PacketNomber);
    int sql_insert_into_users_in_pipes(thread_data* local_buf, unsigned int PacketNomber);
    int sql_insert_into_pipes(thread_data* local_buf, unsigned int PacketNomber);
    int sql_insert_into_pipes_settings(thread_data* local_buf, unsigned int PacketNomber);
  
    int sql_delete_from_users_auth(thread_data* local_buf, unsigned int PacketNomber);
    int sql_delete_from_users_data(thread_data* local_buf, unsigned int PacketNomber);
    int sql_delete_from_users_time(thread_data* local_buf, unsigned int PacketNomber);
    int sql_delete_from_users_messages(thread_data* local_buf, unsigned int PacketNomber);
    int sql_delete_from_pipes_messages(thread_data* local_buf, unsigned int PacketNomber);
    int sql_delete_from_users_in_pipes(thread_data* local_buf, unsigned int PacketNomber);
    int sql_delete_from_pipes(thread_data* local_buf, unsigned int PacketNomber);
    int sql_delete_from_pipes_settings(thread_data* local_buf, unsigned int PacketNomber);
    int sql_delete_from_conference(thread_data* local_buf, unsigned int PacketNomber);
    int sql_delete_from_conference_members(thread_data* local_buf, unsigned int PacketNomber);
    
    int sql_select_from_revoked_tokens(thread_data* local_buf, unsigned int PacketNomber);
    int sql_insert_into_revoked_tokens(thread_data* local_buf, unsigned int PacketNomber);
    int sql_delete_from_revoked_tokens(thread_data* local_buf, unsigned int PacketNomber);
};

#endif	/* MYSQL_CONNECTION_H */

