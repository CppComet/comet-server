// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <mysql.h>

#include "appConf.h"

#ifndef DBLINK_H
#define	DBLINK_H


class stmMapper;

class stmBase{

protected:
    MYSQL_STMT    *stmt = NULL;
    MYSQL_BIND    *param = NULL;
    MYSQL_BIND    *result = NULL;
    MYSQL_RES     *prepare_meta_result = NULL;

    int param_count = 0;
    int column_count = 0;

    void setParamsCount(int size)
    {
        if(param != NULL)
        {
            delete[] param;
        }
        param = new MYSQL_BIND[size];
        bzero(param, sizeof(MYSQL_BIND)*size);
        param_count = size;
    }

    void setResultsCount(int size)
    {
        if(result != NULL)
        {
            delete[] result;
        }
        result = new MYSQL_BIND[size];
        bzero(result, sizeof(MYSQL_BIND)*size);
        column_count = size;
    }

    bool init(MYSQL *mysql, const char* q)
    {
        // https://docs.oracle.com/cd/E17952_01/mysql-5.5-en/mysql-stmt-execute.html
        // https://gist.github.com/hoterran/6365915
        stmt = mysql_stmt_init(mysql);
        if (stmt == NULL)
        {
            TagLoger::error(Log_dbLink, 0, "Could not initialize statement handler [query=%s]\n", q);
            return false;
        }

        if(mysql_stmt_prepare(stmt, q, strlen(q)))
        {
            TagLoger::error(Log_dbLink, 0, "mysql_stmt_prepare(), INSERT failed - %s [query=%s]\n", mysql_stmt_error(stmt), q);
            return false;
        }

        /* Get the parameter count from the statement */
        int count = mysql_stmt_param_count(stmt);
        if(count != param_count)
        {
            TagLoger::error(Log_dbLink, 0, "mysql_stmt_prepare parameters not initialized: %d > %d [query=%s]\n", count, param_count, q);
        }

        // Bind param structure to statement
        if (param != NULL && mysql_stmt_bind_param(stmt, param))
        {
            TagLoger::error(Log_dbLink, 0, "mysql_stmt_bind_param(), failed - %s [query=%s]\n", mysql_stmt_error(stmt), q);
            return false;
        }

        // Result
        if( result != NULL)
        {
            prepare_meta_result = mysql_stmt_result_metadata(stmt);
            if (!prepare_meta_result)
            {
                TagLoger::error(Log_dbLink, 0, "mysql_stmt_result_metadata(), returned no meta information - %s [query=%s]\n", mysql_stmt_error(stmt), q);
                return false;
            }

            /* Get total columns in the query */
            count = mysql_num_fields(prepare_meta_result);
            if (column_count != count) /* validate column count */
            {
                TagLoger::error(Log_dbLink, 0, "total columns in select statement [query=%s]: %d != %d\n", q, column_count, count);
                return false;
            }

            // Bind result
            if (mysql_stmt_bind_result(stmt, result))
            {
                TagLoger::error(Log_dbLink, 0, "Could not bind results - %s [query=%s]\n", mysql_stmt_error(stmt), q);
                return false;
            }
        }
        return true;
    }


    /**
     * Выполняет вставку, обновление, замену, удаление
     */
    int insert()
    {
        if (mysql_stmt_execute(stmt) != 0)
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(), 1 failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
            return -1;
        }

        /* Get the number of affected rows */
        int affected_rows = mysql_stmt_affected_rows(stmt);
        return affected_rows;
    }

    /**
     * Выполняет выборку
     */
    bool select()
    {
        if (mysql_stmt_execute(stmt))
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(), 1 failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
            return false;
        }

        if (mysql_stmt_store_result(stmt) != 0)
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(), 1 failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
            return false;
        }

        return true;
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
        // Deallocate result set
        if (mysql_stmt_free_result(stmt)) /*  deallocate result set*/
        {
            return true;
        }
        return false;
    }
};

class dbLink {
    MYSQL mysqlLink;
    bool isInit = false;

    char db_host[200];
    char db_pw[200];
    char db_user[200];
    char db_name[200];
    int db_port = 3306;
    my_bool reconnect = 1;
public:

    dbLink(){
        bzero(db_host,200);
        bzero(db_pw,200);
        bzero(db_user,200);
        bzero(db_name,200);
        mysql_init(&mysqlLink);

    }

    ~dbLink(){

        if(isInit)
        {
            mysql_close(&mysqlLink);
        }
    }

