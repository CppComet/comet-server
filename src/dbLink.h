// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <mysql.h>

#include "appConf.h"

#ifndef DBLINK_H
#define	DBLINK_H

/**
 * https://dev.mysql.com/doc/refman/5.7/en/error-messages-client.html#error_cr_commands_out_of_sync
 */
#define	CR_COMMANDS_OUT_OF_SYNC 2014

/** 
 * https://dev.mysql.com/doc/refman/5.5/en/error-messages-client.html#error_cr_server_lost
 */
#define	CR_SERVER_LOST 2013

/** 
 * https://dev.mysql.com/doc/refman/5.5/en/error-messages-server.html#error_er_unknown_stmt_handler
 */
#define	ER_UNKNOWN_STMT_HANDLER 1243

/** 
 * https://dev.mysql.com/doc/refman/5.5/en/error-messages-server.html#error_er_server_shutdown
 */
#define	ER_SERVER_SHUTDOWN 1053



class stmMapper;
class dbLink;

class stmBase{

protected:
    bool isInited = false;
    dbLink *db = NULL;
    
    MYSQL_STMT    *stmt = NULL;
    MYSQL_BIND    *param = NULL;
    MYSQL_BIND    *result = NULL;
    MYSQL_RES     *prepare_meta_result = NULL;

    int param_count = 0;
    int column_count = 0;

    void setParamsCount(int size);

    void setResultsCount(int size);

    bool init(dbLink *mysql, const char* q);

    /**
     * Выполняет вставку, обновление, замену, удаление
     */
    int insert();

    /**
     * Выполняет выборку
     */
    bool select();

    /**
     * Извлекает результаты селекта построчно.
     * @return
     */
    int fetch();

    /**
     * Очищает память выделеную для хранения результатов селекта
     * @return
     */
    bool free();
    
    bool virtual prepare(dbLink *mysql){}
    
public:
    MYSQL_STMT *getSTMT()
    {
        return stmt;
    }
};

class dbLink {
    MYSQL* mysqlLink = NULL;
    bool isInit = false;
    bool isConnected = false;

    std::string connectionName;
    std::string db_host;
    std::string db_pw;
    std::string db_user;
    std::string db_name;
    int db_port = 3306;
    my_bool is_reconnect = 1;
    int connect_timeout  = 1; // MYSQL_OPT_CONNECT_TIMEOUT
    int read_timeout  = 1; // MYSQL_OPT_READ_TIMEOUT
    
    stmMapper *stm = NULL; // Привязаные stm запросы
private:

    /**
     * Запрещаем копирование для объектов данного класса
     */
    dbLink(const dbLink& cpy) = delete;

    /**
     * Запрещаем копирование для объектов данного класса
     */
    void operator=( const dbLink& cpy) = delete;

public:

    bool reconnect_on_error = true;
    dbLink(){
        connectionName = "noname";
        connectionName.append(std::to_string(random()));
    }
    
    dbLink(std::string ConnectionName){
        connectionName = ConnectionName;
    }
 
    ~dbLink(){
        close();
    }
    
    void setStmMapper(stmMapper *STM){
        stm = STM;
    }
    
    const char* name(){
        return connectionName.data();
    }

    bool init(std::string connectionString);    
    bool init(const char* host, const char* user, const char* pw, const char* name, int port);

    MYSQL* getLink()
    {
        return mysqlLink;
    }

    operator MYSQL* ()
    {
        return mysqlLink;
    }

    bool query(const char *q);
    
    bool query_format(const char *format, ...);

    bool reconnect();
    
    void close();
    
    bool connect();
};

class stm_log_query: public stmBase{

    friend stmMapper;
protected:

    char*         param_message = NULL;
    unsigned long param_message_length = 0;

    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(1);
            
            if(param_message == NULL)
            {
                param_message_length = appConf::instance()->get_int("db", "buf_size");
                param_message = new char[param_message_length];
            }
            
            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (char *)param_message;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_message_length;
        }
        return init(mysql, "INSERT INTO `log_query`(`id`, `query`) VALUES (NULL, ?)");
    }

public:
    stm_log_query(){}

    int insert(const char* message, unsigned long message_length)
    {
        if(appConf::instance()->get_int("db", "buf_size") < message_length)
        {
            message_length = appConf::instance()->get_int("db", "buf_size") - 1;
        }
        
        memcpy(param_message, message, message_length);
        param_message_length = message_length;
        return stmBase::insert();
    }

    int insert(const char* query)
    {
        return insert(query, strlen(query));
    }
};

#define MYSQL_UUID_LEN 36

class stm_users_queue_insert: public stmBase{

    friend stmMapper;
    char          param_id[MYSQL_UUID_LEN];
    unsigned long param_id_length = MYSQL_UUID_LEN;

    unsigned long param_time = 0;
    unsigned long param_user_id = 0;

