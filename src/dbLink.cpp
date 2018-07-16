// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "dbLink.h"
#include "CometQL.h"

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

void stmBase::setParamsCount(int size)
{
    if(param != NULL)
    {
        delete[] param;
    }
    param = new MYSQL_BIND[size];
    bzero(param, sizeof(MYSQL_BIND)*size);
    param_count = size;
}

void stmBase::setResultsCount(int size)
{
    if(result != NULL)
    {
        delete[] result;
    }
    result = new MYSQL_BIND[size];
    bzero(result, sizeof(MYSQL_BIND)*size);
    column_count = size;
}

bool stmBase::init(dbLink *mysql, const char* q)
{
    db = mysql;
    // https://docs.oracle.com/cd/E17952_01/mysql-5.5-en/mysql-stmt-execute.html
    // https://gist.github.com/hoterran/6365915
    
    if(stmt != NULL)
    {
        mysql_stmt_close(stmt);
    }
    
    stmt = mysql_stmt_init(db->getLink());
    if (stmt == NULL)
    {
        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mCould not initialize statement handler [query=%s]\x1b[0m\n", q);
        return false;
    }

    if(mysql_stmt_prepare(stmt, q, strlen(q)))
    {
        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_prepare(), INSERT failed - %s [query=%s]\x1b[0m\n", mysql_stmt_error(stmt), q);
        return false;
    }

    /* Get the parameter count from the statement */
    int count = mysql_stmt_param_count(stmt);
    if(count != param_count)
    {
        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_prepare Parameters not initialized: %d > %d [query=%s]\x1b[0m\n", count, param_count, q);
        return false;
    }

    // Bind param structure to statement
    if (param != NULL && mysql_stmt_bind_param(stmt, param))
    {
        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_bind_param(), failed - %s [query=%s]\x1b[0m\n", mysql_stmt_error(stmt), q);
        return false;
    }

    // Result
    if( result != NULL)
    {
        prepare_meta_result = mysql_stmt_result_metadata(stmt);
        if (!prepare_meta_result)
        {
            TagLoger::error(Log_dbLink, 0, "mysql_stmt_result_metadata(), returned no meta information - %s [query=%s]\x1b[0m\n", mysql_stmt_error(stmt), q);
            return false;
        }

        /* Get total columns in the query */
        count = mysql_num_fields(prepare_meta_result);
        if (column_count != count) /* validate column count */
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mtotal columns in select statement [query=%s]: %d != %d\x1b[0m\n", q, column_count, count);
            return false;
        }

        // Bind result
        if (mysql_stmt_bind_result(stmt, result))
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mCould not bind results - %s [query=%s]\x1b[0m\n", mysql_stmt_error(stmt), q);
            return false;
        }
    }
    return true;
}


/**
 * Выполняет вставку, обновление, замену, удаление
 * @rereturn -1 если ошибка и affected_rows если успех
 */
int stmBase::insert()
{
    if (mysql_stmt_execute(stmt) != 0)
    {
        int error_code = mysql_stmt_errno(stmt);
        if( error_code != CR_SERVER_LOST && error_code != ER_UNKNOWN_STMT_HANDLER && error_code != ER_SERVER_SHUTDOWN)
        {
            TagLoger::trace(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(insert), (1) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
            return -1;
        }
        
        TagLoger::log(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(insert), (1) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
        if(prepare(db))
        {
            if (mysql_stmt_execute(stmt) != 0)
            { 
                TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(insert), (2) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt)); 
                if(db->reconnect() && prepare(db))
                {
                    if (mysql_stmt_execute(stmt) != 0)
                    {
                        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(insert), (3) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));  
                        return -1;
                    }
                }
                else
                {
                    TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(insert), (4) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));  
                }
                return -1;
            }
        }
        else
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(insert), (5) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt)); 
            return -1;
        }
    }

    /* Get the number of affected rows */
    int affected_rows = mysql_stmt_affected_rows(stmt);
    return affected_rows;
}

/**
 * Выполняет выборку
 */
