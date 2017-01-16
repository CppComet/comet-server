// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   Client_connection.cpp
 * Author: levha
 *
 * Created on 01 Май 2015 г., 13:20
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <vector>
#include <map>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "mystring.h"
#include "user_index.h"
#include "internalApi.h"
#include "Client_connection.h"
#include "MySql_connection.h"

#include "CometQL.h"
#include "y.tab.h"
#include "sha1.h" 
#include "devManager.h"
#include <ctime>

#include <openssl/sha.h>

#include "sha1.h"


SqlTable* MySql_connection::tables;
int _countUerys = 0;
pthread_mutex_t MySql_connection::QLParsing_mutex;

/**
 * Рассчитывает получить валидный текст сообщения и валидные pipe_name и event_name
 * @param local_buf
 * @param pipe_name имя канала
 * @param event_name имя события
 * @param msg сообщение (уже с экранированными данными)
 * @param quote Кавычки
 * @return
 *
 * @todo добавить обработку authType в логирование канала.
 */
int PipeLog::addToLog(thread_data* local_buf, const char* pipe_name, const char* event_name, const char* auth_type, unsigned int from_user_id, const char* msg,  unsigned int msg_length)
{
    char def_auth_type[] = "*";
    if(auth_type == NULL || strlen(auth_type) == 0)
    {
        auth_type = def_auth_type;
    }

    char def_event_name[] = "undefined";
    if(event_name == NULL || strlen(event_name) == 0)
    {
        event_name = def_event_name;
    }

    if(pipe_name == NULL)
    {
        return -1;
    }

    if(msg == NULL)
    {
        return -1;
    }

    char uid[37];
    bzero(uid, 37);
    uid37(uid);
    if(memcmp("trust_", pipe_name, 6) == 0)
    {
        local_buf->stm.pipe_messages_insert.execute(uid, (long int)time(NULL), pipe_name, event_name, msg, msg_length, from_user_id);
        return 0;
    }


    local_buf->stm.pipes_settings_select.execute(pipe_name);
    if(local_buf->stm.pipes_settings_select.fetch())
    {
        local_buf->stm.pipes_settings_select.free();
        return 0;
    }

    int result_length = local_buf->stm.pipes_settings_select.result_length;
    local_buf->stm.pipes_settings_select.free();

    if(result_length > 0)
    {
        // Вставка в бд
        local_buf->stm.pipe_messages_insert.execute(uid, (long int)time(NULL), pipe_name, event_name, msg, msg_length, from_user_id);
        
        // @todo Заменить потом на stm выражение 
        local_buf->db.query_format("delete from pipe_messages where pipe_messages.id in( \
                                        select p2.id from ( \
                                            select id FROM `pipe_messages` as p3 where p3.pipe_name = '%s' order by p3.time desc limit %d, 999\
                                        ) as p2\
                                    )", pipe_name, result_length);
    }
    return 0;
}
 
MySql_connection::MySql_connection():connection()
{
    //printf("create MySql_connection\n");
    bzero(random20bytes, 20);
}

MySql_connection::~MySql_connection()
{
    //printf("delete MySql_connection\n");
}

#define table_users_auth tables[0]
#define table_users_time tables[1]
#define table_users_messages tables[2]
#define table_pipes_messages tables[3]
#define table_users_in_pipes tables[4]
#define table_pipes tables[5]
#define table_pipes_settings tables[6]

/**
 * Инициализация структуры таблиц
 */
void MySql_connection::initTables()
{
    if(tables != NULL)
    {
        return;
    }

    tables = new SqlTable[MYSQL_TABLES_COUNT];
    bzero(tables, sizeof(SqlTable)*MYSQL_TABLES_COUNT);

    table_users_auth.setName("users_auth");
    table_users_auth.setColumsCount(MAX_COLUMNS_COUNT);
    table_users_auth.setColumDef(0, "id", "int");
    table_users_auth.setColumDef(1, "hash", "char(32)");


    table_users_time.setName("users_time");
    table_users_time.setColumsCount(MAX_COLUMNS_COUNT);
    table_users_time.setColumDef(0, "id", "int(10)");
    table_users_time.setColumDef(1, "time", "int(10)");


    table_users_messages.setName("users_messages");
    table_users_messages.setColumsCount(MAX_COLUMNS_COUNT);
    table_users_messages.setColumDef(0, "id", "int(10)");
    table_users_messages.setColumDef(1, "index", "int(2)");
    table_users_messages.setColumDef(1, "event", "varchar(32)");
    table_users_messages.setColumDef(1, "message", "text");


    table_pipes_messages.setName("pipes_messages");
    table_pipes_messages.setColumsCount(MAX_COLUMNS_COUNT);
    table_pipes_messages.setColumDef(0, "name", "int(10)");
    table_pipes_messages.setColumDef(1, "index", "int(2)");
    table_pipes_messages.setColumDef(1, "event", "varchar(32)");
    table_pipes_messages.setColumDef(1, "message", "text");
    table_pipes_messages.setColumDef(1, "auth_type", "char(1)");
    table_pipes_messages.setColumDef(1, "user_id", "int(10)");


    table_users_in_pipes.setName("users_in_pipes");
    table_users_in_pipes.setColumsCount(MAX_COLUMNS_COUNT);
    table_users_in_pipes.setColumDef(0, "name", "varchar(32)");
    table_users_in_pipes.setColumDef(1, "user_id", "int(10)");


    table_pipes.setName("pipes");
    table_pipes.setColumsCount(MAX_COLUMNS_COUNT);
    table_pipes.setColumDef(0, "name", "varchar(32)");
    table_pipes.setColumDef(1, "users", "text");


    table_pipes_settings.setName("pipes_settings");
    table_pipes_settings.setColumsCount(MAX_COLUMNS_COUNT);
    table_pipes_settings.setColumDef(0, "name", "varchar(32)");
    table_pipes_settings.setColumDef(1, "length", "int(2)");
}

/**
 * Обрабатывает сообщения от клиентов
 * @param client идентификатор клиента
 * @param text Сообщение
 * @param len Длина сообщения
 * @return 0 в случае успеха
 *
 */