    char          param_event[EVENT_NAME_LEN];
    unsigned long param_event_length = EVENT_NAME_LEN;

    char*          param_message = NULL;
    unsigned long  param_message_length = 0;

public:
    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(5);

            if(param_message == NULL) 
            {
                param_message = new char[appConf::instance()->get_int("db", "buf_size")];
                param_message_length = appConf::instance()->get_int("db", "buf_size");
            }

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *) param_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_id_length;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_time;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *) param_event;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_event_length;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *) param_message;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_message_length;
        }
        return init(mysql, "INSERT INTO `users_messages`(`id`, `time`, `user_id`, `event`, `message`) VALUES (?,?,?,?,?)");
    }

    stm_users_queue_insert()
    {
        bzero(param_id, MYSQL_UUID_LEN);
        bzero(param_event, EVENT_NAME_LEN);
    }

    int execute(const char* id, unsigned long time, unsigned long user_id, const char* event, const char* message, unsigned long message_length)
    {
        param_time = time; 
        param_user_id = user_id;

        bzero(param_id, MYSQL_UUID_LEN);
        bzero(param_event, EVENT_NAME_LEN); 
        
        param_id_length = strlen(id);
        if(param_id_length > MYSQL_UUID_LEN)
        {
            param_id_length = MYSQL_UUID_LEN;
        }
        strncpy(param_id, id, param_id_length);

        param_event_length = strlen(event);
        if(param_event_length > EVENT_NAME_LEN)
        {
            param_event_length = EVENT_NAME_LEN;
        }
        strncpy(param_event, event, param_event_length);
        
        if(appConf::instance()->get_int("db", "buf_size") < message_length)
        {
            message_length = appConf::instance()->get_int("db", "buf_size") - 1;
        }
        
        memcpy(param_message, message, message_length);
        param_message_length = message_length;
        
        
        return stmBase::insert();
    }
};

class stm_conference_delete: public stmBase{

    friend stmMapper;
    unsigned long param_user_id = 0;
    
    char          param_name[EVENT_NAME_LEN];
    unsigned long param_name_length = EVENT_NAME_LEN;

public:
    bool virtual prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(2);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *)&param_name;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_name_length; 
            
            i++;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;
        }
        return init(mysql, "delete from `conference` WHERE name = ? and user_id = ? ");
    }

    stm_conference_delete(){}

    int execute(const char* name, unsigned long user_id)
    {
        param_user_id = user_id;
        
        param_name_length = strlen(name);
        if(param_name_length > EVENT_NAME_LEN)
        {
            param_name_length = EVENT_NAME_LEN;
        }
        strncpy(param_name, name, param_name_length);
         
        return stmBase::insert();
    }
};

class stm_conference_insert: public stmBase{

    friend stmMapper; 
    
    unsigned long param_time = 0;
    unsigned long param_active = 0;
    unsigned long param_user_id = 0;
    unsigned long param_caller_id = 0;

    char          param_name[EVENT_NAME_LEN];
    unsigned long param_name_length = EVENT_NAME_LEN;

    char          param_profile[EVENT_NAME_LEN];
    unsigned long param_profile_length = EVENT_NAME_LEN;

    char          param_stream[EVENT_NAME_LEN];
    unsigned long param_stream_length = EVENT_NAME_LEN;

    char          param_node[EVENT_NAME_LEN];
    unsigned long param_node_length = EVENT_NAME_LEN;

    char*          param_message = NULL;
    unsigned long  param_message_length = 0;

public:
    bool virtual prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(8);

            if(param_message == NULL) 
            {
                param_message = new char[appConf::instance()->get_int("db", "buf_size")];
                param_message_length = appConf::instance()->get_int("db", "buf_size");
            }