    bool init(const char* host, const char* user, const char* pw, const char* name, int port)
    {
        isInit = true;
        bzero(db_host,200);
        strncpy(db_host, host, 200);

        bzero(db_pw,200);
        strncpy(db_pw, pw, 200);

        bzero(db_user,200);
        strncpy(db_user, user, 200);

        bzero(db_name,200);
        strncpy(db_name, name, 200);

        db_port = port;

        mysql_options(&mysqlLink, MYSQL_OPT_RECONNECT, &reconnect);
        mysql_options(&mysqlLink, MYSQL_SET_CHARSET_NAME, "utf8");
        mysql_options(&mysqlLink, MYSQL_INIT_COMMAND, "SET NAMES utf8");
        return true;
    }

    MYSQL* getLink()
    {
        return &mysqlLink;
    }

    operator MYSQL* ()
    {
        return &mysqlLink;
    }

    bool query(const char *q)
    {
        TagLoger::log(Log_dbLink, 0, "\x1b[1;32mMySQL query[%d]=%s\x1b[0m", strlen(q), q);
        if(mysql_real_query(&mysqlLink, q, strlen(q)) == 0)
        {
            return true;
        }

        if(mysql_errno(&mysqlLink))
        {
            TagLoger::log(Log_dbLink, 0, "\x1b[1;31mMySQL error=%s [errno=%d] [query=%s]\x1b[0m", mysql_error(&mysqlLink), mysql_errno(&mysqlLink), q);
        }

        return false;
    }

    bool query_format(const char *format, ...)
    {
        int buffer_size = 2000;
        char buf[2000];
        bzero(buf, buffer_size);
        va_list ap;
        va_start(ap, format);
        vsnprintf(buf, buffer_size, format, ap);

        TagLoger::log(Log_dbLink, 0, "\x1b[1;32mMySQL query[%d]=%s\x1b[0m", strlen(buf), buf);
        if(mysql_real_query(&mysqlLink, buf, strlen(buf)) == 0)
        {
            va_end(ap);
            return true;
        }

        if(mysql_errno(&mysqlLink))
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mMySQL error=%s [errno=%d] [query=%s]\x1b[0m\n", mysql_error(&mysqlLink), mysql_errno(&mysqlLink), buf);
        }

        va_end(ap);
        return false;
    }

    bool connect()
    {
        mysql_real_connect(&mysqlLink, db_host, db_user, db_pw, db_name, db_port, NULL, 0);

        if(mysql_errno(&mysqlLink))
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mMySQL connection not established\n%s\nip=%s:%d user=%s [errno=%d]\x1b[0m", mysql_error(&mysqlLink),
                    db_host, db_port, db_user, mysql_errno(&mysqlLink));
            return false;
        }


        TagLoger::log(Log_dbLink, 0, "\x1b[1;32mMySQL connection established\nip=%s:%d user=%s\x1b[0m", db_host, db_port, db_user);
        return true;

        //return query("SET CHARACTER SET 'utf8' ");
    }
};

class stm_log_query: public stmBase{

    friend stmMapper;
protected:

    char*         param_query = NULL;
    unsigned long param_query_length = 0;

    bool prepare(MYSQL *mysql)
    {
        setParamsCount(1);
        param_query = new char[appConf::instance()->get_int("main", "buf_size")];

        int i = 0;
        param[i].buffer_type    = MYSQL_TYPE_STRING;
        param[i].buffer         = (char *)param_query;
        param[i].is_unsigned    = 0;
        param[i].is_null        = 0;
        param[i].length         = &param_query_length;

        return init(mysql, "INSERT INTO `log_query`(`id`, `query`) VALUES (NULL, ?)");
    }

public:
    stm_log_query(){}