int MySql_connection::request(int client, int len, thread_data* local_buf)
{
    /**
     * @fixme Тут нет проверок на то что покет мог прийти не полностью
     *
     * Хосты должны поддерживать передачу пакетов размером до 576 байт, но современные реализации обычно поддерживают гораздо больший размер.
     * Пакеты большего размера, чем поддерживает канал связи, фрагментируются.
     */

    // @FixMe Перепроверить всё общение по протоколу ещё раз так как не валидный пакет может убить систему!


    // если len = 0 то соединение закрыто.
    if(TagLoger::isLog(Log_MySqlServer, TAGLOG_LOG))
    {
        TagLoger::log(Log_MySqlServer, 0, "Обработка mySQL запроса ------------------------[%d]\n", local_buf->buf[len-1]);
        for(int i=0; i < len; i++)
        {
            if(i % 8 == 0)  TagLoger::log(Log_MySqlServer, 0, "!\n%d:\t", i);
            char c = (char)local_buf->buf[i];
            if(c < 32 ) c = '.';
            TagLoger::log(Log_MySqlServer, 0, "!%02x [%c] ",  (unsigned char)local_buf->buf[i], c );
        }
        TagLoger::log(Log_MySqlServer, 0, "!\n ");
    }

    unsigned int PacketLen = 0;
    char* p = local_buf->buf.getData();

    char* t = (char*)&PacketLen;
    t[0] = p[0];
    t[1] = p[1];
    t[2] = p[2];
    t[3] = 0;
    p+=3;
    TagLoger::log(Log_MySqlServer, 0, "PacketLen:%d\n", PacketLen); // @FixMe добавить проверку длины пакета и длины данных в обе стороны.
 
    unsigned char PacketNomber = *p;
    p++;
    TagLoger::log(Log_MySqlServer, 0, "PacketNomber:%d\n", PacketNomber);

    if(clientState == STATE_SEND_HANDSHAKE)
    {
        memcpy(&ClientFlags, p,4);
        p+=4;
        TagLoger::log(Log_MySqlServer, 0, "ClientFlags:%d\n", ClientFlags);

        if(ClientFlags & MYSQL_CLIENT_LONG_PASSWORD)                  TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_LONG_PASSWORD\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_FOUND_ROWS)                     TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_FOUND_ROWS\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_LONG_FLAG)                      TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_LONG_FLAG\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_CONNECT_WITH_DB)                TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_CONNECT_WITH_DB\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_NO_SCHEMA)                      TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_NO_SCHEMA\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_COMPRESS)                       TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_COMPRESS\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_ODBC)                           TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_ODBC\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_LOCAL_FILES)                    TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_LOCAL_FILES\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_IGNORE_SPACE)                   TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_IGNORE_SPACE\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_PROTOCOL_41)                    TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_PROTOCOL_41\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_INTERACTIVE)                    TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_INTERACTIVE\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_SSL)                            TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_SSL\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_IGNORE_SIGPIPE)                 TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_IGNORE_SIGPIPE\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_TRANSACTIONS)                   TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_TRANSACTIONS\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_RESERVED)                       TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_RESERVED\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_SECURE_CONNECTION)              TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_SECURE_CONNECTION\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_MULTI_STATEMENTS)               TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_MULTI_STATEMENTS\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_MULTI_RESULTS)                  TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_MULTI_RESULTS\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_PS_MULTI_RESULTS)               TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_PS_MULTI_RESULTS\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_PLUGIN_AUTH )                   TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_PLUGIN_AUTH\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_CONNECT_ATTRS)                  TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_CONNECT_ATTRS\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA) TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS)   TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_SESSION_TRACK)                  TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_SESSION_TRACK\ttrue\n");
        if(ClientFlags & MYSQL_CLIENT_DEPRECATE_EOF)                  TagLoger::log(Log_MySqlServer, 0, "MYSQL_CLIENT_DEPRECATE_EOF\ttrue\n");

        if(ClientFlags & MYSQL_CLIENT_PROTOCOL_41)
        {
            memcpy(&PacketMaxSize, p,4);
            p+=4;
            TagLoger::log(Log_MySqlServer, 0, "PacketMaxSize:%d\n", PacketMaxSize);
        }

        ClientCharacterSet = *p;
        p++;
        TagLoger::log(Log_MySqlServer, 0, "ClientCharacterSet:%d\n", ClientCharacterSet);

        if(ClientFlags & MYSQL_CLIENT_PROTOCOL_41)
        {
            // Пропускаем 23 байта в том случаии если пришли обычные заголовки с кучей флагов.
            // Если пришёл заголовок с простой авторизацией только по логину без пароля то пропускать 23 байта не надо.
            // @note не факт что именно MYSQL_CLIENT_PLUGIN_AUTH обеспечивает необходимость пропускать 23 байта но скорее всего он а не один из других флагов или их комбинация
            // string[23]     reserved (all [0])
            p+=23;
        }

        char* name = p;
        p+= strlen(name)+1;
        TagLoger::log(Log_MySqlServer, 0, "UserName:%s\n", name); 
        
        if(ClientFlags & MYSQL_CLIENT_SECURE_CONNECTION)
        {
            char authDataLen = *p;
            TagLoger::log(Log_MySqlServer, 0, "authDataLen:%d\n", authDataLen);
            p++;
            char* DevKeyHashStart = p;

            if(authDataLen == 0)
            {
                TagLoger::log(Log_MySqlServer, 0, "Авторизация без пароля authDataLen=%d\n", authDataLen);

                // Здесь проверка для того можно ли пускать без пароля по имени пользователя
                if(memcmp(name, "haproxy_check", strlen("haproxy_check")) != 0)
                {
                    TagLoger::log(Log_MySqlServer, 0, "Авторизация не прошла, не ожиданный размер authDataLen=%d\n", authDataLen);
                    Send_Err_Package(SQL_ERR_AUTHENTICATION,"Authentication failure, authDataLen!=20", PacketNomber+1, local_buf, this);
                    return -1;
                }
                
                isRootUser = false;

                // Таки решили пустить без пароля
                Send_OK_Package(PacketNomber+1, local_buf, this);
                clientState = STATE_RECEIVED_HANDSHAKE;
                TagLoger::log(Log_MySqlServer, 0, "Соединение не будет закрыто [1] [MySql_connection]\n");
                return 0;
            }
            else if(authDataLen != 20)
            {
                TagLoger::error(Log_MySqlServer, 0, "Авторизация не прошла, не ожиданный размер authDataLen=%d\n", authDataLen);
                Send_Err_Package(SQL_ERR_AUTHENTICATION,"Authentication failure, authDataLen!=20", PacketNomber+1, local_buf, this);
                return -1;
            }
            else
            {
                p+=20;
            }

            if(ClientFlags & MYSQL_CLIENT_CONNECT_WITH_DB) TagLoger::log(Log_MySqlServer, 0, "dbName:%s\n", p);

            if(devInfo::testDevKey(random20bytes, DevKeyHashStart, local_buf))
            {
                TagLoger::log(Log_MySqlServer, 0, "Авторизация успешна ok\n");
                devManager::instance()->getDevInfo()->incrBackendOnline();
                devManager::instance()->getDevInfo()->incrMessages(); 
                isRootUser = true;
            }
            else
            {
                isRootUser = false;
                TagLoger::error(Log_MySqlServer, 0, "Авторизация не прошла");
                Send_Err_Package(SQL_ERR_AUTHENTICATION,"Authentication failure", PacketNomber+1, local_buf, this);
                return -1;
            }
        }
        else
        {
            // string[NULL]    auth-response
            p++;
        }

        if(ClientFlags & MYSQL_CLIENT_CONNECT_WITH_DB)
        {
            char* dbName = p;
            TagLoger::log(Log_MySqlServer, 0, "dbName:%s\n", dbName);
            if(!sql_use_db(dbName))
            {
                Send_Err_Package(SQL_ERR_NOT_EXIST,"Select database failure", PacketNomber+1, local_buf, this);
                return -1;
            }
        }

        Send_OK_Package(PacketNomber+1, local_buf, this);
        clientState = STATE_RECEIVED_HANDSHAKE;
        TagLoger::log(Log_MySqlServer, 0, "Соединение не будет закрыто [2] [MySql_connection]\n");
        return 0;
    }
    else if(clientState == STATE_RECEIVED_HANDSHAKE)
    {
        char com_type = *p;
        TagLoger::log(Log_MySqlServer, 0, "Query: %02x\n", *p);

        if( com_type == MYSQL_PROTOCOL_COM_QUERY)
        {
            if(!isRootUser)
            {
                local_buf->qInfo.setError("Access denied", SQL_ERR_ACCESS_DENIED);
                Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
                return 0;
            }
            
            devManager::instance()->getDevInfo()->incrMessages(); 
            p++;

            char* startQuery = p;
            int queryLen = strlen(startQuery);
            _countUerys++;
            TagLoger::warn(Log_MySqlServer, 0, "QUERY[%d, len=%d][BASE]:%s\n", _countUerys, queryLen, startQuery);

            if(appConf::instance()->useQueryLoger)
            {
                if(
                    memcmp("set ", startQuery, strlen("set "))                                                          != 0 &&
                    memcmp("show ", startQuery, strlen("show "))                                                        != 0 &&
                    memcmp("SET ", startQuery, strlen("SET "))                                                          != 0 &&
                    memcmp("SHOW ", startQuery, strlen("SHOW "))                                                        != 0 &&
                    memcmp("select @@version_comment limit 1", startQuery, strlen("select @@version_comment limit 1"))  != 0
                )
                {
                    // Пишем в лог запросов
                    local_buf->stm.queryLoger.insert(startQuery);
                }
            }

            if( queryLen <= 3)
            {
                TagLoger::error(Log_MySqlServer, 0, "\x1b[1;31mQUERY[%d, len=%d <= 1 ]:%s\x1b[0m\n", _countUerys, queryLen, startQuery);
                Send_Err_Package(SQL_ERR_EMPTY, "Data is too short", PacketNomber+1, local_buf, this);
                return 0;
            }
            else if( queryLen >= MAX_MESSAGE_SIZE)
            {
                TagLoger::error(Log_MySqlServer, 0, "\x1b[1;31mQUERY[%d, len=%d >= MAX_MESSAGE_SIZE ]:%s\x1b[0m\n", _countUerys, queryLen, startQuery);
                Send_Err_Package(SQL_ERR_OVERFLOW, "Data is too long", PacketNomber+1, local_buf, this);
                return 0;
            }

            char tmp[MAX_MESSAGE_SIZE];
            memcpy(tmp, startQuery, MAX_MESSAGE_SIZE);
            if(strReplaceQuote(startQuery))
            {
                TagLoger::warn(Log_MySqlServer, 0, "\x1b[1;33mQUERY[%d, len=%d][REPL]:%s\x1b[0m\n", _countUerys, queryLen, startQuery);
            }

            pthread_mutex_lock(&QLParsing_mutex);
            void* buff = QLParsing(startQuery, &local_buf->qInfo);


            if(local_buf->qInfo.hasError)
            {
                TagLoger::error(Log_MySqlServer, 0, "\x1b[1;31mError query:%s\x1b[0m\n", startQuery);
                QLdeleteBuffer(buff, &local_buf->qInfo);
                pthread_mutex_unlock(&QLParsing_mutex);

                Send_Err_Package(SQL_ERR_SYNTAX_ERROR, "Syntax error in query", PacketNomber+1, local_buf, this);

                return 0;
            }
            pthread_mutex_unlock(&QLParsing_mutex);
            memcpy(startQuery, tmp, MAX_MESSAGE_SIZE);

            if(local_buf->qInfo.command == TOK_SHOW)
            {
                TagLoger::log(Log_MySqlServer, 0, "cmd show:%d\n", local_buf->qInfo.arg_show.command);
                if(local_buf->qInfo.arg_show.command == TOK_DATABASES)
                {
                    sql_show_databases(local_buf,PacketNomber);
                }
                else if(test_api_version(local_buf,PacketNomber))
                {
                    if(local_buf->qInfo.arg_show.command == TOK_TABLES)
                    {
                        sql_show_tables(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.arg_show.command == TOK_COLUMNS)
                    {
                        sql_show_columns(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.arg_show.command == TOK_STATUS)
                    {
                        sql_show_status(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.arg_show.command == TOK_PROCESSLIST)
                    {
                        sql_show_processlist(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.arg_show.command == TOK_VARIABLES)
                    {
                        sql_show_variables(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.arg_show.command == TOK_TABLE_STATUS)
                    {
                        sql_show_table_status(local_buf,PacketNomber);
                    }
                }

                // [Почти готов] SHOW SESSION VARIABLES LIKE 'lower_case_table_names'
                // SELECT current_user()
                // show create table `users`
            }
            else if(local_buf->qInfo.command == TOK_SELECT)
            {
                TagLoger::log(Log_MySqlServer, 0, "cmd select:%d\n", local_buf->qInfo.arg_select.command);

                if(local_buf->qInfo.arg_select.command == TOK_DATABASE)
                {
                    sql_select_database_name(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.arg_select.command == VAL_SYSTEM_VARIBLE)
                {
                    sql_select_systemvarible(local_buf,PacketNomber);
                }
                else if(test_api_version(local_buf,PacketNomber))
                {
                    if(local_buf->qInfo.arg_select.command == TOK_FROM)
                    {
                        if(local_buf->qInfo.tokCompare("users_auth",  local_buf->qInfo.tableName))
                        {
                            sql_select_from_users_auth(local_buf,PacketNomber);
                        }
                        else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
                        {
                            sql_select_from_users_time(local_buf,PacketNomber);
                        }
                        else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
                        {
                            sql_select_from_users_messages(local_buf,PacketNomber);
                        }
                        else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
                        {
                            sql_select_from_pipes_messages(local_buf,PacketNomber);
                        }
                        else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
                        {
                            sql_select_from_users_in_pipes(local_buf,PacketNomber);
                        }
                        else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
                        {
                            sql_select_from_pipes(local_buf,PacketNomber);
                        }
                        else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
                        {
                            sql_select_from_pipes_settings(local_buf,PacketNomber);
                        }
                        else
                        {
                            Send_Err_Package(SQL_ERR_NOT_EXIST, "Table doesn't exist", PacketNomber+1, local_buf, this);
                        }
                    }
                    else
                    {
                        Send_Err_Package(SQL_ERR_SYNTAX_ERROR, "Select query has error", PacketNomber+1, local_buf, this);
                    }
                }
            }
            else if(local_buf->qInfo.command == TOK_INSERT)
            {
                if(test_api_version(local_buf,PacketNomber))
                {
                    if(local_buf->qInfo.tokCompare("users_auth",  local_buf->qInfo.tableName))
                    {
                        sql_insert_into_users_auth(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
                    {
                        sql_insert_into_users_time(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
                    {
                        sql_insert_into_users_messages(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
                    {
                        sql_insert_into_pipes_messages(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
                    {
                        sql_insert_into_users_in_pipes(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
                    {
                        sql_insert_into_pipes(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
                    {
                        sql_insert_into_pipes_settings(local_buf,PacketNomber);
                    } 
                    else
                    {
                        Send_Err_Package(SQL_ERR_NOT_EXIST, "Table doesn't exist", PacketNomber+1, local_buf, this);
                    }
                }
            }
            else if(local_buf->qInfo.command == TOK_DELETE)
            {
                if(test_api_version(local_buf,PacketNomber))
                {
                    if(local_buf->qInfo.tokCompare("users_auth",  local_buf->qInfo.tableName))
                    {
                        sql_delete_from_users_auth(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
                    {
                        sql_delete_from_users_time(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
                    {
                        sql_delete_from_users_messages(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
                    {
                        sql_delete_from_pipes_messages(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
                    {
                        sql_delete_from_users_in_pipes(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
                    {
                        sql_delete_from_pipes(local_buf,PacketNomber);
                    }
                    else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
                    {
                        sql_delete_from_pipes_settings(local_buf,PacketNomber);
                    }
                    else
                    {
                        Send_Err_Package(SQL_ERR_NOT_EXIST, "Table doesn't exist", PacketNomber+1, local_buf, this);
                    }
                }
            }
            else
            {
                TagLoger::log(Log_MySqlServer, 0, "cmd undefined:%d %s\n", local_buf->qInfo.arg_select.command, startQuery);
                Send_OK_Package(PacketNomber+1, local_buf, this);
            }


            QLdeleteBuffer(buff, &local_buf->qInfo);
            return 0;
        }
        else if(com_type == MYSQL_PROTOCOL_COM_FIELD_LIST)
        {
            TagLoger::log(Log_MySqlServer, 0, "COM_FIELD_LIST %d: %s", *p, p+1);
            Send_EOF_Package(PacketNomber+1, local_buf, this);
        }
        else if(com_type == MYSQL_PROTOCOL_COM_PING)
        {
            Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
        }
        else if(com_type == MYSQL_PROTOCOL_COM_QUIT)
        {
            Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
        }
        else if(com_type == MYSQL_PROTOCOL_COM_INIT_DB )
        {
            TagLoger::log(Log_MySqlServer, 0, "COM_INIT_DB %d: %s", *p, p+1);
            if(sql_use_db(p+1))
            {
                Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
            }
            else
            {
                Send_Err_Package(SQL_ERR_NOT_EXIST, "Select database failure", PacketNomber+1, local_buf, this);
            }
            return 0;
        }
        else
        {
            TagLoger::log(Log_MySqlServer, 0, "COM_UNDEFINED %d", *p);
            Send_EOF_Package(PacketNomber+1, local_buf, this); // Send_EOF_Package
        }

        TagLoger::log(Log_MySqlServer, 0, "Соединение не будет закрыто [3] [MySql_connection]\n");
        return 0;
    }

    TagLoger::log(Log_MySqlServer, 0, "Завершение обработки подключения [MySql_connection]\n");
    return -1;
}

/**
 *  Если версия api не выбрана то вернёт false и отправит информацию об ошибке клиенту.
 */
bool MySql_connection::test_api_version(thread_data* local_buf, unsigned int PacketNomber)
{
    TagLoger::log(Log_MySqlServer, 0, "api_version %d", api_version);
    if(api_version == 1)
    {
        return true;
    }

    // Указать нормальный код и текст ошибки.
    Send_Err_Package(SQL_ERR_NOT_EXIST, "Select database failure", PacketNomber+1, local_buf, this);
    return false;
}

bool MySql_connection::sql_use_db(char* db_name)
{
    // Работает но с багами
    if(strlen(db_name) == 10 && memcmp(db_name, "CometQL_v1", 10) == 0)
    {
        TagLoger::log(Log_MySqlServer, 0, "OK set api_version %d", 1);
        api_version = 1;
        return true;
    }

        TagLoger::log(Log_MySqlServer, 0, "Error: set api_version %d", 0);
    api_version = 0;
    return false;
}

/**
 * show databases
 *
 * @param local_buf
 * @param PacketNomber
 * @return
 */
int MySql_connection::sql_show_databases(thread_data* local_buf, unsigned int PacketNomber)
{
    // Отправляем пакет описания 1 колонки
    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();

    local_buf->sql.columns[0] = "Database";
    answer += HeadAnswer(1, &local_buf->sql.columns[0], PacketNomber, answer);

    MySqlResulValue values;
    values = "CometQL_v1";

    answer += RowPackage(1, &values, ++PacketNomber, answer);

    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());

    local_buf->answer_buf.unlock();
    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package
    return 0;
}

int MySql_connection::sql_show_tables(thread_data* local_buf, unsigned int PacketNomber)
{
    /*// Отправляем пакет описания 1 колонки
    const static MySqlResultset_ColumDef column("Tables");

    int countRows = 0;
    local_buf->sql.getValue(countRows++, 0) = "users_auth";
    local_buf->sql.getValue(countRows++, 0) = "users_time";
    local_buf->sql.getValue(countRows++, 0) = "users_messages";
    local_buf->sql.getValue(countRows++, 0) = "pipes_messages";
    local_buf->sql.getValue(countRows++, 0) = "users_in_pipes";
    local_buf->sql.getValue(countRows++, 0) = "pipes";
    local_buf->sql.getValue(countRows++, 0) = "pipes_settings";
    local_buf->sql.sendAllRowsAndHeaders(local_buf, 1, &column, PacketNomber, countRows, this);
    */

    // Отправляем пакет описания 1 колонки
    const static MySqlResultset_ColumDef column("Tables");

    int countRows = 0;
    while(countRows < MYSQL_TABLES_COUNT && tables[countRows].getName() != NULL )
    {
        local_buf->sql.getValue(countRows, 0) = tables[countRows].getName();
        countRows++;
    }

    sendAllRowsAndHeaders(local_buf, 1, &column, PacketNomber, countRows);
    return 0;
}

/**
 * Паказ колонок в таблице
 * @note Только для таблицы users_auth для остальных таблиц не доделано.
 * @param local_buf
 * @param PacketNomber
 * @return
 */
int MySql_connection::sql_show_columns(thread_data* local_buf, unsigned int PacketNomber)
{
    // Field	Type	Null	Key	Default	Extra

    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();

    local_buf->sql.columns[0] = "Field";
    local_buf->sql.columns[1] = "Type";
    local_buf->sql.columns[2] = "Null";
    local_buf->sql.columns[3] = "Key";
    local_buf->sql.columns[4] = "Default";
    local_buf->sql.columns[5] = "Extra";

    answer += HeadAnswer(6, local_buf->sql.columns, PacketNomber, answer);

    if(local_buf->qInfo.tokCompare("users_auth", local_buf->qInfo.tableName))
    {
        MySqlResulValue value[6];
        value[0] = "id";
        value[1] = "int(11)";
        value[2] = "NO";
        value[3] = "PRI";
        value[4] = "";
        value[5] = "";
        answer += RowPackage(6, value, ++PacketNomber, answer);

        value[0] = "hash";
        value[1] = "varchar(32)";
        value[2] = "NO";
        value[3] = "";
        value[4] = "";
        value[5] = "";
        answer += RowPackage(6, value, ++PacketNomber, answer);
    }
    else
    {
        local_buf->answer_buf.unlock();
        return Send_Err_Package(SQL_ERR_NOT_EXIST, "Table doesn't exist", PacketNomber+1, local_buf, this);
    }

    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());

    local_buf->answer_buf.unlock();
    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package
    return 0;
}

int MySql_connection::sql_show_table_status(thread_data* local_buf, unsigned int PacketNomber)
{
    // Отправляем пакет описания 1 колонки
    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();

    MySqlResultset_ColumDef column[18];

    int i = -1;
    column[++i] = "Name";
    column[++i] = "Engine";
    column[++i] = "Version";
    column[++i] = "Row_format";
    column[++i] = "Rows";
    column[++i] = "Avg_row_length";
    column[++i] = "Data_length";
    column[++i] = "Max_data_length";
    column[++i] = "Index_length";
    column[++i] = "Data_free";
    column[++i] = "Auto_increment";
    column[++i] = "Create_time";
    column[++i] = "Update_time";
    column[++i] = "Check_time";
    column[++i] = "Collation";
    column[++i] = "Checksum";
    column[++i] = "Create_options";
    column[++i] = "Comment";

    int delta = HeadAnswer(18, column, PacketNomber, answer);
    answer += delta;

    MySqlResulValue values[18];

    i = -1;
    values[++i] = "users";
    values[++i] = "StarComet";
    values[++i] = "1";
    values[++i] = "Compact";
    values[++i] = "0";               // Rows
    values[++i] = "0";               // Avg_row_length
    values[++i] = "0";               // Data_length
    values[++i] = "0";               // Max_data_length
    values[++i] = "0";               // Index_length
    values[++i] = "0";               // Data_free
    values[++i] = "0";               // Auto_increment
    values[++i] = "0";               // Create_time
    values[++i] = "";                // Update_time
    values[++i] = "";                // Check_time
    values[++i] = "utf8_general_ci"; // Collation
    values[++i] = "";                // Checksum
    values[++i] = "";                // Create_options
    values[++i] = "Таблица с данными авторизации пользователей"; // Comment

    delta = RowPackage(18, values, ++PacketNomber, answer);
    answer += delta;


    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());

    local_buf->answer_buf.unlock();
    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package
    return 0;
}

int MySql_connection::sql_show_processlist(thread_data* local_buf, unsigned int PacketNomber)
{
    if(!isRootUser)
    {
        // Нет доступа не Root пользователям
        local_buf->qInfo.setError("Access denied", SQL_ERR_ACCESS_DENIED);
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();

    local_buf->sql.columns[0] = "ServiceName";
    local_buf->sql.columns[1] = "id";
    local_buf->sql.columns[2] = "status";
    local_buf->sql.columns[3] = "count";

    answer += HeadAnswer(4, local_buf->sql.columns, PacketNomber, answer);

    MySqlResulValue value[4];

    value[0] = tcpServer <MySql_connection>::instance()->bm.getServiceName();
    for(int i =0; i < tcpServer <MySql_connection>::instance()->bm.get_th_num(); i++)
    {
        value[1] = i;
        value[2] = (char)tcpServer <MySql_connection>::instance()->bm.get_th_status(i);
        value[3] = tcpServer <MySql_connection>::instance()->bm.get_th_count(i);
        answer += RowPackage(4, value, ++PacketNomber, answer);
    }

    value[0] = tcpServer <Client_connection>::instance()->bm.getServiceName();
    for(int i =0; i < tcpServer <Client_connection>::instance()->bm.get_th_num(); i++)
    {
        value[1] = i;
        value[2] = (char)tcpServer <Client_connection>::instance()->bm.get_th_status(i);
        value[3] = tcpServer <Client_connection>::instance()->bm.get_th_count(i);
        answer += RowPackage(4, value, ++PacketNomber, answer);
    }


    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());
    local_buf->answer_buf.unlock();

    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package
    return 0;
}

int MySql_connection::sql_show_variables(thread_data* local_buf, unsigned int PacketNomber)
{
    // SHOW SESSION VARIABLES LIKE 'lower_case_table_names'
    // @todo не доделан like и флаги SESSION и GLOBAL

    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();

    local_buf->sql.columns[0] = "Variable_name";
    local_buf->sql.columns[1] = "Value";

    int delta = HeadAnswer(2, local_buf->sql.columns, PacketNomber, answer);
    answer += delta;

    MySqlResulValue value[2];

    if(local_buf->qInfo.arg_show.flag == FLAG_GLOBAL)
    {
        value[0] = "lower_case_table_names";
        value[1] = "0";

        delta = RowPackage(2, value, ++PacketNomber, answer);    answer += delta;
    }
    else if(local_buf->qInfo.arg_show.flag == FLAG_SESSION || local_buf->qInfo.arg_show.flag == 0)
    {
        value[0] = "lower_case_table_names";
        value[1] = "0";

        delta = RowPackage(2, value, ++PacketNomber, answer);    answer += delta;
    }

    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());
    local_buf->answer_buf.unlock();

    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package

    return 0;
}

int MySql_connection::sql_show_status(thread_data* local_buf, unsigned int PacketNomber)
{
    if( !isRootUser && (
            local_buf->qInfo.arg_show.flag == FLAG_GLOBAL ||
            local_buf->qInfo.arg_show.flag == FLAG_FILESYSTEM ||
            local_buf->qInfo.arg_show.flag == FLAG_RAM ||
            local_buf->qInfo.arg_show.flag == FLAG_AVG ))
    {
        // Нет доступа не Root пользователям к флагу GLOBAL
        local_buf->qInfo.setError("Access denied", SQL_ERR_ACCESS_DENIED);
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();

    local_buf->sql.columns[0] = "Variable_name";
    local_buf->sql.columns[1] = "Value";

    int delta = HeadAnswer(2, local_buf->sql.columns, PacketNomber, answer);
    answer += delta;

    MySqlResulValue value[2];
    if(local_buf->qInfo.arg_show.flag == FLAG_FILESYSTEM )
    {
        // Вывод команды df -h | grep "\/$" позволяет оценить количество свободного места
        char Filesystem[20];
        char Size[20];
        char Used[20];
        char Avail[20];
        char Use[20];

        char cli[600];
        if(exec("df -h | grep \"\\/$\"", cli, 600))
        {
            sscanf(cli, "%19s %19s %19s %19s %19s", Filesystem, Size, Used, Avail, Use);
            value[0] = "flag";
            value[1] = "filesystem";
            delta = RowPackage(2, value, ++PacketNomber, answer);
            answer += delta;

            value[0] = "filesystem";
            value[1] = Filesystem;
            delta = RowPackage(2, value, ++PacketNomber, answer);
            answer += delta;

            value[0] = "size";
            value[1] = Size;
            delta = RowPackage(2, value, ++PacketNomber, answer);
            answer += delta;

            value[0] = "used";
            value[1] = Used;
            delta = RowPackage(2, value, ++PacketNomber, answer);
            answer += delta;

            value[0] = "available";
            value[1] = Avail;
            delta = RowPackage(2, value, ++PacketNomber, answer);
            answer += delta;

            value[0] = "use";
            value[1] = Use;
            delta = RowPackage(2, value, ++PacketNomber, answer);
            answer += delta;
        }
        else
        {
            local_buf->answer_buf.unlock();
            // Не удалось выполнить команду
            local_buf->qInfo.setError("exec command failed", SQL_ERR_INTERNAL_SERVER);
            Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
            return 0;
        }
    }
    else if(local_buf->qInfo.arg_show.flag == FLAG_RAM )
    {
        FILE* loadavgFp = fopen("/proc/meminfo", "r");

        char name[200];
        char val[200];
        char tmpNull[200];
        int len;
        int i = 0;
        while(fscanf(loadavgFp, "%s %s %s", name, val, tmpNull) != EOF && i < 34)
        {
            i++;
            len = strlen(name);
            if(len > 0)
            {
                name[len - 1] = 0;
            }
            value[0] = name;
            value[1] = val;
            delta = RowPackage(2, value, ++PacketNomber, answer);
            answer += delta;
        }
        fclose(loadavgFp);
    }
    else if(local_buf->qInfo.arg_show.flag == FLAG_AVG )
    {
        FILE* loadavgFp = fopen("/proc/loadavg", "r");
        float loadavg_1, loadavg_2, loadavg_3;
        int running_processes, total_processes;

        fscanf(loadavgFp, "%5f %5f %5f %5d/%5d", &loadavg_1, &loadavg_2, &loadavg_3, &running_processes, &total_processes);
        fclose(loadavgFp);

        value[0] = "load_avg_1";
        snprintf(value[1].clear(), 255, "%.2f", loadavg_1);
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "load_avg_2";
        snprintf(value[1].clear(), 255, "%.2f", loadavg_2);
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "load_avg_3";
        snprintf(value[1].clear(), 255, "%.2f", loadavg_3);
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "running_processes";
        value[1] = running_processes;
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "total_processes";
        value[1] = total_processes;
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;
    }
    else if(local_buf->qInfo.arg_show.flag == FLAG_UPTIME )
    {
        // http://linuxinsight.com/proc_uptime.html
        float os_uptime, os_idle;
        FILE* uptimeFp = fopen("/proc/uptime", "r");
        fscanf(uptimeFp, "%f %f", &os_uptime, &os_idle);
        fclose(uptimeFp);

        value[0] = "os_uptime";
        value[1] = os_uptime;
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "os_idle";
        value[1] = os_idle;
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;
    }
    else if(local_buf->qInfo.arg_show.flag == FLAG_GLOBAL )
    {
        value[0] = "flag";
        value[1] = "global";

        delta = RowPackage(2, value, ++PacketNomber, answer);    answer += delta;

        value[0] = "uptime";
        value[1] = (int)tcpServer <MySql_connection>::instance()->bm.get_uptime();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "uptime_in_hours";
        snprintf(value[1].clear(), 255, "%.2f", (float)tcpServer <MySql_connection>::instance()->bm.get_uptime()/3600);
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "uptime_in_days";
        snprintf(value[1].clear(), 255, "%.2f", (float)tcpServer <MySql_connection>::instance()->bm.get_uptime()/(3600*24));
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        // Service_name
        value[0] = "frontend_name";
        value[1] = tcpServer <Client_connection>::instance()->bm.getServiceName();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "backend_name";
        value[1] = tcpServer <MySql_connection>::instance()->bm.getServiceName();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        // stat_interval
        value[0] = "frontend_stat_interval";
        value[1] = tcpServer <Client_connection>::instance()->bm.stat_interval;
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "backend_stat_interval";
        value[1] = tcpServer <MySql_connection>::instance()->bm.stat_interval;
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        // online
        value[0] = "frontend_online";
        value[1] = tcpServer <Client_connection>::instance()->bm.getConections();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "backend_online";
        value[1] = tcpServer <MySql_connection>::instance()->bm.getConections();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        // messages
        value[0] = "frontend_messages";
        value[1] = tcpServer <Client_connection>::instance()->bm.getHandleMessage();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "backend_messages";
        value[1] = tcpServer <MySql_connection>::instance()->bm.getHandleMessage();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        // add_client
        value[0] = "frontend_add_client";
        value[1] = tcpServer <Client_connection>::instance()->bm.getAddClient();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "backend_add_client";
        value[1] = tcpServer <MySql_connection>::instance()->bm.getAddClient();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        // delete_client
        value[0] = "frontend_delete_client";
        value[1] = tcpServer <Client_connection>::instance()->bm.getDeleteClient();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "backend_delete_client";
        value[1] = tcpServer <MySql_connection>::instance()->bm.getDeleteClient();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        // messages_ps
        value[0] = "frontend_messages_ps";
        snprintf(value[1].clear(), 255, "%.2f", (float)tcpServer <Client_connection>::instance()->bm.getPsHandleMessage());
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "backend_messages_ps";
        snprintf(value[1].clear(), 255, "%.2f", (float)tcpServer <MySql_connection>::instance()->bm.getPsHandleMessage());
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        // addClient_ps
        value[0] = "frontend_add_client_ps";
        snprintf(value[1].clear(), 255, "%.2f", (float)tcpServer <Client_connection>::instance()->bm.getPsAddClient());
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "backend_add_client_ps";
        snprintf(value[1].clear(), 255, "%.2f", (float)tcpServer <MySql_connection>::instance()->bm.getPsAddClient());
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        // deleteClient_ps
        value[0] = "frontend_delete_client_ps";
        snprintf(value[1].clear(), 255, "%.2f", (float)tcpServer <Client_connection>::instance()->bm.getPsDeleteClient());
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "backend_delete_client_ps";
        snprintf(value[1].clear(), 255, "%.2f", (float)tcpServer <MySql_connection>::instance()->bm.getPsDeleteClient());
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;       
        
        value[0] = "network_events"; // Сетевые события только от авторизованных пользователей
        value[1] = devManager::instance()->getPsNetworkEvents();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;  
    }
    else if(local_buf->qInfo.arg_show.flag == FLAG_SESSION || local_buf->qInfo.arg_show.flag == 0)
    {
        value[0] = "flag";
        value[1] = "session";
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "user_online";
        value[1] = devManager::instance()->getDevInfo()->getFrontendOnline();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "server_online";
        value[1] = devManager::instance()->getDevInfo()->getBackendOnline();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "messages";
        value[1] = devManager::instance()->getDevInfo()->getMessages();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;
    }
    else
    {
        // не известный флаг, ошибка в граматике
        value[0] = "flag";
        value[1] = "Не известный флаг, ошибка в граматике";

        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;
    }

    /**
     * Для выявления SYN Flood достаточно посмотреть очередь полуоткрытых соединений командой
     * netstat -n -t | grep SYN_RECV | wc -l
     *
     * UDP Flood
     * vnstat -l -i venet0
     * Если значение rx большое – вероятнее всего, на сервер идет DDoS атака.
     *
     * http://rusua.org.ua/2013/01/07/411/
     *
     *
     * https://beget.com/ru/articles/syncookied
     * https://github.com/LTD-Beget/syncookied
     *
     * df -l Проверять наличие свободного места на сервере.
     */


    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());
    local_buf->answer_buf.unlock();

    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package
    return 0;
}

int MySql_connection::sql_select_systemvarible(thread_data* local_buf, unsigned int PacketNomber)
{
    // Отправляем пакет описания 1 колонки
    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();

    local_buf->sql.columns[0].setName(local_buf->qInfo.tokStart(local_buf->qInfo.arg_select.systemVaribleName), local_buf->qInfo.arg_select.systemVaribleName.tokLen);
    answer += HeadAnswer(1, local_buf->sql.columns, PacketNomber, answer);
    //printf("systemVaribleName %s\n", local_buf->qInfo.tokStart(local_buf->qInfo.arg_select.systemVaribleName));
    MySqlResulValue values;
    values = "StarComet (comet-server.ru, support@comet-server.ru)";

    answer += RowPackage(1, &values, ++PacketNomber, answer);

    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());

    local_buf->answer_buf.unlock();
    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package

    return 0;
}

int MySql_connection::sql_select_database_name(thread_data* local_buf, unsigned int PacketNomber)
{
    // Отправляем пакет описания 1 колонки
    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();

    local_buf->sql.columns[0] = "DATABASE()";
    answer += HeadAnswer(1, local_buf->sql.columns, PacketNomber, answer);

    MySqlResulValue values;

    if(api_version != 0)
    {
        values = "CometQL_v0";
        char* name = values;
        name[9] = '0' + api_version;
    }
    else
    {
        values = "";
    }
    answer += RowPackage(1, &values, ++PacketNomber, answer);

    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());

    local_buf->answer_buf.unlock();
    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package

    return 0;
}


// users_auth
int MySql_connection::sql_select_from_users_auth(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "id",
        "hash"
    };

    if(!local_buf->sql.prepare_columns_for_select(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }


    char hash[USER_HASH_LEN+1];
    bzero(hash, USER_HASH_LEN+1);

    int countRows = 0;
    for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
    {
        if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
        {
            // Значения закончились
            break;
        }

        int userId = local_buf->qInfo.where.whereExprValue[idExprPos][i].ToInt(local_buf->qInfo);
        if(userId <= 0)
        {
            continue;
        }

        if(!devManager::instance()->getDevInfo()->index->get_hash(local_buf, userId, hash))
        {
            continue;
        }

        if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = userId;
        if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1).setValue(hash, USER_HASH_LEN);

        countRows++;
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_insert_into_users_auth(thread_data* local_buf, unsigned int PacketNomber)
{
    TagLoger::log(Log_MySqlServer, 0, " >MySql_connection::add_user_hash\n");

    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "id",
        "hash"
    };

    if(!local_buf->sql.prepare_columns_for_insert(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[0] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `id` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[1] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `hash` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    int user_id = local_buf->qInfo.tokToInt(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]]);
    if(user_id <= 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "The id field must be non-negative", PacketNomber+1, local_buf, this);
        return 0;
    }

    char* hash = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]]);
    hash[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]].tokLen] = 0;

    if(!validation_string(hash, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]].tokLen))
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "Сontains any invalid characters in the string hash", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]].tokLen > USER_HASH_LEN)
    {
        Send_Err_Package(SQL_ERR_OVERFLOW, "Hash string is too long", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(!devManager::instance()->getDevInfo()->index->add_hash(local_buf, user_id, hash))
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "Error in hash or user id", PacketNomber+1, local_buf, this);
        return 0;
    }

    /**
     * Данные доступны в режиме кластера из за необходимости на каждой ноде всех ключей авторизации хотя и это утверждение не факт что верное
     * Но даже если и нет то можно их легко посчитать
     */
    Send_OK_Package(1, user_id, PacketNomber+1, local_buf, this);

    return 0;
}

int MySql_connection::sql_delete_from_users_auth(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "id",
        "hash"
    };

    if(!local_buf->sql.prepare_where_expressions(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }

    int countRows = 0;
    for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
    {
        if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
        {
            // Значения закончились
            break;
        }

        int userId = local_buf->qInfo.where.whereExprValue[idExprPos][i].ToInt(local_buf->qInfo);
        if(userId <= 0 )
        {
            continue;
        }

        if(!devManager::instance()->getDevInfo()->index->delete_hash(local_buf, userId))
        {
            continue;
        }

        int *conection_id = devManager::instance()->getDevInfo()->index->get_conection_id(local_buf, userId);
        if(conection_id != NULL)
        {
            for(int j=0; j< MAX_CONECTION_ON_USER_ID; j++)
            {
                if( conection_id[j] == 0 )
                {
                    continue;
                }

                CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(conection_id[j]);
                if(r)
                {
                    tcpServer <Client_connection>::instance()->deleteClient(r->getfd(), local_buf);
                }
                else
                {
                    TagLoger::log(Log_MySqlServer, 0, "Не найден объект соединения\n");
                }
            }
        }
        else
        {
            TagLoger::log(Log_MySqlServer, 0, "Не найден идентификатор соединения\n");
        }


        countRows++;
    }

    /**
     * Для операций удаления affectedRows возвращатся не будет в целях оптимизации.
     */
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
    return 0;
}





// users_time
int MySql_connection::sql_select_from_users_time(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "id",
        "time"
    };

    if(!local_buf->sql.prepare_columns_for_select(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }

    int countRows = 0;
    for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
    {
        if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
        {
            // Значения закончились
            break;
        }

        int userId = local_buf->qInfo.where.whereExprValue[idExprPos][i].ToInt(local_buf->qInfo);
        if(userId < 0 )
        {
            continue;
        }

        long time = devManager::instance()->getDevInfo()->index->get_last_online_time(local_buf, userId);

        if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = userId;
        if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = time;

        countRows++;
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_insert_into_users_time(thread_data* local_buf, unsigned int PacketNomber)
{
    Send_Err_Package(SQL_ERR_READ_ONLY, "Table `users_time` is read only", PacketNomber+1, local_buf, this);
    return 0;
}

int MySql_connection::sql_delete_from_users_time(thread_data* local_buf, unsigned int PacketNomber)
{
    Send_Err_Package(SQL_ERR_READ_ONLY, "Table `users_time` is read only", PacketNomber+1, local_buf, this);
    return 0;
}




// users_messages
int MySql_connection::sql_select_from_users_messages(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "id",
        "index",
        "event",
        "message"
    };

    if(!local_buf->sql.prepare_columns_for_select(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }

    int countRows = 0;
    for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
    {
        if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
        {
            // Значения закончились
            break;
        }

        int userId = local_buf->qInfo.where.whereExprValue[idExprPos][i].ToInt(local_buf->qInfo);
        if(userId < 0 )
        {
            continue;
        }


        local_buf->stm.users_queue_select.execute(userId, 10);
        while(!local_buf->stm.users_queue_select.fetch())
        {
            if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = userId;
            if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = local_buf->stm.users_queue_select.result_id;
            if(local_buf->sql.useColumn(2)) local_buf->sql.getValue(countRows, 2) = local_buf->stm.users_queue_select.result_event;
            if(local_buf->sql.useColumn(3)) local_buf->sql.getValue(countRows, 3) = local_buf->stm.users_queue_select.result_message;

            countRows++;
        }
        local_buf->stm.users_queue_select.free();
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_insert_into_users_messages(thread_data* local_buf, unsigned int PacketNomber)
{
    TagLoger::log(Log_MySqlServer, 0, " >MySQL_connection::insert_from_users_messages\n");

    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "id",
        "index", // Игнорируется так как является read only полем в таблице, но допустим в перечислении полей.
        "event",
        "message"
    };

    if(!local_buf->sql.prepare_columns_for_insert(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[0] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `id` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[2] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `event` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[3] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `message` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    int user_id = local_buf->qInfo.tokToInt(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]]);
    if(user_id < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "The id field must be non-negative", PacketNomber+1, local_buf, this);
        return 0;
    }

    char* message = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]]);
    message[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]].tokLen] = 0;
    char messageQuote = local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]].quote;

    local_buf->answer_buf.lock();
    //json_escape_string(message, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]].tokLen, local_buf->answer_buf.getData());
    mysql_real_escape_string(local_buf->db.getLink(), local_buf->answer_buf.getData(), message, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]].tokLen);

    char* pipe_event = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]]);
    pipe_event[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]].tokLen] = 0;

    if(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]].tokLen > EVENT_NAME_LEN)
    {
        Send_Err_Package(SQL_ERR_OVERFLOW, "Channel event_name is too long", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(!AZ09test(pipe_event, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]].tokLen))
    {
        // @FixME добавить ссылки на документацию в сообщения об ошибках
        Send_Err_Package(SQL_ERR_INVALID_DATA, "The channel name can contain only the characters A-Za-z0-9_-", PacketNomber+1, local_buf, this);
        return 0;
    }

    internalApi::send_to_user(local_buf, user_id, pipe_event, local_buf->answer_buf.getData());
    local_buf->answer_buf.unlock();
    /**
     * Если affectedRows == 0 то сообщение не доставлено и не может быть помещено в очередь для доставки позже из за переполненения очереди сообщений.
     * Если affectedRows == 1 то сообщение либо доставлено сразу либо помещено в очередь для доставки потом.
     *
     * Скорее всего эти данные доступны в режиме кластера, но ещё не факт.
     */
    Send_OK_Package(0, user_id, PacketNomber+1, local_buf, this);
    return 0;
}