            int i = 0; 
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *) param_name;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_name_length;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_caller_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *) param_message;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_message_length;
            
            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *) param_profile;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_profile_length;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *) param_stream;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_stream_length;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *) param_node;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_node_length;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_time;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

        }
        
        return init(mysql, "REPLACE INTO `conference`(`name`, `user_id`, `caller_id`, `message`, `profile`, `stream`, `node`, `time`)"
                " VALUES (?,?,?,?,?,?,?,?)");
    }

    stm_conference_insert()
    { 
        bzero(param_node, EVENT_NAME_LEN);
        bzero(param_stream, EVENT_NAME_LEN);
        bzero(param_profile, EVENT_NAME_LEN);
        bzero(param_name, EVENT_NAME_LEN); 
    }

    int execute(
            const char* name,
            unsigned long user_id,
            unsigned long caller_id,
            const char* message,
            unsigned long message_length,
            const char* profile,
            const char* stream,
            const char* node)
    { 
        param_time = time(NULL); 
        
        param_user_id = user_id;
        param_caller_id = caller_id;

        param_message_length = message_length;
 
        bzero(param_name, EVENT_NAME_LEN); 

        param_name_length = strlen(name);
        if(param_name_length > EVENT_NAME_LEN)
        {
            param_name_length = EVENT_NAME_LEN;
        }
        strncpy(param_name, name, param_name_length);
        
        param_profile_length = strlen(profile);
        if(param_profile_length > EVENT_NAME_LEN)
        {
            param_profile_length = EVENT_NAME_LEN;
        }
        strncpy(param_profile, profile, param_profile_length);
        
        param_stream_length = strlen(stream);
        if(param_stream_length > EVENT_NAME_LEN)
        {
            param_stream_length = EVENT_NAME_LEN;
        }
        strncpy(param_stream, stream, param_stream_length);
        
        param_node_length = strlen(node);
        if(param_node_length > EVENT_NAME_LEN)
        {
            param_node_length = EVENT_NAME_LEN;
        }
        strncpy(param_node, node, param_node_length);
         
        if(appConf::instance()->get_int("db", "buf_size") < message_length)
        {
            message_length = appConf::instance()->get_int("db", "buf_size") - 1;
        }
        
        memcpy(param_message, message, message_length);
        param_message_length = message_length;
        
        return stmBase::insert();
    }
};

class stm_conference_select: public stmBase{

    friend stmMapper; 
    
    char          param_name[EVENT_NAME_LEN];
    unsigned long param_name_length = EVENT_NAME_LEN;

public:
   
    unsigned long result_user_id = 0;
    unsigned long result_caller_id = 0;

    char          result_node[EVENT_NAME_LEN];
    unsigned long result_node_length = EVENT_NAME_LEN;

    char          result_profile[EVENT_NAME_LEN];
    unsigned long result_profile_length = EVENT_NAME_LEN;

    char          result_stream[EVENT_NAME_LEN];
    unsigned long result_stream_length = EVENT_NAME_LEN;

    char*          result_message = NULL;
    unsigned long  result_message_length = 0;

    my_bool       is_null[6];
    my_bool       error[6];
    unsigned long length[6];

    stm_conference_select()
    { 
        bzero(is_null, 6);
        bzero(error, 6);
        bzero(length, 6);

        bzero(result_node, EVENT_NAME_LEN);
        bzero(result_stream, EVENT_NAME_LEN);
        bzero(result_profile, EVENT_NAME_LEN);
        bzero(param_name, EVENT_NAME_LEN); 
    }

    bool virtual prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            if(result_message == NULL) 
            {
                result_message = new char[appConf::instance()->get_int("db", "buf_size")];
                result_message_length = appConf::instance()->get_int("db", "buf_size"); 
            }

            setParamsCount(1);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *)&param_name;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_name_length;


            setResultsCount(6);

            i = 0; 
            result[i].buffer_type    = MYSQL_TYPE_LONG;
            result[i].buffer         = (void *)&result_user_id;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];

            i++;
            result[i].buffer_type    = MYSQL_TYPE_LONG;
            result[i].buffer         = (void *)&result_caller_id;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            
            i++;
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_message;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = result_message_length;
            
            i++;
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_profile;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = result_profile_length;
            
            i++;
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_stream;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = result_stream_length;

            i++;
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_node;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = result_node_length;

        }
        return init(mysql, "SELECT `user_id`, `caller_id`, `message`, `profile`, `stream`, `node` FROM `conference` WHERE name = ?");
    }

    bool execute(const char* name)
    {
        param_name_length = strlen(name);
        if(param_name_length > EVENT_NAME_LEN)
        {
            param_name_length = EVENT_NAME_LEN;
        }
        strncpy(param_name, name, param_name_length);
        
        return stmBase::select();
    }

    /**
     * Извлекает результаты селекта построчно.
     * @return
     */
    int fetch()
    {
        // Fetch
        // https://docs.oracle.com/cd/E17952_01/mysql-5.5-en/mysql-stmt-fetch.html
        int res = mysql_stmt_fetch(stmt);
        if(res == MYSQL_DATA_TRUNCATED)
        {
            TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), MYSQL_DATA_TRUNCATED\x1b[0m\n");
        }
        else if(res == 1)
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), 1 failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
        } 
        return res;
    }

    /**
     * Очищает память выделеную для хранения результатов селекта
     * @return
     */
    bool free()
    {
        return stmBase::free();
    }
};

class stm_conference_select_nodes_for_room: public stmBase{

    friend stmMapper; 
    char          param_name[EVENT_NAME_LEN];
    unsigned long param_name_length = EVENT_NAME_LEN;

public:
    
    char          result_node[EVENT_NAME_LEN];
    unsigned long result_node_length = EVENT_NAME_LEN;
 
    my_bool       is_null[1];
    my_bool       error[1];
    unsigned long length[1];