    int insert(const char* query, unsigned long length)
    {
        strncpy(param_query, query, appConf::instance()->get_int("main", "buf_size"));
        param_query_length = length;
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
    bool prepare(MYSQL *mysql)
    {
        setParamsCount(5);

        param_message = new char[appConf::instance()->get_int("main", "buf_size")];
        param_message_length = appConf::instance()->get_int("main", "buf_size");

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

        param_message_length = message_length;

        bzero(param_id, MYSQL_UUID_LEN);
        bzero(param_event, EVENT_NAME_LEN);
        bzero(param_message, appConf::instance()->get_int("main", "buf_size"));

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
        strncpy(param_message, message, appConf::instance()->get_int("main", "buf_size"));

        return stmBase::insert();
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

    bool prepare(MYSQL *mysql)
    {
        result_message = new char[appConf::instance()->get_int("main", "buf_size")];
        result_message_length = appConf::instance()->get_int("main", "buf_size");

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
    bool prepare(MYSQL *mysql)
    {
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
    bool prepare(MYSQL *mysql)
    {
        setParamsCount(6);

        param_message = new char[appConf::instance()->get_int("main", "buf_size")];
        param_message_length = appConf::instance()->get_int("main", "buf_size");

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
        bzero(param_message, appConf::instance()->get_int("main", "buf_size"));

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
        strncpy(param_message, message, appConf::instance()->get_int("main", "buf_size"));

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

    bool prepare(MYSQL *mysql)
    {
        result_message = new char[appConf::instance()->get_int("main", "buf_size")];
        result_message_length = appConf::instance()->get_int("main", "buf_size");
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

        return init(mysql, "SELECT `id`, `time`, `event`, `message`, `user_id` FROM `pipe_messages` WHERE pipe_name = ? order by time limit ?");
    }

public:
    stm_pipe_messages_select()
    {
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
    bool prepare(MYSQL *mysql)
    {
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
    bool prepare(MYSQL *mysql)
    {
        setParamsCount(1);

        int i = 0;
        param[i].buffer_type    = MYSQL_TYPE_STRING;
        param[i].buffer         = (void *)&param_id;
        param[i].is_unsigned    = 0;
        param[i].is_null        = 0;
        param[i].length         = &param_id_length;
 
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
    bool prepare(MYSQL *mysql)
    {
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
    bool prepare(MYSQL *mysql)
    {
        setParamsCount(1);

        int i = 0;
        param[i].buffer_type    = MYSQL_TYPE_LONG;
        param[i].buffer         = (void *)&param_user_id;
        param[i].is_unsigned    = 0;
        param[i].is_null        = 0;
        param[i].length         = 0;

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

    my_bool       is_null[3];
    my_bool       error[3];
    unsigned long length[3];

    bool prepare(MYSQL *mysql)
    {
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
        
        return init(mysql, "SELECT `hash` FROM `users_auth` WHERE user_id = ? ");
    }

public:
    stm_users_auth_select()
    {
        bzero(result_hash, USER_HASH_LEN);
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

class stm_pipes_settings_select: public stmBase{

    friend stmMapper;
    
    char          param_pipe_name[PIPE_NAME_LEN];
    unsigned long param_pipe_name_length = PIPE_NAME_LEN;
    
public: 
    long          result_length = 0; 
    
    my_bool       is_null[1];
    my_bool       error[1];
    unsigned long length[1];

    bool prepare(MYSQL *mysql)
    {
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
        result[i].is_unsigned    = 0;
        result[i].is_null        = 0;
        result[i].length         = 0;
          
        return init(mysql, "SELECT `length` FROM `pipes_settings` WHERE `name` = ? limit 1");
    }

public:
    stm_pipes_settings_select()
    { 
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

    bool prepare(MYSQL *mysql)
    {
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
        result[i].is_unsigned    = 0;
        result[i].is_null        = 0;
        result[i].length         = 0;
          
        return init(mysql, "SELECT `time` FROM `users_time` WHERE `user_id` = ? ");
    }

public:
    stm_users_time_select()
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
    stm_log_query queryLoger;
    stm_users_queue_insert users_queue_insert;
    stm_users_queue_select users_queue_select;
    stm_users_queue_delete users_queue_delete;

    stm_pipe_messages_insert pipe_messages_insert;
    stm_pipe_messages_select pipe_messages_select;
    stm_pipe_messages_delete pipe_messages_delete;
    stm_pipe_messages_delete_by_message_id pipe_messages_delete_by_message_id;

    stm_users_auth_replace users_auth_replace;
    stm_users_auth_delete users_auth_delete;
    stm_users_auth_select users_auth_select;
 
    stm_users_time_select users_time_select;
    stm_pipes_settings_select pipes_settings_select;
    
    void init(MYSQL *mysql)
    {
        queryLoger.prepare(mysql);
        users_queue_insert.prepare(mysql);
        users_queue_select.prepare(mysql);
        users_queue_delete.prepare(mysql);

        pipe_messages_insert.prepare(mysql);
        pipe_messages_select.prepare(mysql);
        pipe_messages_delete.prepare(mysql);
        pipe_messages_delete_by_message_id.prepare(mysql);

        users_auth_replace.prepare(mysql);
        users_auth_delete.prepare(mysql);
        users_auth_select.prepare(mysql);
         
        pipes_settings_select.prepare(mysql);
        users_time_select.prepare(mysql);
    }
};



#endif