bool stmBase::select()
{
    if (mysql_stmt_execute(stmt))
    {
        int error_code = mysql_stmt_errno(stmt);
         
        if( error_code != CR_SERVER_LOST && error_code != ER_UNKNOWN_STMT_HANDLER && error_code != ER_SERVER_SHUTDOWN)
        {
            TagLoger::trace(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(select), (1) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
            return false;
        }
        
        TagLoger::log(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(select), (1) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt)); 
        if(prepare(db))
        {
            if (mysql_stmt_execute(stmt) != 0)
            {
                TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(select), (2) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
                if(db->reconnect() && prepare(db))
                {
                    if (mysql_stmt_execute(stmt) != 0)
                    {
                        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(select), (3) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));  
                        return -1;
                    }
                }
                else
                {
                    TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(select), (4) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));  
                }
                return -1; 
            }
        }
        else
        {
            TagLoger::error(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_execute(select), (5) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt)); 
            return false;
        }
    }

    if (mysql_stmt_store_result(stmt) != 0)
    {
        TagLoger::trace(Log_dbLink, 0, "\x1b[1;31mmysql_stmt_store_result(), (1) failed - %s [errno=%d]\x1b[0m\n", mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
        return false;
    }

    return true;
}

/**
 * Извлекает результаты селекта построчно.
 * @return
 */
int stmBase::fetch()
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
bool stmBase::free()
{
    // Deallocate result set
    if (mysql_stmt_free_result(stmt)) /*  deallocate result set*/
    {
        return true;
    }
    return false;
}

bool dbLink::init(std::string connectionString)
{ 
    int pos = 0;
    int nextPos = 0;

    std::string Server = "localhost";
    std::string Database;
    std::string Uid = "root";
    std::string Pwd;
    int Port = 3301;

    int i = 0;
    while(pos <= connectionString.length() && i < 10)
    {
        i++;
        nextPos = connectionString.find('=', pos);
        if(nextPos == std::string::npos)
        {
            break;
        }

        auto paramName = connectionString.substr(pos, nextPos - pos); 
        //TagLoger::debug(Log_dbLink, 0, "\x1b[1;32mparamName=%s, pos=%d, nextPos=%d\x1b[0m", paramName.data(), pos, nextPos);
        pos = nextPos + 1;

        nextPos = connectionString.find(',', pos);
        if(nextPos == std::string::npos)
        {
            nextPos = connectionString.length();
        }

        auto paramValue = connectionString.substr(pos, nextPos - pos); 
        //TagLoger::debug(Log_dbLink, 0, "\x1b[1;32mparamValue=%s, pos=%d, nextPos=%d\x1b[0m", paramValue.data(), pos, nextPos);
        pos = nextPos + 1;

        if(paramName.compare("Server") == 0)
        {
            Server = paramValue;
        } 
        else if(paramName.compare("Database") == 0)
        {
            Database = paramValue;
        } 
        else if(paramName.compare("Uid") == 0)
        {
            Uid = paramValue;
        } 
        else if(paramName.compare("Pwd") == 0)
        {
            Pwd = paramValue; 
        } 
        else if(paramName.compare("Port") == 0)
        {
            try{
                //printf("get_long [%s] %s=%s\n", section.data(), name.data(), sections.at(section).at(name).data());
                Port = std::stoi(paramValue);
            }catch(...)
            {
                printf("\x1b[1;31mexeption in parsing Port value Port=%s\x1b[0m\n", paramValue.data());
                return false;
            } 
        }
    }

    return init(Server.data(), Uid.data(), Pwd.data(), Database.data(), Port);
}