    stm_conference_select_nodes_for_room()
    { 
        bzero(is_null, 1);
        bzero(error, 1);
        bzero(length, 1); 
        bzero(result_node, EVENT_NAME_LEN); 
    }

    bool virtual prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true; 
            
            setParamsCount(1);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *)&param_name;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_name_length;


            setResultsCount(1);

            i = 0;  
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_node;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = result_node_length;

        }
        return init(mysql, "SELECT `node` FROM `conference` WHERE name = ? GROUP BY `node`");
    }

    bool execute(const char* name)
    {
        param_name_length = strlen(name);
        if(param_name_length > EVENT_NAME_LEN)
        {
            param_name_length = EVENT_NAME_LEN;
        }
        strncpy(param_name, name, param_name_length);
        
        return stmBase::select();
    }

    /**
     * Извлекает результаты селекта построчно.
     * @return
     */
    int fetch()
    {
        // Fetch
        // https://docs.oracle.com/cd/E17952_01/mysql-5.5-en/mysql-stmt-fetch.html
        int res = mysql_stmt_fetch(stmt);
        if(res == MYSQL_DATA_TRUNCATED)
        {
            TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), MYSQL_DATA_TRUNCATED\x1b[0m\n");
        }
        else if(res == 1)
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), 1 failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
        } 
        return res;
    }

    /**
     * Очищает память выделеную для хранения результатов селекта
     * @return
     */
    bool free()
    {
        return stmBase::free();
    }
};

class stm_users_queue_select: public stmBase{

    friend stmMapper;
    unsigned long param_limit = 10;
    unsigned long param_user_id = 0;

public:
    char          result_id[PIPE_NAME_LEN];
    unsigned long result_id_length = PIPE_NAME_LEN;


    unsigned long result_time = 0;

    char          result_event[EVENT_NAME_LEN];
    unsigned long result_event_length = EVENT_NAME_LEN;

    char*          result_message = NULL;
    unsigned long  result_message_length = 0;

    my_bool       is_null[4];
    my_bool       error[4];
    unsigned long length[4];

    stm_users_queue_select()
    {
        bzero(result_id, PIPE_NAME_LEN);
        bzero(result_event, EVENT_NAME_LEN);

        bzero(is_null, 4);
        bzero(error, 4);
        bzero(length, 4);

    }

    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            if(result_message == NULL) 
            {
                result_message = new char[appConf::instance()->get_int("db", "buf_size")];
                result_message_length = appConf::instance()->get_int("db", "buf_size");
            }
            setParamsCount(2);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_limit;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;


            setResultsCount(4);

            i = 0;
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_id;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = EVENT_NAME_LEN;

            i++;
            result[i].buffer_type    = MYSQL_TYPE_LONG;
            result[i].buffer         = (void *)&result_time;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];

            i++;
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_event;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = EVENT_NAME_LEN;

            i++;
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_message;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = result_message_length;
        }
        return init(mysql, "SELECT `id`, `time`, `event`, `message` FROM `users_messages` WHERE user_id = ? order by time limit ?");
    }

    bool execute(unsigned long user_id, unsigned long limit)
    {
        // https://docs.oracle.com/cd/E17952_01/mysql-5.5-en/mysql-stmt-fetch.html
        param_user_id = user_id;
        param_limit = limit;

        return stmBase::select();
    }

    /**
     * Извлекает результаты селекта построчно.
     * @return
     */
    int fetch()
    {
        // Fetch
        int res = mysql_stmt_fetch(stmt);
        if(res == MYSQL_DATA_TRUNCATED)
        {
            TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), MYSQL_DATA_TRUNCATED\x1b[0m\n");
        }
        else if(res == 1)
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), 1 failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
        } 
        return res;
    }

    /**
     * Очищает память выделеную для хранения результатов селекта
     * @return
     */
    bool free()
    {
        return stmBase::free();
    }
};

class stm_users_queue_delete: public stmBase{

    friend stmMapper;
    unsigned long param_time = 0;
    unsigned long param_user_id = 0;

public:
    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(2);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_time;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;
        }
        return init(mysql, "DELETE FROM `users_messages` where time < ? and user_id = ? ");
    }

    stm_users_queue_delete(){}

    int execute(unsigned long time, unsigned long user_id)
    {
        param_time = time;
        param_user_id = user_id;

        return stmBase::insert();
    }
};


class stm_pipe_messages_insert: public stmBase{

    friend stmMapper;
    char          param_id[MYSQL_UUID_LEN];
    unsigned long param_id_length = MYSQL_UUID_LEN;

    unsigned long param_time = 0;
    unsigned long param_user_id = 0; // Отправитель

    char          param_pipe_name[PIPE_NAME_LEN];
    unsigned long param_pipe_name_length = PIPE_NAME_LEN;

    char          param_event[EVENT_NAME_LEN];
    unsigned long param_event_length = EVENT_NAME_LEN;