int MySql_connection::sql_delete_from_users_messages(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "id",
        "index",
        "event",
        "message"
    };

    if(!local_buf->sql.prepare_where_expressions(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }

    int countRows = 0;
    for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
    {
        if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
        {
            // Значения закончились
            break;
        }

        int user_id = local_buf->qInfo.where.whereExprValue[idExprPos][i].ToInt(local_buf->qInfo);
        if(user_id < 0)
        {
            continue;
        }

        local_buf->stm.users_queue_delete.execute((long int)time(NULL), user_id);
        countRows++;
    }

    /**
     * Для операций удаления affectedRows возвращатся не будет в целях оптимизации.
     */
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
    return 0;
}


// pipes_messages
int MySql_connection::sql_select_from_pipes_messages(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",
        "index",
        "event",
        "message",
        "user_id"
    };

    if(!local_buf->sql.prepare_columns_for_select(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }

    int countRows = 0;
    for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
    {
        if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
        {
            // Значения закончились
            break;
        }

        char* pipe_name = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo);
        int nameLen = local_buf->qInfo.where.whereExprValue[idExprPos][i].tokLen;
        pipe_name[nameLen] = 0;
        if(nameLen < 3 || nameLen > PIPE_NAME_LEN || !AZ09test(pipe_name, nameLen))
        {
            continue;
        }

        local_buf->stm.pipe_messages_select.execute(pipe_name, 99);
        while(!local_buf->stm.pipe_messages_select.fetch())
        {
            if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = pipe_name;
            if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = countRows;
            if(local_buf->sql.useColumn(2)) local_buf->sql.getValue(countRows, 2) = local_buf->stm.pipe_messages_select.result_event;
            if(local_buf->sql.useColumn(3)) local_buf->sql.getValue(countRows, 3) = local_buf->stm.pipe_messages_select.result_message;
            if(local_buf->sql.useColumn(4)) local_buf->sql.getValue(countRows, 4) = (long)local_buf->stm.pipe_messages_select.result_user_id;

            countRows++;
        }
        local_buf->stm.pipe_messages_select.free();
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_insert_into_pipes_messages(thread_data* local_buf, unsigned int PacketNomber)
{
    TagLoger::log(Log_MySqlServer, 0, " >MySql_connection::insert_into_pipes_messages\n");

    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",
        "index",
        "event",
        "message"
    };

    if(!local_buf->sql.prepare_columns_for_insert(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[0] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `name` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[2] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `event` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[3] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `message` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    char* pipe_name = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]]);
    pipe_name[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]].tokLen] = 0;
    if(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]].tokLen > PIPE_NAME_LEN)
    {
        Send_Err_Package(SQL_ERR_OVERFLOW, "Channel name is too long", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]].tokLen < 3)
    {
        Send_Err_Package(SQL_ERR_EMPTY, "Channel name is too short", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(!AZ09test(pipe_name, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]].tokLen))
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "The channel name can contain only the characters A-Za-z0-9_-", PacketNomber+1, local_buf, this);
        return 0;
    }

    char* pipe_event = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]]);
    pipe_event[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]].tokLen] = 0;

    if(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]].tokLen > EVENT_NAME_LEN)
    {
        Send_Err_Package(SQL_ERR_OVERFLOW, "Channel event_name is too long", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(!AZ09test(pipe_event, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]].tokLen))
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "The channel name can contain only the characters A-Za-z0-9_-", PacketNomber+1, local_buf, this);
        return 0;
    }

    char* message = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]]);
    message[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]].tokLen] = 0;

    local_buf->answer_buf.lock();
    //json_escape_string(message, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]].tokLen, local_buf->answer_buf.getData());
    mysql_real_escape_string(local_buf->db.getLink(), local_buf->answer_buf.getData(), message, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]].tokLen);


    char serverData[EVENT_NAME_LEN+64];
    snprintf(serverData, EVENT_NAME_LEN+64, "\"event_name\":\"%s\"", pipe_event);

    TagLoger::log(Log_MySqlServer, 0, "message:%s\n", local_buf->answer_buf.getData());

    PipeLog::addToLog(local_buf, pipe_name, pipe_event, 0, 0, message, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]].tokLen);
    internalApi::send_event_to_pipe(local_buf, pipe_name, local_buf->answer_buf.getData(), serverData);

    local_buf->answer_buf.unlock();

    /**
     * Точные данные по affectedRows не доступны в режиме кластера
     * Если очень надо то количество людей в канале можно узнать отдельным запросом
     */
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
    return 0;
}