bool dbLink::init(const char* host, const char* user, const char* pw, const char* name, int port)
{
    if(host == NULL)
    { 
        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mCppComet MySQL connection `host` is NULL\x1b[0m");
        return false;
    }

    if(pw == NULL)
    { 
        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mCppComet MySQL connection `password` is NULL\x1b[0m");
        return false;
    }

    if(user == NULL)
    { 
        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mCppComet MySQL connection `user` is NULL\x1b[0m");
        return false;
    }

    if(name == NULL)
    { 
        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mCppComet MySQL connection `db_name` is NULL\x1b[0m");
        return false;
    }

    if(port <= 0)
    {
        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mCppComet MySQL connection `port` is %d\x1b[0m", port);
        return false;
    }

    TagLoger::log(Log_dbLink, 0, "init dbLink host=%s, user=%s, name=%s, port=%d\n", host, user, name, port);  
    isInit = true; 
    db_host = host;
    db_pw = pw;
    db_user = user;
    db_name = name;

    db_port = port;

    return true;
}

bool dbLink::connect()
{
    if(!isInit)
    {
        TagLoger::trace(Log_dbLink, 0, "\x1b[1;32mMySQL connection was not initialized\x1b[0m");
        return false;
    }
    
    auto t = TagTimer::mtime();
    
    isConnected = true;

    mysqlLink = mysql_init(mysqlLink);

    mysql_options(mysqlLink, MYSQL_OPT_RECONNECT, &is_reconnect);
    mysql_options(mysqlLink, MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(mysqlLink, MYSQL_INIT_COMMAND, "SET NAMES utf8");
    mysql_options(mysqlLink, MYSQL_OPT_CONNECT_TIMEOUT, &connect_timeout);
    mysql_options(mysqlLink, MYSQL_OPT_READ_TIMEOUT, &read_timeout);

    // MYSQL_OPT_CONNECT_TIMEOUT
    // MYSQL_OPT_READ_TIMEOUT
    // MYSQL_OPT_WRITE_TIMEOUT
    //bool ssl = false;
    //mysql_options(mysqlLink,MYSQL_OPT_COMPRESS, &ssl);
    //mysql_options(mysqlLink,MYSQL_OPT_SSL_MODE, &ssl);

    TagLoger::debug(Log_dbLink, 0, "\x1b[1;32mmysql_real_connect %s:%d user=%s\x1b[0m", db_host.data(), db_port, db_user.data());
    mysql_real_connect(mysqlLink, db_host.data(), db_user.data(), db_pw.data(), db_name.data(), db_port, NULL, 0);

    if(mysql_errno(mysqlLink))
    {
        TagLoger::error(Log_dbLink, 0, "\x1b[1;31mMySQL connection not established\n%s\nip=%s:%d user=%s password=%s db_name=%s [errno=%d]\x1b[0m",
                mysql_error(mysqlLink),
                db_host.data(),
                db_port,
                db_user.data(),
                db_pw.data(),
                db_name.data(),
                mysql_errno(mysqlLink)
        );
        TagTimer::add("dbLink::connect", t); 
        return false;
    }
 
    if(stm != NULL)
    {
        stm->init(this);
    }

    TagLoger::log(Log_dbLink, 0, "\x1b[1;32mMySQL connection established\nip=%s:%d user=%s\x1b[0m", db_host.data(), db_port, db_user.data());
    TagTimer::add("dbLink::connect", t); 
    return true;

    //return query("SET CHARACTER SET 'utf8' ");
}


bool dbLink::query(const char *q)
{
    auto t = TagTimer::mtime();
    
    if(!isConnected)
    {
        connect();
    }

    TagLoger::log(Log_dbLink, 0, "\x1b[1;32mMySQL query[%d]=%s\x1b[0m", strlen(q), q);
    if(mysql_real_query(mysqlLink, q, strlen(q)) == 0)
    {
        TagTimer::add("dbLink::query", t); 
        return true;
    }


    if(mysql_errno(mysqlLink))
    {
        TagLoger::warn(Log_dbLink, 0, "\x1b[1;33mMySQL (warn)error=%s [errno=%d] [query=%s]\x1b[0m", mysql_error(mysqlLink), mysql_errno(mysqlLink), q);
        if(mysql_errno(mysqlLink) == CR_COMMANDS_OUT_OF_SYNC)
        {
            MYSQL_RES* res = mysql_store_result(mysqlLink);
            mysql_free_result(res);

            if(mysql_real_query(mysqlLink, q, strlen(q)) == 0)
            {
                TagTimer::add("dbLink::query", t); 
                return true;
            }

            TagLoger::warn(Log_dbLink, 0, "\x1b[1;33mMySQL (warn[2])error=%s [errno=%d] [query=%s]\x1b[0m", mysql_error(mysqlLink), mysql_errno(mysqlLink), q);
        }

        if(reconnect_on_error && reconnect())
        {
            if(mysql_real_query(mysqlLink, q, strlen(q)) == 0)
            {
                TagTimer::add("dbLink::query", t); 
                return true;
            }

            int err = mysql_errno(mysqlLink);
            if(err)
            {
                if(err == SQL_ERR_READ_ONLY || err == SQL_ERR_OVERFLOW)
                {
                    TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mMySQL error=%s [errno=%d] [query=%s]\x1b[0m", mysql_error(mysqlLink), mysql_errno(mysqlLink), q);
                }
                else
                {
                    TagLoger::trace(Log_dbLink, 0, "\x1b[1;31mMySQL error=%s [errno=%d] [query=%s]\x1b[0m", mysql_error(mysqlLink), mysql_errno(mysqlLink), q);
                }
            }
        } 
    }

    TagTimer::add("dbLink::query", t); 
    return false;
}

bool dbLink::query_format(const char *format, ...)
{
    auto t = TagTimer::mtime();
    if(!isConnected)
    {
        connect();
    }

    int buffer_size = 2000;
    char buf[2000];
    bzero(buf, buffer_size);
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, buffer_size, format, ap);

    TagLoger::log(Log_dbLink, 0, "\x1b[1;32mMySQL query[%d]=%s\x1b[0m", strlen(buf), buf);
    if(mysql_real_query(mysqlLink, buf, strlen(buf)) == 0)
    {
        va_end(ap);
        TagTimer::add("dbLink::query_format", t); 
        return true;
    }

    if(mysql_errno(mysqlLink))
    {
        // @todo Проверять код ошибки и не паниковать если по коду ясно что проблема в самом запросе а не соединении. 
        TagLoger::warn(Log_dbLink, 0, "\x1b[1;33mMySQL (warn)error=%s [errno=%d] [query=%s]\x1b[0m", mysql_error(mysqlLink), mysql_errno(mysqlLink), buf); 
        if(mysql_errno(mysqlLink) == CR_COMMANDS_OUT_OF_SYNC)
        {
            MYSQL_RES* res = mysql_store_result(mysqlLink);
            mysql_free_result(res);

            if(mysql_real_query(mysqlLink, buf, strlen(buf)) == 0)
            {
                TagTimer::add("dbLink::query_format", t); 
                return true;
            }

            TagLoger::warn(Log_dbLink, 0, "\x1b[1;33mMySQL (warn[2])error=%s [errno=%d] [query=%s]\x1b[0m", mysql_error(mysqlLink), mysql_errno(mysqlLink), buf);
        }

        if(reconnect_on_error && reconnect())
        {
            if(mysql_real_query(mysqlLink, buf, strlen(buf)) == 0)
            {
                va_end(ap);
                TagTimer::add("dbLink::query_format", t); 
                return true;
            }

            int err = mysql_errno(mysqlLink);
            if(err)
            {
                if(err == SQL_ERR_READ_ONLY || err == SQL_ERR_OVERFLOW)
                { 
                    TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mMySQL error=%s [errno=%d] [query=%s]\x1b[0m", mysql_error(mysqlLink), mysql_errno(mysqlLink), buf);
                }
                else
                {
                    TagLoger::trace(Log_dbLink, 0, "\x1b[1;31mMySQL error=%s [errno=%d] [query=%s]\x1b[0m", mysql_error(mysqlLink), mysql_errno(mysqlLink), buf);
                }
            }
        } 
    }

    va_end(ap);
    TagTimer::add("dbLink::query_format", t); 
    return false;
}

bool dbLink::reconnect()
{
    close();
    TagLoger::warn(Log_dbLink, 0, "\x1b[1;31mMySQL reconnect\x1b[0m");
    return connect();
}

void dbLink::close()
{
    if(isConnected)
    {
        mysql_close(mysqlLink);
        mysqlLink = NULL;
        isConnected = false;
    } 
}


void stmMapper::init(dbLink *mysql)
{
    // \.(execute|fetch|result_id|result_event|result_message|free|result_length|result_time|result_user_id)
    if(queryLoger == NULL)
    {
        queryLoger = new stm_log_query(); 
    } 
    queryLoger->prepare(mysql);
    
    if(users_queue_insert == NULL)
    {
        users_queue_insert = new stm_users_queue_insert(); 
    }
    users_queue_insert->prepare(mysql);
    
    if(users_queue_select == NULL)
    {
        users_queue_select = new stm_users_queue_select(); 
    } 
    users_queue_select->prepare(mysql);
    
    if(users_queue_delete == NULL)
    {
        users_queue_delete = new stm_users_queue_delete(); 
    }  
    users_queue_delete->prepare(mysql);

    
    if(pipe_messages_insert == NULL)
    {
        pipe_messages_insert = new stm_pipe_messages_insert(); 
    } 
    pipe_messages_insert->prepare(mysql);
    
    if(pipe_messages_select == NULL)
    {
        pipe_messages_select = new stm_pipe_messages_select(); 
    } 
    pipe_messages_select->prepare(mysql);
    
    if(pipe_messages_delete == NULL)
    {
        pipe_messages_delete = new stm_pipe_messages_delete(); 
    } 
    pipe_messages_delete->prepare(mysql);
    
    if(pipe_messages_delete_by_message_id == NULL)
    {
        pipe_messages_delete_by_message_id = new stm_pipe_messages_delete_by_message_id(); 
    } 
    pipe_messages_delete_by_message_id->prepare(mysql);

    
    if(users_auth_replace == NULL)
    {
        users_auth_replace = new stm_users_auth_replace(); 
    } 
    users_auth_replace->prepare(mysql);
    
    if(users_auth_delete == NULL)
    {
        users_auth_delete = new stm_users_auth_delete(); 
    } 
    users_auth_delete->prepare(mysql);
    
    if(users_auth_select ==NULL)
    {
        users_auth_select = new stm_users_auth_select(); 
    } 
    users_auth_select->prepare(mysql);

    if(users_data_replace == NULL)
    {
        users_data_replace = new stm_users_data_replace(); 
    } 
    users_data_replace->prepare(mysql);
    
    if(users_data_delete == NULL)
    {
        users_data_delete = new stm_users_data_delete(); 
    } 
    users_data_delete->prepare(mysql);
    
    if(users_data_select ==NULL)
    {
        users_data_select = new stm_users_data_select(); 
    } 
    users_data_select->prepare(mysql);

    

    
    if(pipes_settings_select == NULL)
    {
        pipes_settings_select = new stm_pipes_settings_select(); 
    } 
    pipes_settings_select->prepare(mysql);
    
    if(users_time_select == NULL)
    {
        users_time_select = new stm_users_time_select(); 
    } 
    users_time_select->prepare(mysql);
    
    if(conference_insert == NULL)
    {
        conference_insert = new stm_conference_insert(); 
    } 
    conference_insert->prepare(mysql);
    
    if(conference_delete == NULL)
    {
        conference_delete = new stm_conference_delete(); 
    } 
    conference_delete->prepare(mysql);
    
    if(conference_select == NULL)
    {
        conference_select = new stm_conference_select(); 
    } 
    conference_select->prepare(mysql);
    
    if(revoked_tokens_delete == NULL)
    {
        revoked_tokens_delete = new stm_revoked_tokens_delete(); 
    } 
    revoked_tokens_delete->prepare(mysql);
    
    if(revoked_tokens_select == NULL)
    {
        revoked_tokens_select = new stm_revoked_tokens_select(); 
    } 
    revoked_tokens_select->prepare(mysql);
    
    if(revoked_tokens_replace == NULL)
    {
        revoked_tokens_replace = new stm_revoked_tokens_replace(); 
    }
    revoked_tokens_replace->prepare(mysql);
    
    if(conference_select_nodes_for_room == NULL)
    {
        conference_select_nodes_for_room = new stm_conference_select_nodes_for_room(); 
    } 
    conference_select_nodes_for_room->prepare(mysql);
}