    char*          param_message = NULL;
    unsigned long  param_message_length = 0;

public:
    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(6);

            if(param_message == NULL)
            {
                param_message = new char[appConf::instance()->get_int("db", "buf_size")];
                param_message_length = appConf::instance()->get_int("db", "buf_size");
            }
            
            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *) param_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_id_length;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_time;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *)&param_pipe_name;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_pipe_name_length;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *) param_event;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_event_length;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *) param_message;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_message_length;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;
        }
        return init(mysql, "INSERT INTO `pipe_messages`(`id`, `time`, `pipe_name`, `event`, `message`, `user_id`) VALUES (?,?,?,?,?,?)");
    }

    stm_pipe_messages_insert()
    {
        bzero(param_id, MYSQL_UUID_LEN);
        bzero(param_event, EVENT_NAME_LEN);
        bzero(param_pipe_name, PIPE_NAME_LEN);
    }

    int execute(const char* id, unsigned long time, const char* pipe_name, const char* event, const char* message, unsigned long message_length, unsigned long user_id)
    {
        param_time = time;
        param_user_id = user_id;

        param_message_length = message_length;

        bzero(param_id, MYSQL_UUID_LEN);
        bzero(param_event, EVENT_NAME_LEN);
        bzero(param_pipe_name, PIPE_NAME_LEN);
        
        param_id_length = strlen(id);
        if(param_id_length > MYSQL_UUID_LEN)
        {
            param_id_length = MYSQL_UUID_LEN;
        }
        strncpy(param_id, id, param_id_length);

        param_event_length = strlen(event);
        if(param_event_length > EVENT_NAME_LEN)
        {
            param_event_length = EVENT_NAME_LEN;
        }
        strncpy(param_event, event, param_event_length);



        param_pipe_name_length = strlen(pipe_name);
        if(param_pipe_name_length > PIPE_NAME_LEN)
        {
            param_pipe_name_length = PIPE_NAME_LEN;
        }
        strncpy(param_pipe_name, pipe_name, PIPE_NAME_LEN);
        
        if(appConf::instance()->get_int("db", "buf_size") < message_length)
        {
            message_length = appConf::instance()->get_int("db", "buf_size") - 1;
        }
        
        memcpy(param_message, message, message_length);
        param_message_length = message_length;
        return stmBase::insert();
    }
};

class stm_pipe_messages_select: public stmBase{

    friend stmMapper;
    unsigned long param_limit = 10;

    char          param_pipe_name[PIPE_NAME_LEN];
    unsigned long param_pipe_name_length = PIPE_NAME_LEN;

public:
    char          result_id[PIPE_NAME_LEN];
    unsigned long result_id_length = PIPE_NAME_LEN;

    unsigned long result_user_id = 0;
    unsigned long result_time = 0;

    char          result_event[EVENT_NAME_LEN];
    unsigned long result_event_length = EVENT_NAME_LEN;

    char*          result_message = NULL;
    unsigned long  result_message_length = 0;

    my_bool       is_null[5];
    my_bool       error[5];
    unsigned long length[5];

    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            
            if(result_message == NULL)
            {
                result_message = new char[appConf::instance()->get_int("db", "buf_size")];
                result_message_length = appConf::instance()->get_int("db", "buf_size");
            }
            bzero(result_message, result_message_length);


            setParamsCount(2);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *)&param_pipe_name;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_pipe_name_length;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_limit;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;


            setResultsCount(5);

            i = 0;
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_id;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = EVENT_NAME_LEN;

            i++;
            result[i].buffer_type    = MYSQL_TYPE_LONG;
            result[i].buffer         = (void *)&result_time;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];

            i++;
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_event;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = EVENT_NAME_LEN;

            i++;
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_message;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = result_message_length;

            i++;
            result[i].buffer_type    = MYSQL_TYPE_LONG;
            result[i].buffer         = (void *)&result_user_id;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
        }
        return init(mysql, "SELECT `id`, `time`, `event`, `message`, `user_id` FROM `pipe_messages` WHERE pipe_name = ? order by time limit ?");
    }

public:
    stm_pipe_messages_select()
    {
        bzero(is_null, 5);
        bzero(error, 5);
        bzero(length, 5);
        
        bzero(result_id, PIPE_NAME_LEN);
        bzero(result_event, EVENT_NAME_LEN);
    }

    bool execute(const char* pipe_name, unsigned long limit)
    {
        // https://docs.oracle.com/cd/E17952_01/mysql-5.5-en/mysql-stmt-fetch.html
        param_limit = limit;


        param_pipe_name_length = strlen(pipe_name);
        if(param_pipe_name_length > PIPE_NAME_LEN)
        {
            param_pipe_name_length = PIPE_NAME_LEN;
        }
        bzero(param_pipe_name, PIPE_NAME_LEN);
        memcpy(param_pipe_name, pipe_name, param_pipe_name_length);

        return stmBase::select();
    }

    /**
     * Извлекает результаты селекта построчно.
     * @return
     */
    int fetch()
    {
        // Fetch
        int res = mysql_stmt_fetch(stmt);
        if(res == MYSQL_DATA_TRUNCATED)
        {
            TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), MYSQL_DATA_TRUNCATED\x1b[0m\n");
        }
        else if(res == 1)
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), 1 failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
        } 
        return res;
    }

    /**
     * Очищает память выделеную для хранения результатов селекта
     * @return
     */
    bool free()
    {
        return stmBase::free();
    }
};