int MySql_connection::sql_delete_from_pipes_messages(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",
        "index",
        "event",
        "message",
        "auth_type" // По умолчанию как * тоесть сообщение для всех
                    // Если authType == 1 то не отправляем сообщение не авторизованным пользователям.
                    // Если authType == 0 то не отправляем сообщение авторизованным пользователям.
    };

    if(!local_buf->sql.prepare_where_expressions(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }

    int countRows = 0;
    for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
    {
        if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
        {
            // Значения закончились
            break;
        }

        char* pipe_name = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo);
        int nameLen = local_buf->qInfo.where.whereExprValue[idExprPos][i].tokLen;
        pipe_name[nameLen] = 0;
        if(nameLen < 3 || nameLen > PIPE_NAME_LEN || !AZ09test(pipe_name, nameLen))
        {
            continue;
        }

        local_buf->stm.pipe_messages_delete.execute(0x7FFFFFFF-1, pipe_name); // максимальное значение unixtime
        countRows++;
    }

    /**
     * Для операций удаления affectedRows возвращатся не будет в целях оптимизации.
     */
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
    return 0;
}

// users_in_pipes
int MySql_connection::sql_select_from_users_in_pipes(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",
        "user_id",
        "user_uuid",
        "user_agent",
        "ip",
        "origin",
        "language"
    };

    if(!local_buf->sql.prepare_columns_for_select(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }

    int countRows = 0;
    for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
    {
        if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
        {
            // Значения закончились
            break;
        }
        // Проверить на пустой строке
        char* pipe_name = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo);
        int nameLen = local_buf->qInfo.where.whereExprValue[idExprPos][i].tokLen;
        pipe_name[nameLen] = 0;
        if(nameLen < 3 || nameLen > PIPE_NAME_LEN || !AZ09test(pipe_name, nameLen))
        {
            continue;
        }

        TagLoger::log(Log_MySqlServer, 0, "text>%s\n",pipe_name);



        CP<Pipe> pipe = devManager::instance()->getDevInfo()->findPipe(std::string(pipe_name));

        if(pipe.isNULL())
        {
            // internal server error
            TagLoger::log(Log_MySqlServer, 0, "pipe[%s] == NULL\n",pipe_name);
            continue;
        }
        else if(pipe->empty() )
        {
            // internal server error
            TagLoger::log(Log_MySqlServer, 0, "pipe[%s] == empty\n",pipe_name);
            continue;
        }

        auto it = pipe->subscribers.begin();
        while(it)
        {
            int val =  it->data;
            TagLoger::log(Log_MySqlServer, 0, "pipe[%s] client_id - %d\n",pipe_name, val);

            CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(val);
            if(r)
            {
                if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = pipe_name;
                if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = r->web_user_id;
                if(local_buf->sql.useColumn(2)) local_buf->sql.getValue(countRows, 2) = r->web_user_uuid;
                if(local_buf->sql.useColumn(3)) local_buf->sql.getValue(countRows, 3) = r->web_user_agent;
                if(local_buf->sql.useColumn(4)) local_buf->sql.getValue(countRows, 4) = r->getIp();
                if(local_buf->sql.useColumn(5)) local_buf->sql.getValue(countRows, 5) = r->web_user_host;
                if(local_buf->sql.useColumn(6)) local_buf->sql.getValue(countRows, 6) = r->web_user_language;

                countRows++;
            }

            it = it->Next();
        }
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_insert_into_users_in_pipes(thread_data* local_buf, unsigned int PacketNomber)
{
    Send_Err_Package(SQL_ERR_READ_ONLY, "Table `users_in_pipes` is read only", PacketNomber+1, local_buf, this);
    return 0;
}

int MySql_connection::sql_delete_from_users_in_pipes(thread_data* local_buf, unsigned int PacketNomber)
{
    Send_Err_Package(SQL_ERR_READ_ONLY, "Table `users_in_pipes` is read only", PacketNomber+1, local_buf, this);
    return 0;
}


// users_in_pipes
int MySql_connection::sql_select_from_pipes(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",
        "users"
    };

    if(!local_buf->sql.prepare_columns_for_select(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }

    int countRows = 0;
    for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
    {
        if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
        {
            // Значения закончились
            break;
        }

        char* pipe_name = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo);
        int nameLen = local_buf->qInfo.where.whereExprValue[idExprPos][i].tokLen;
        pipe_name[nameLen] = 0;
        if(nameLen < 3 || nameLen > PIPE_NAME_LEN || !AZ09test(pipe_name, nameLen))
        {
            continue;
        }


        CP<Pipe> pipe = devManager::instance()->getDevInfo()->findPipe(std::string(pipe_name));

        TagLoger::log(Log_MySqlServer, 0, "text>%s\n",pipe_name);
        if( !pipe.isNULL() && pipe->empty() )
        {
            if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = pipe_name;
            if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = pipe->size();
        }
        else
        {
            continue;
        }

        countRows++;
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_insert_into_pipes(thread_data* local_buf, unsigned int PacketNomber)
{
    Send_Err_Package(SQL_ERR_READ_ONLY, "Table `pipes` is read only", PacketNomber+1, local_buf, this);
    return 0;
}

int MySql_connection::sql_delete_from_pipes(thread_data* local_buf, unsigned int PacketNomber)
{
    Send_Err_Package(SQL_ERR_READ_ONLY, "Table `pipes` is read only", PacketNomber+1, local_buf, this);
    return 0;
}



// pipes_settings
int MySql_connection::sql_select_from_pipes_settings(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",
        "length"
    };

    if(!local_buf->sql.prepare_columns_for_select(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }

    int countRows = 0;
    for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
    {
        if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
        {
            // Значения закончились
            break;
        }

        char* pipe_name = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo);
        int nameLen = local_buf->qInfo.where.whereExprValue[idExprPos][i].tokLen;
        pipe_name[nameLen] = 0;
        if(nameLen < 3 || nameLen > PIPE_NAME_LEN || !AZ09test(pipe_name, nameLen))
        {
            continue;
        }

        TagLoger::log(Log_MySqlServer, 0, "text>%s\n",pipe_name);

        local_buf->stm.pipes_settings_select.execute(pipe_name);
        if(local_buf->stm.pipes_settings_select.fetch())
        {
            local_buf->stm.pipes_settings_select.free();
            continue;
        }

        int result_length = local_buf->stm.pipes_settings_select.result_length;
        local_buf->stm.pipes_settings_select.free();

        if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = pipe_name;
        if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = result_length;
        countRows++;
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_insert_into_pipes_settings(thread_data* local_buf, unsigned int PacketNomber)
{
    TagLoger::log(Log_MySqlServer, 0, " >MySql_connection::insert_into_pipes_messages\n");

    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",
        "length"
    };

    if(!local_buf->sql.prepare_columns_for_insert(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[0] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `name` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    char* pipe_name = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]]);
    pipe_name[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]].tokLen] = 0;
    if(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]].tokLen > PIPE_NAME_LEN)
    {
        Send_Err_Package(SQL_ERR_OVERFLOW, "Channel name is too long", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]].tokLen < 3)
    {
        Send_Err_Package(SQL_ERR_EMPTY, "Channel name is too short", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(!AZ09test(pipe_name, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]].tokLen))
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "The channel name can contain only the characters A-Za-z0-9_-", PacketNomber+1, local_buf, this);
        return 0;
    }

    PipeSettings pipe_settings(pipe_name);

    if(local_buf->sql.columPositions[1] >= 0)
    {
        pipe_settings.setLogLength(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]].ToInt(local_buf->qInfo));
    }

    if( pipe_settings.save(local_buf) < 0)
    {
        Send_Err_Package(SQL_ERR_INTERNAL_SERVER, "internal server error: sql_insert_into_pipes_settings", PacketNomber+1, local_buf, this);
        return 0;
    }

    Send_OK_Package(1, 0, PacketNomber+1, local_buf, this);
    return 0;
}