class stm_pipe_messages_delete: public stmBase{

    friend stmMapper;
    unsigned long param_time = 0;

    char          param_pipe_name[PIPE_NAME_LEN];
    unsigned long param_pipe_name_length = PIPE_NAME_LEN;

public:
    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(2);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_time;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *)&param_pipe_name;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_pipe_name_length;
        }
        return init(mysql, "DELETE FROM `pipe_messages` where time < ? and pipe_name = ? ");
    }

    stm_pipe_messages_delete()
    { 
        bzero(param_pipe_name, PIPE_NAME_LEN);
    }

    int execute(unsigned long time, const char* pipe_name)
    {
        param_time = time;
        param_pipe_name_length = strlen(pipe_name);
        if(param_pipe_name_length > PIPE_NAME_LEN)
        {
            param_pipe_name_length = PIPE_NAME_LEN;
        }
        bzero(param_pipe_name, PIPE_NAME_LEN);
        memcpy(param_pipe_name, pipe_name, param_pipe_name_length);

        return stmBase::insert();
    }
};


class stm_pipe_messages_delete_by_message_id: public stmBase{

    friend stmMapper;
    char param_id[MYSQL_UUID_LEN];
    unsigned long param_id_length = MYSQL_UUID_LEN;
     
public:
    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(1);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *)&param_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_id_length;
        }
        return init(mysql, "DELETE FROM `pipe_messages` where id = ? ");
    }

    stm_pipe_messages_delete_by_message_id()
    { 
        bzero(param_id, MYSQL_UUID_LEN);
    }

    int execute(const char* id)
    {
        bzero(param_id, MYSQL_UUID_LEN);
        strncpy(param_id, id, MYSQL_UUID_LEN); 
          
        return stmBase::insert();
    }
};

class stm_users_auth_replace: public stmBase{

    friend stmMapper;
    unsigned long param_user_id = 0;

    char          param_hash[USER_HASH_LEN];
    unsigned long param_hash_length = USER_HASH_LEN;

public:
    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(2);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *)&param_hash;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_hash_length;
        }
        return init(mysql, "REPLACE INTO `users_auth`(`user_id`, `hash`) VALUES (?,?)");
    }

    stm_users_auth_replace()
    {
        bzero(param_hash, USER_HASH_LEN);
    }

    int execute(unsigned long user_id, const char* hash)
    {
        param_user_id = user_id;

        param_hash_length = strlen(hash);
        if(param_hash_length > USER_HASH_LEN)
        {
            param_hash_length = USER_HASH_LEN;
        }
        bzero(param_hash, USER_HASH_LEN);
        memcpy(param_hash, hash, param_hash_length);
        return stmBase::insert();
    }
};

class stm_users_auth_delete: public stmBase{

    friend stmMapper;
    unsigned long param_user_id = 0;

public:
    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(1);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;
        }
        return init(mysql, "delete from `users_auth` where `user_id` = ? ");
    }

    stm_users_auth_delete(){}

    int execute(unsigned long user_id)
    {
        param_user_id = user_id;

        return stmBase::insert();
    }
};

class stm_users_auth_select: public stmBase{

    friend stmMapper;
    unsigned long param_user_id = 0;

public:
    char          result_hash[USER_HASH_LEN];
    unsigned long result_hash_length = USER_HASH_LEN;

    my_bool       is_null[1];
    my_bool       error[1];
    unsigned long length[1];
 
    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(1);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            setResultsCount(1);

            i = 0; 
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_hash;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = USER_HASH_LEN;
        }
        return init(mysql, "SELECT `hash` FROM `users_auth` WHERE user_id = ? ");
    }

public:
    stm_users_auth_select()
    {
        bzero(result_hash, USER_HASH_LEN);
        bzero(is_null, 1);
        bzero(error, 1);
        bzero(length, 1); 
    }

    bool execute(unsigned long user_id)
    {
        // https://docs.oracle.com/cd/E17952_01/mysql-5.5-en/mysql-stmt-fetch.html
        param_user_id = user_id;

        return stmBase::select();
    }

    /**
     * Извлекает результаты селекта построчно.
     * @return
     */
    int fetch()
    {
        // Fetch
        int res = mysql_stmt_fetch(stmt);
        if(res == MYSQL_DATA_TRUNCATED)
        {
            TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), MYSQL_DATA_TRUNCATED\x1b[0m\n");
        }
        else if(res == 1)
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), 1 failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
        } 
        return res;
    }

    /**
     * Извлекает num_rows
     * @return
     */
    int num_rows()
    {
        // Fetch
        return mysql_stmt_num_rows(stmt);
    }

    /**
     * Очищает память выделеную для хранения результатов селекта
     * @return
     */
    bool free()
    {
        return stmBase::free();
    }
};

class stm_users_data_replace: public stmBase{

    friend stmMapper;
    unsigned long param_user_id = 0;

    char*          param_data = NULL;
    unsigned long param_data_length = 0;
  
public:
    bool virtual prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(2);

            if(param_data == NULL)
            {
                param_data = new char[appConf::instance()->get_int("db", "buf_size")];
                param_data_length = appConf::instance()->get_int("db", "buf_size");
            }
            
            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            i++;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *)&param_data;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_data_length;
        }
        return init(mysql, "REPLACE INTO `users_data`(`user_id`, `data`) VALUES (?,?)");
    }

    stm_users_data_replace()
    { 
    }

    int execute(unsigned long user_id, const char* data, int data_length)
    {
        param_user_id = user_id;
        
        if(data_length > appConf::instance()->get_int("db", "buf_size"))
        {
            data_length = appConf::instance()->get_int("db", "buf_size") - 1;
        }
        
        memcpy(param_data, data, data_length);
        param_data_length = data_length;
        
        return stmBase::insert();
    }
};

class stm_users_data_delete: public stmBase{

    friend stmMapper;
    unsigned long param_user_id = 0;

public:
    bool virtual prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(1);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;
        }
        return init(mysql, "delete from `users_data` where `user_id` = ? ");
    }

    stm_users_data_delete(){}

    int execute(unsigned long user_id)
    {
        param_user_id = user_id;

        return stmBase::insert();
    }
};

class stm_users_data_select: public stmBase{

    friend stmMapper;
    unsigned long param_user_id = 0;

public:
   
    char*         result_data = NULL;
    unsigned long result_data_length = 0;
    
    my_bool       is_null[2];
    my_bool       error[2];
    unsigned long length[2];

    bool virtual prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            
            if(result_data == NULL)
            {
                result_data = new char[appConf::instance()->get_int("db", "buf_size")];
                result_data_length = appConf::instance()->get_int("db", "buf_size");
            }
            
            setParamsCount(1);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            setResultsCount(1);

            i = 0; 
            result[i].buffer_type    = MYSQL_TYPE_STRING;
            result[i].buffer         = (char *)result_data;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
            result[i].buffer_length  = result_data_length;
        }
        return init(mysql, "SELECT `data` FROM `users_data` WHERE user_id = ? ");
    }

public:
    stm_users_data_select()
    { 
    }

    bool execute(unsigned long user_id)
    {
        // https://docs.oracle.com/cd/E17952_01/mysql-5.5-en/mysql-stmt-fetch.html
        param_user_id = user_id;

        return stmBase::select();
    }

    /**
     * Извлекает результаты селекта построчно.
     * @return 0 если успех или MYSQL_NO_DATA или MYSQL_DATA_TRUNCATED и -1 если ошибка
     */
    int fetch()
    {
        // Fetch
        int res = mysql_stmt_fetch(stmt);
        if(res == MYSQL_DATA_TRUNCATED)
        {
            TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), MYSQL_DATA_TRUNCATED\x1b[0m\n");
        }
        else if(res == 1)
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), 1 failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
        } 
        return res;
    }

    /**
     * Извлекает num_rows
     * @return
     */
    int num_rows()
    {
        // Fetch
        return mysql_stmt_num_rows(stmt);
    }

    /**
     * Очищает память выделеную для хранения результатов селекта
     * @return
     */
    bool free()
    {
        return stmBase::free();
    }
};

class stm_pipes_settings_select: public stmBase{

    friend stmMapper;
    
    char          param_pipe_name[PIPE_NAME_LEN];
    unsigned long param_pipe_name_length = PIPE_NAME_LEN;
    
public: 
    long          result_length = 0; 
    
    my_bool       is_null[1];
    my_bool       error[1];
    unsigned long length[1];

    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(1);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_STRING;
            param[i].buffer         = (void *)&param_pipe_name;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = &param_pipe_name_length;

            setResultsCount(1);

            i = 0; 
            result[i].buffer_type    = MYSQL_TYPE_LONG;
            result[i].buffer         = (void *)&result_length; 
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
        }
        return init(mysql, "SELECT `length` FROM `pipes_settings` WHERE `name` = ? limit 1");
    }