int MySql_connection::sql_delete_from_pipes_settings(thread_data* local_buf, unsigned int PacketNomber)
{

    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",
        "length"
    };

    if(!local_buf->sql.prepare_where_expressions(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }

    int countRows = 0;
    for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
    {
        if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
        {
            // Значения закончились
            break;
        }

        char* pipe_name = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo);
        int nameLen = local_buf->qInfo.where.whereExprValue[idExprPos][i].tokLen;
        pipe_name[nameLen] = 0;
        if(nameLen < 3 || nameLen > PIPE_NAME_LEN || !AZ09test(pipe_name, nameLen))
        {
            continue;
        }

        local_buf->db.query_format("DELETE FROM `pipes_settings` WHERE `name` = '%s' ;", pipe_name);
        local_buf->stm.pipe_messages_delete.execute(0x7FFFFFFF-1, pipe_name); // максимальное значение unixtime

        countRows++;
    }

    /**
     * Для операций удаления affectedRows возвращатся не будет в целях оптимизации.
     */
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
    return 0;
}
 
int MySql_connection::set_online(thread_data* local_buf)
{
    if(isOnLine)
    {
        return 0;
    }

    TagLoger::log(Log_MySqlServer, 0, "MySQL itit start \n");
    isOnLine = true;
    start_online_time = time(0);

    local_buf->answer_buf.lock();
    char* p = local_buf->answer_buf.getData();

    // первые 3 байта — длина пакета
    p+=3;

    // четвертый байт — номер пакета.
    p+=1;

    // protocol_version
    *p = 0x0a;
    p+=1;

    // server_version
    memcpy(p, MYSQL_SERVERNAME, strlen(MYSQL_SERVERNAME));
    p+=strlen(MYSQL_SERVERNAME)+1;

    // connection_id
    memcpy(p, &fd, 4);
    p+=4;

    // auth_plugin_data_part_1 (string.fix_len) -- [len=8] first 8 bytes of the auth-plugin data
    for(int i=0; i< 20; i+=1)
    {
        random20bytes[i] = (unsigned char)(rand() % 254);
    }
    memcpy(p,random20bytes,8);
    p+=8;

    // filler_1
    p+=1;

    // capability_flag_1 (флаги первые 2 байта)
    int capability_flag = 0xc00fffff; // MYSQL_CLIENT_CONNECT_WITH_DB | MYSQL_CLIENT_PROTOCOL_41 | MYSQL_CLIENT_LONG_PASSWORD | MYSQL_CLIENT_SECURE_CONNECTION | MYSQL_CLIENT_PLUGIN_AUTH; // 0xF7FF; // MYSQL_CLIENT_DEPRECATE_EOF
    memcpy(p, &capability_flag, 2);
    p+=2;

    // character_set https://dev.mysql.com/doc/internals/en/character-set.html#packet-Protocol::CharacterSet
    *p = 0x08; // 0x21;
    p++;

    // status_flags
    *p = 0x02;  // auto-commit is enabled
    p+=2;

    // capability_flags_2 (флаги вторые 2 байта)
    //memcpy(p, (&capability_flag)+2, 2);
    //p+=2;
    *p = 0x0f;
    p++;
    *p = 0xc0;
    p++;

    // auth_plugin_data_len
    *p = 0x15;
    p+=1;

    // string[10]     reserved (all [00])
    p+=10;

    // string[NUL]    auth-plugin-data-part-2 (12 байт, продолжение строки со случайными данными для авторизации)
    memcpy(p,random20bytes+8,12);
    p+=12+1;

    // auth_plugin_name mysql_clear_password | mysql_native_password
    memcpy(p,"mysql_native_password", strlen("mysql_native_password"));
    p+=strlen("mysql_native_password") + 1;

    int dataLen = p - local_buf->answer_buf.getData();
    TagLoger::log(Log_MySqlServer, 0, "itit len:%d\n", dataLen);

    int t = dataLen - 4;
    memcpy(local_buf->answer_buf.getData(),  &t, 3); //  Длина пакета

    if(TagLoger::isLog(Log_MySqlServer, TAGLOG_DEBUG))
    {
        for(int i=0; i < dataLen; i++)
        {
            if(i % 8 == 0)  TagLoger::log(Log_MySqlServer, 0, "!\n%d:\t", i);
            char c = (char)local_buf->answer_buf[i];
            if(c < 32 ) c = '.';

            TagLoger::log(Log_MySqlServer, 0, "!%02x[%c] ",  (unsigned char)local_buf->answer_buf[i], c);
        }
        TagLoger::log(Log_MySqlServer, 0, "!\n ");
    }

    clientState = STATE_SEND_HANDSHAKE;
    TagLoger::log(Log_MySqlServer, 0, "Соединение не будет закрыто [4] [MySql_connection]\n");
    return web_write(local_buf->answer_buf.getAndUnlock(), dataLen);
}