public:
    stm_pipes_settings_select()
    { 
        bzero(is_null, 1);
        bzero(error, 1);
        bzero(length, 1); 
        bzero(param_pipe_name, PIPE_NAME_LEN);
    }

    bool execute(const char* pipe_name)
    {
        // https://docs.oracle.com/cd/E17952_01/mysql-5.5-en/mysql-stmt-fetch.html
        param_pipe_name_length = strlen(pipe_name);
        if(param_pipe_name_length > PIPE_NAME_LEN)
        {
            param_pipe_name_length = PIPE_NAME_LEN;
        }
        bzero(param_pipe_name, PIPE_NAME_LEN);
        memcpy(param_pipe_name, pipe_name, param_pipe_name_length);

        return stmBase::select();
    }

    /**
     * Извлекает результаты селекта построчно.
     * @return
     */
    int fetch()
    {
        // Fetch
        int res = mysql_stmt_fetch(stmt);
        if(res == MYSQL_DATA_TRUNCATED)
        {
            TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), MYSQL_DATA_TRUNCATED\x1b[0m\n");
        }
        else if(res == 1)
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), 1 failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
        } 
        return res;
    }

    /**
     * Извлекает num_rows
     * @return
     */
    int num_rows()
    {
        // Fetch
        return mysql_stmt_num_rows(stmt);
    }

    /**
     * Очищает память выделеную для хранения результатов селекта
     * @return
     */
    bool free()
    {
        return stmBase::free();
    }
};

class stm_users_time_select: public stmBase{

    friend stmMapper;
    unsigned long param_user_id = 0; 
    
public: 
    long          result_time = 0; 
    
    my_bool       is_null[1];
    my_bool       error[1];
    unsigned long length[1];

    bool prepare(dbLink *mysql)
    {
        if(!isInited)
        {
            isInited = true;
            setParamsCount(1);

            int i = 0;
            param[i].buffer_type    = MYSQL_TYPE_LONG;
            param[i].buffer         = (void *)&param_user_id;
            param[i].is_unsigned    = 0;
            param[i].is_null        = 0;
            param[i].length         = 0;

            setResultsCount(1);

            i = 0; 
            result[i].buffer_type    = MYSQL_TYPE_LONG;
            result[i].buffer         = (void *)&result_time;
            result[i].is_null        = &is_null[i];
            result[i].error          = &error[i];
            result[i].length         = &length[i];
        }
        return init(mysql, "SELECT `time` FROM `users_time` WHERE `user_id` = ? ");
    }

public:
    stm_users_time_select()
    { 
        bzero(is_null, 1);
        bzero(error, 1);
        bzero(length, 1); 
    }

    bool execute(unsigned long user_id)
    {
        // https://docs.oracle.com/cd/E17952_01/mysql-5.5-en/mysql-stmt-fetch.html
        param_user_id = user_id; 
        return stmBase::select();
    }

    /**
     * Извлекает результаты селекта построчно.
     * @return
     */
    int fetch()
    {
        // Fetch
        int res = mysql_stmt_fetch(stmt);
        if(res == MYSQL_DATA_TRUNCATED)
        {
            TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), MYSQL_DATA_TRUNCATED\x1b[0m\n");
        }
        else if(res == 1)
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_fetch(), 1 failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
        } 
        return res;
    }

    /**
     * Извлекает num_rows
     * @return
     */
    int num_rows()
    {
        // Fetch
        return mysql_stmt_num_rows(stmt);
    }

    /**
     * Очищает память выделеную для хранения результатов селекта
     * @return
     */
    bool free()
    {
        return stmBase::free();
    }
};

class stmMapper{
public:
    stm_log_query *queryLoger = NULL;
    stm_users_queue_insert *users_queue_insert = NULL;
    stm_users_queue_select *users_queue_select = NULL;
    stm_users_queue_delete *users_queue_delete = NULL;

    stm_pipe_messages_insert *pipe_messages_insert = NULL;
    stm_pipe_messages_select *pipe_messages_select = NULL;
    stm_pipe_messages_delete *pipe_messages_delete = NULL;
    stm_pipe_messages_delete_by_message_id *pipe_messages_delete_by_message_id = NULL;

    stm_users_auth_replace *users_auth_replace = NULL;
    stm_users_auth_delete *users_auth_delete = NULL;
    stm_users_auth_select *users_auth_select = NULL;
 
    stm_users_data_replace *users_data_replace = NULL;
    stm_users_data_delete *users_data_delete = NULL;
    stm_users_data_select *users_data_select = NULL;
    
    stm_users_time_select *users_time_select = NULL;
    stm_pipes_settings_select *pipes_settings_select = NULL; 
    
    stm_conference_insert *conference_insert = NULL;
    stm_conference_select *conference_select = NULL;
    stm_conference_delete *conference_delete = NULL;
    stm_conference_select_nodes_for_room *conference_select_nodes_for_room = NULL;
    void init(dbLink *mysql);
};



#endif