int MySql_connection::set_offline(thread_data* local_buf)
{
    if(!isOnLine)
    {
        return 0;
    }

    TagLoger::log(Log_MySqlServer, 0, " >MySql_connection::set_online false\n");
    devManager::instance()->getDevInfo()->decrBackendOnline();
    api_version = 0;
    start_online_time = 0;
 
    isOnLine = false;
    return web_close();
}

int MySql_connection::sendAllRowsAndHeaders(thread_data* local_buf, int columCount, const MySqlResultset_ColumDef* columDef, unsigned int PacketNomber, int countRows)
{
    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();
    answer += HeadAnswer(columCount, columDef, PacketNomber, answer);

    int realSendPackage = 0;
    int maxSize = local_buf->answer_buf.getSize() - (answer - local_buf->answer_buf.getData()); // Объём свободной памяти в буфере.
    answer += local_buf->sql.rowsToBuff(PacketNomber, answer, countRows, local_buf->qInfo, maxSize, realSendPackage);
    PacketNomber += realSendPackage;

    int dataSize = answer - local_buf->answer_buf.getData();
    web_write(local_buf->answer_buf.getData(), dataSize);

    TagLoger::log(Log_MySqlServer, 0, "dataSize=%d, realSendRows=%d, countRows=%d", dataSize, realSendPackage, countRows);
    local_buf->answer_buf.unlock();
    return Send_EOF_Package(++PacketNomber, local_buf, this);
}

void MySql_connection::addIntervalRoutine()
{
    intervalLoop::instance()->add([](int uptime, thread_data* local_buf)
    {
        if(uptime % 120 != 0)
        {
            // выполнять один раз в 120 секунд
            return;
        }

        // Удалить все недоставленые сообщения старше 120 секунд
        local_buf->db.query_format("DELETE FROM `users_messages` where time < %d", time(0) - 120);


        // Удалить все устаревшие сообщения старше 120 секунд
        local_buf->db.query_format("DELETE FROM `pipe_messages` where time < %d and pipe_name like 'trust_%%' ", time(0) - 120);
    });

}