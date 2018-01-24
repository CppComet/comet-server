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
#include "CometQLProxy_connection.h"
#include "Freeswitch_connection.h"


#include "CometQL.h"
#include "y.tab.h"
#include "sha1.h"
#include "devManager.h"
#include <ctime>

#include <iterator>
#include <openssl/sha.h>

#include "sha1.h" 

#include "tinyxml2/tinyxml2.h"

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
 */
int PipeLog::addToLog(thread_data* local_buf, unsigned int dev_id, const char* pipe_name, const char* event_name, unsigned int from_user_id, const char* msg,  unsigned int msg_length)
{
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
    uuid37(uid);
    if(memcmp("trust_", pipe_name, 6) == 0)
    {
        local_buf->stm.pipe_messages_insert->execute(uid, (long int)time(NULL), dev_id, pipe_name, event_name, msg, msg_length, from_user_id);
        return 0;
    }


    local_buf->stm.pipes_settings_select->execute(dev_id, pipe_name);
    if(local_buf->stm.pipes_settings_select->fetch())
    {
        local_buf->stm.pipes_settings_select->free();
        return 0;
    }

    int result_length = local_buf->stm.pipes_settings_select->result_length;
    local_buf->stm.pipes_settings_select->free();

    if(result_length > 0)
    {
        // Вставка в бд
        local_buf->stm.pipe_messages_insert->execute(uid, (long int)time(NULL), dev_id, pipe_name, event_name, msg, msg_length, from_user_id);

        // @todo simpleTask Заменить потом на stm выражение
        local_buf->db.query_format("delete from pipe_messages where pipe_messages.id in( \
                                        select p2.id from ( \
                                            select id FROM `pipe_messages` as p3 where p3.dev_id = %d and p3.pipe_name = '%s' order by p3.time desc limit %d, 999\
                                        ) as p2\
                                    )", dev_id, pipe_name, result_length);
    }
    return 0;
}

MySql_connection::MySql_connection():connection(),fragment_buf(appConf::instance()->get_int("main", "fragment_buf_size"))
{
    //printf("create MySql_connection\n");
    bzero(random20bytes, 20);
}

MySql_connection::~MySql_connection()
{
    web_close();
    //printf("delete MySql_connection\n");
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
        TagLoger::debug(Log_MySqlServer, 0, "Processing MYSQL query ------------------------[%d]\n", local_buf->buf[len-1]);
        for(int i=0; i < len && len < 500; i++)
        {
            if(i % 8 == 0)  TagLoger::log(Log_MySqlServer, 0, "!\n%d:\t", i);
            char c = (char)local_buf->buf[i];
            if(c < 32 ) c = '.';
            TagLoger::debug(Log_MySqlServer, 0, "!%02x [%c] ",  (unsigned char)local_buf->buf[i], c );
        }
        TagLoger::debug(Log_MySqlServer, 0, "!\n ");
    }
     
    if(fragment_buf.isLock())
    {
        unsigned char* tmpMsgData = new unsigned char[len];
        memcpy(tmpMsgData, local_buf->buf.getData(), len);

        local_buf->buf.setSize(fragment_buf.getSize() + len + 100);

        memcpy(local_buf->buf.getData(), fragment_buf.getData(), fragment_buf.getSize());
        memcpy(local_buf->buf.getData() + fragment_buf.getSize(), tmpMsgData, len);

        len += fragment_buf.getSize();
        delete[] tmpMsgData;
        fragment_buf.unlock();
        TagLoger::log(Log_ClientServer, 0, "\x1b[1;32mAssembled MySQL packet new len=%d\x1b[0m\n",len);
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

    if(PacketLen + 4 > len)
    { 
        fragment_buf.lock();
        fragment_buf.setSize(len);
        TagLoger::debug(Log_ClientServer, 0, "\x1b[1;31mThe MySQL packet did not fully come len=%d, PacketLen=%d + 4\x1b[0m",len, PacketLen);
        memcpy(fragment_buf.getData(), local_buf->buf.getData(), len);
        return 0;
    }
 
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

        isAuthUser = false; 
        
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
                TagLoger::log(Log_MySqlServer, 0, "Authorization without password authDataLen=%d\n", authDataLen);
 
                // Здесь проверка для того можно ли пускать без пароля по имени пользователя
                if(memcmp(name, "haproxy_check", strlen("haproxy_check")) != 0)
                {
                    TagLoger::warn(Log_MySqlServer, 0, "Authorization failed, not expected size authDataLen=%d\n", authDataLen);
                    Send_Err_Package(SQL_ERR_AUTHENTICATION,"Authentication failure, authDataLen!=20", PacketNomber+1, local_buf, this);
                    return -1;
                }
 
                // Таки решили пустить без пароля
                Send_OK_Package(PacketNomber+1, local_buf, this);
                clientState = STATE_RECEIVED_HANDSHAKE;
                TagLoger::debug(Log_MySqlServer, 0, "Connection will not be closed [1] [MySql_connection]\n");
                return 0;
            }
            else if(authDataLen != 20)
            {
                TagLoger::error(Log_MySqlServer, 0, "Authorization failed, not expected size dev_id=%d, authDataLen=%d\n", dev_id, authDataLen);
                Send_Err_Package(SQL_ERR_AUTHENTICATION,"Authentication failure, authDataLen!=20", PacketNomber+1, local_buf, this);
                return -1;
            }
            else
            {
                p+=20;
            }

            if(ClientFlags & MYSQL_CLIENT_CONNECT_WITH_DB) TagLoger::log(Log_MySqlServer, 0, "dbName:%s\n", p);

            if(devInfo::testDevKey(random20bytes, DevKeyHashStart))
            {
                isAuthUser = true; 
                TagLoger::debug(Log_MySqlServer, 0, "Authorization successful ok\n");
                devManager::instance()->getDevInfo(dev_id)->incrBackendOnline();
                devManager::instance()->getDevInfo(dev_id)->incrMessages();
            }
            else
            {
                isAuthUser = false; 
                TagLoger::error(Log_MySqlServer, 0, "Authorization failed");
                Send_Err_Package(SQL_ERR_AUTHENTICATION,"Authentication failure", PacketNomber+1, local_buf, this);
                return -1;
            }
        }
        else
        {
            isAuthUser = false; 

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
        TagLoger::log(Log_MySqlServer, 0, "Connection will not be closed [2] [MySql_connection]\n");
        return 0;
    }
    else if(clientState == STATE_RECEIVED_HANDSHAKE)
    {
        char com_type = *p;
        TagLoger::log(Log_MySqlServer, 0, "Query: %02x\n", *p);
        devManager::instance()->getDevInfo(dev_id)->incrMessages();

        if( com_type == MYSQL_PROTOCOL_COM_QUERY)
        {
            if(!isAuthUser)
            {
                local_buf->qInfo.setError("Access denied", SQL_ERR_ACCESS_DENIED);
                Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
                return 0;
            }

            devManager::instance()->getDevInfo(dev_id)->incrMessages();
            p++;

            char* startQuery = p;
            int queryLen = strlen(startQuery); 
            _countUerys++;
            TagLoger::warn(Log_MySqlServer, 0, "QUERY[%d, dev_id=%d, len=%d][BASE]:%s\n", _countUerys, dev_id, queryLen, startQuery);

            if(isAuthUser)
            {
                // От имени root пользователя допустима работы репликации
                // Сдвиг в тексте запроса, если к запросу прикреплены мета данные от систем репликации

                int headerLength = strlen("cometqlcluster_v1 set dev_id=");
                if(memcmp("cometqlcluster_v1 set dev_id=", startQuery, headerLength) == 0)
                {
                    cometqlcluster = 1;
                    startQuery += headerLength;

                    headerLength = 0;
                    int new_dev_id = read_int(startQuery, ';', &headerLength);
                    startQuery += headerLength;

                    if(new_dev_id >= 0 && new_dev_id < devManager::instance()->getDevIndexSize())
                    {
                        /**
                         *  Устанавливаем смену пользователя соединения на горячию.
                         *  Чтоб не переподключатся когда приходят запросы от систем репликации на других серверах от имени разных пользователей
                         */
                        dev_id = new_dev_id;
                        TagLoger::log(Log_MySqlServer, 0, "QUERY set dev_id = %d\n", new_dev_id);
                    }
                    else
                    {
                        TagLoger::error(Log_MySqlServer, 0, "QUERY not set dev_id = %d in [[%s]]\n", new_dev_id, startQuery);
                        local_buf->qInfo.setError("Invalid dev_id value", SQL_ERR_AUTHENTICATION);
                        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
                        return 0;
                    }
                }
            }

            if(appConf::instance()->get_bool("main", "useQueryLoger"))
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
                    local_buf->stm.queryLoger->insert(dev_id, startQuery, queryLen);
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

            query_router(local_buf, PacketNomber);
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

        TagLoger::log(Log_MySqlServer, 0, "Connection will not be closed [3] [MySql_connection]\n");
        return 0;
    }

    TagLoger::log(Log_MySqlServer, 0, "Terminate the connection processing [MySql_connection]\n");
    return -1;
}

int MySql_connection::query_router(thread_data* local_buf, int PacketNomber)
{
    if(local_buf->qInfo.command == TOK_SHOW)
    {
        TagLoger::log(Log_MySqlServer, 0, "cmd show:%d\n", local_buf->qInfo.arg_show.command);
        if(local_buf->qInfo.arg_show.command == TOK_DATABASES)
        {
            return sql_show_databases(local_buf,PacketNomber);
        }
        else if(test_api_version(local_buf,PacketNomber))
        {
            if(local_buf->qInfo.arg_show.command == TOK_TABLES)
            {
                return sql_show_tables(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_COLUMNS)
            {
                return sql_show_columns(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_STATUS)
            {
                return sql_show_status(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_PROCESSLIST)
            {
                return sql_show_processlist(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_VARIABLES)
            {
                return sql_show_variables(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_TABLE_STATUS)
            {
                return sql_show_table_status(local_buf,PacketNomber);
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
            return sql_select_database_name(local_buf,PacketNomber);
        }
        else if(local_buf->qInfo.arg_select.command == VAL_SYSTEM_VARIBLE)
        {
            return sql_select_systemvarible(local_buf,PacketNomber);
        }
        else if(test_api_version(local_buf,PacketNomber))
        {
            if(local_buf->qInfo.arg_select.command == TOK_FROM)
            {
                if(local_buf->qInfo.tokCompare("users_auth",  local_buf->qInfo.tableName))
                {
                    return sql_select_from_users_auth(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
                {
                    return sql_select_from_users_time(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("users_data",  local_buf->qInfo.tableName))
                {
                    return sql_select_from_users_data(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("revoked_tokens",  local_buf->qInfo.tableName))
                {
                    return sql_select_from_revoked_tokens(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
                {
                    return sql_select_from_users_messages(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
                {
                    return sql_select_from_pipes_messages(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
                {
                    return sql_select_from_users_in_pipes(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
                {
                    return sql_select_from_pipes(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
                {
                    return sql_select_from_pipes_settings(local_buf,PacketNomber);
                } 
                else if(local_buf->qInfo.tokCompare("conference",  local_buf->qInfo.tableName))
                {
                    return sql_select_from_conference(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("conference_members",  local_buf->qInfo.tableName))
                {
                    return sql_select_from_conference_members(local_buf,PacketNomber);
                }
                else
                {
                    return Send_Err_Package(SQL_ERR_NOT_EXIST, "Table doesn't exist", PacketNomber+1, local_buf, this);
                }
            }
            else
            {
                return Send_Err_Package(SQL_ERR_SYNTAX_ERROR, "Select query has error", PacketNomber+1, local_buf, this);
            }
        }
    }
    else if(local_buf->qInfo.command == TOK_INSERT)
    {
        if(test_api_version(local_buf,PacketNomber))
        {
            if(local_buf->qInfo.tokCompare("users_auth",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_users_auth(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_users_time(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_data",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_users_data(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("revoked_tokens",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_revoked_tokens(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_users_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_pipes_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_users_in_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_pipes_settings(local_buf,PacketNomber);
            } 
            else if(local_buf->qInfo.tokCompare("conference",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_conference(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("conference_members",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_conference_members(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("dialogs",  local_buf->qInfo.tableName))
            {
                return sql_insert_into_dialogs(local_buf,PacketNomber);
            }
            else
            {
                return Send_Err_Package(SQL_ERR_NOT_EXIST, "Table doesn't exist", PacketNomber+1, local_buf, this);
            }
        }
    }
    else if(local_buf->qInfo.command == TOK_DELETE)
    {
        if(test_api_version(local_buf,PacketNomber))
        {
            if(local_buf->qInfo.tokCompare("users_auth",  local_buf->qInfo.tableName))
            {
                return sql_delete_from_users_auth(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_data",  local_buf->qInfo.tableName))
            {
                return sql_delete_from_users_data(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("revoked_tokens",  local_buf->qInfo.tableName))
            {
                return sql_delete_from_revoked_tokens(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
            {
                return sql_delete_from_users_time(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
            {
                return sql_delete_from_users_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
            {
                return sql_delete_from_pipes_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
            {
                return sql_delete_from_users_in_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
            {
                return sql_delete_from_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
            {
                return sql_delete_from_pipes_settings(local_buf,PacketNomber);
            } 
            else if(local_buf->qInfo.tokCompare("conference",  local_buf->qInfo.tableName))
            {
                return sql_delete_from_conference(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("conference_members",  local_buf->qInfo.tableName))
            {
                return sql_delete_from_conference_members(local_buf,PacketNomber);
            }
            else
            {
                return Send_Err_Package(SQL_ERR_NOT_EXIST, "Table doesn't exist", PacketNomber+1, local_buf, this);
            }
        }
    }
    else if(local_buf->qInfo.command == TOK_SET)
    {
        sql_set_value(local_buf,PacketNomber);
    }
    else
    {
        TagLoger::log(Log_MySqlServer, 0, "cmd undefined:%d %s\n", local_buf->qInfo.arg_select.command, local_buf->qInfo.StartQury);
        return Send_OK_Package(PacketNomber+1, local_buf, this);
    }

    return 0;
}

/**
 *  Если версия api не выбрана то вернёт false и отправит информацию об ошибке клиенту.
 */
bool MySql_connection::test_api_version(thread_data* local_buf, unsigned int PacketNomber)
{
    TagLoger::log(Log_MySqlServer, 0, "api_version %d", api_version);
    if(api_version != -1)
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
    if(strcmp(db_name, "CometQL_v1") == 0)
    {
        TagLoger::log(Log_MySqlServer, 0, "OK set api_version `%d`", 1);
        api_version = 1;
        return true;
    }

    TagLoger::log(Log_MySqlServer, 0, "Error: set api_version `%s`", db_name); 
    
    int conf_api_version = appConf::instance()->get_int("main", "api_version");
    if(conf_api_version > 0)
    {
        TagLoger::log(Log_MySqlServer, 0, "Set api_version `%d` from ini file", conf_api_version); 
        api_version =  conf_api_version;
        return true;
    }
    else
    {
        api_version = -1;
    }
    return false;
}

int MySql_connection::sql_set_value(thread_data* local_buf, unsigned int PacketNomber)
{
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
    return 0;
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
    // Отправляем пакет описания 1 колонки
    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();

    local_buf->sql.columns[0] = "Tables";
    answer += HeadAnswer(1, &local_buf->sql.columns[0], PacketNomber, answer);

    MySqlResulValue value;

    value = "users_auth";
    answer += RowPackage(1, &value, ++PacketNomber, answer);

    value = "users_time";
    answer += RowPackage(1, &value, ++PacketNomber, answer);

    value = "users_messages";
    answer += RowPackage(1, &value, ++PacketNomber, answer);

    value = "pipes_messages";
    answer += RowPackage(1, &value, ++PacketNomber, answer);

    value = "users_in_pipes";
    answer += RowPackage(1, &value, ++PacketNomber, answer);

    value = "pipes";
    answer += RowPackage(1, &value, ++PacketNomber, answer);

    value = "pipes_settings";
    answer += RowPackage(1, &value, ++PacketNomber, answer);


    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());

    local_buf->answer_buf.unlock();
    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package
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
    values[++i] = "Table with authorization data for users"; // Comment

    delta = RowPackage(18, values, ++PacketNomber, answer);
    answer += delta;


    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());

    local_buf->answer_buf.unlock();
    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package
    return 0;
}

int MySql_connection::sql_show_processlist(thread_data* local_buf, unsigned int PacketNomber)
{
    if(!isAuthUser)
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

    value[0] = tcpServer <Freeswitch_connection>::instance()->bm.getServiceName();
    for(int i =0; i < tcpServer <Freeswitch_connection>::instance()->bm.get_th_num(); i++)
    {
        value[1] = i;
        value[2] = (char)tcpServer <Freeswitch_connection>::instance()->bm.get_th_status(i);
        value[3] = tcpServer <Freeswitch_connection>::instance()->bm.get_th_count(i);
        answer += RowPackage(4, value, ++PacketNomber, answer);
    }

    value[0] = tcpServer <CometQLProxy_connection>::instance()->bm.getServiceName();
    for(int i =0; i < tcpServer <CometQLProxy_connection>::instance()->bm.get_th_num(); i++)
    {
        value[1] = i;
        value[2] = (char)tcpServer <CometQLProxy_connection>::instance()->bm.get_th_status(i);
        value[3] = tcpServer <CometQLProxy_connection>::instance()->bm.get_th_count(i);
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
    // @todo simpleTask не доделан like и флаги SESSION и GLOBAL
    // @todo simpleTask показывать параметры ini файла

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
    if( !isAuthUser && (
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
        while(fscanf(loadavgFp, "%32s %32s %32s", name, val, tmpNull) != EOF && i < 34)
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

        value[0] = "id";
        value[1] = dev_id;
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "user_online";
        value[1] = devManager::instance()->getDevInfo(dev_id)->getFrontendOnline();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "server_online";
        value[1] = devManager::instance()->getDevInfo(dev_id)->getBackendOnline();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;

        value[0] = "messages";
        value[1] = devManager::instance()->getDevInfo(dev_id)->getMessages();
        delta = RowPackage(2, value, ++PacketNomber, answer);
        answer += delta;
    }
    else
    {
        // не известный флаг, ошибка в граматике
        value[0] = "flag";
        value[1] = "Unknown flag, grammar error";

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
    values = MYSQL_SYSTEMVARIBLE;

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
        values = "CometQL_v1";
        //char* name = values;
        //name[9] = '0' + api_version;
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


    char hash[MAX_JWT_LEN+1];
    bzero(hash, MAX_JWT_LEN+1);

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

        if(!devManager::instance()->getDevInfo(dev_id)->index->get_hash(local_buf, userId, hash))
        {
            continue;
        }

        if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = userId;
        if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = hash;

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
        Send_Err_Package(SQL_ERR_INVALID_DATA, "Contains an invalid characters in the string hash", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]].tokLen > USER_HASH_LEN)
    {
        Send_Err_Package(SQL_ERR_OVERFLOW, "Hash string is too long", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(!devManager::instance()->getDevInfo(dev_id)->index->add_hash(local_buf, user_id, hash))
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

        if(!devManager::instance()->getDevInfo(dev_id)->index->delete_hash(local_buf, userId))
        {
            continue;
        }

        int *conection_id = devManager::instance()->getDevInfo(dev_id)->index->get_conection_id(local_buf, userId);
        if(conection_id != NULL)
        {
            for(int j=0; j< MAX_CONECTION_ON_USER_ID; j++)
            {
                if( conection_id[j] == 0 )
                {
                    continue;
                }

                CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(conection_id[j]);
                if(r && r->web_user_dev_id == dev_id)
                {
                    tcpServer <Client_connection>::instance()->deleteClient(r->getfd(), local_buf);
                }
                else
                {
                    TagLoger::log(Log_MySqlServer, 0, "Connection object is not found\n");
                }
            }
        }
        else
        {
            TagLoger::log(Log_MySqlServer, 0, "Connection ID not found\n");
        }


        countRows++;
    }

    /**
     * Для операций удаления affectedRows возвращатся не будет в целях оптимизации.
     */
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
    return 0;
}

 
/**
 * users_data - представляет из себя элемент key-value хранилища доступного на запись из CometQL и на чтение из CometQL и JS API
 * 
 * 
 * Надо подумать на тем нужна ли эта таблица
 * И надо ли организовывать полноценное key-value хранилище с доступом к данным как из CometQL так и из JS API с возможностью задавать тип доступа
 * 
 * Идея частичного доступа к данным из JS API вроде интересна.
 * 
 * 
 * 
 */
int MySql_connection::sql_select_from_users_data(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "id",
        "data", // Можно записать только из cometQL
        //"public",    // Можно записать и из cometQL и из js api 
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
        if(userId <= 0)
        {
            continue;
        }
 
        local_buf->stm.users_data_select->execute(dev_id, userId);
        while(!local_buf->stm.users_data_select->fetch())
        {
            if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = userId;
            if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = local_buf->stm.users_data_select->result_data;

            countRows++;
        }
        local_buf->stm.users_data_select->free();
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_insert_into_users_data(thread_data* local_buf, unsigned int PacketNomber)
{ 
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "id",
        "data"
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
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `data` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    int user_id = local_buf->qInfo.tokToInt(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]]);
    if(user_id <= 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "The id field must be non-negative", PacketNomber+1, local_buf, this);
        return 0;
    }

    char* data = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]]);
    int data_length = local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]].tokLen;
    
    data[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]].tokLen] = 0;
 
    if(local_buf->stm.users_data_replace->execute(dev_id, user_id, data, data_length) < 0)
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

int MySql_connection::sql_delete_from_users_data(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "id",
        "data"
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

        if(local_buf->stm.users_data_delete->execute(dev_id, userId) < 0)
        {
            continue;
        }
             
        countRows++;
    }

    /**
     * Для операций удаления affectedRows возвращатся не будет в целях оптимизации.
     */
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
    return 0;
}


/** 
 */
int MySql_connection::sql_select_from_revoked_tokens(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = { 
        "token", 
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

        char* token = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo); 
        if(token <= 0)
        {
            continue;
        }
 
        local_buf->stm.revoked_tokens_select->execute(dev_id, token);
        while(!local_buf->stm.revoked_tokens_select->fetch())
        {
            if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = token; 
            countRows++;
        }
        local_buf->stm.revoked_tokens_select->free(); 
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_insert_into_revoked_tokens(thread_data* local_buf, unsigned int PacketNomber)
{ 
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "token"
    };

    if(!local_buf->sql.prepare_columns_for_insert(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[0] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `token` is required", PacketNomber+1, local_buf, this);
        return 0;
    }
 
    int user_id = local_buf->qInfo.tokToInt(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]]);
    if(user_id <= 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "The id field must be non-negative", PacketNomber+1, local_buf, this);
        return 0;
    }

    char* token = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]]); 
    token[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]].tokLen] = 0;
    if(local_buf->stm.revoked_tokens_replace->execute(dev_id, token) < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "Error in token", PacketNomber+1, local_buf, this);
        return 0;
    }

    /**
     * Данные доступны в режиме кластера из за необходимости на каждой ноде всех ключей авторизации хотя и это утверждение не факт что верное
     * Но даже если и нет то можно их легко посчитать
     */
    Send_OK_Package(1, user_id, PacketNomber+1, local_buf, this);

    return 0;
}

int MySql_connection::sql_delete_from_revoked_tokens(thread_data* local_buf, unsigned int PacketNomber)
{
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "token"
    };

    if(!local_buf->sql.prepare_where_expressions(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    if(idExprPos == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Deleting without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
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

        char* token = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo); 
        if(token <= 0)
        {
            continue;
        }
 
        if(local_buf->stm.revoked_tokens_delete->execute(dev_id, token) < 0)
        {
            continue;
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

        long time = devManager::instance()->getDevInfo(dev_id)->index->get_last_online_time(local_buf, userId);

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


        local_buf->stm.users_queue_select->execute(dev_id, userId, 10);
        while(!local_buf->stm.users_queue_select->fetch())
        {
            if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = userId;
            if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = local_buf->stm.users_queue_select->result_id;
            if(local_buf->sql.useColumn(2)) local_buf->sql.getValue(countRows, 2) = local_buf->stm.users_queue_select->result_event;
            if(local_buf->sql.useColumn(3)) local_buf->sql.getValue(countRows, 3) = local_buf->stm.users_queue_select->result_message;

            countRows++;
        }
        local_buf->stm.users_queue_select->free();
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
    //char messageQuote = local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]].quote;

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

    int affectedRows = 0;
    
    if(cometqlcluster != 0)
    {
        affectedRows = internalApi::local_send_to_user(local_buf, dev_id, user_id, pipe_event, local_buf->answer_buf.getData());
    }
    else
    {
        affectedRows = internalApi::cluster_send_to_user(local_buf, dev_id, user_id, pipe_event, local_buf->answer_buf.getData()); 
    }
    
    local_buf->answer_buf.unlock();
    /**
     * Если affectedRows == 0 то сообщение не доставлено и не может быть помещено в очередь для доставки позже из за переполненения очереди сообщений.
     * Если affectedRows == 1 то сообщение либо доставлено сразу либо помещено в очередь для доставки потом.
     *
     * Скорее всего эти данные доступны в режиме кластера, но ещё не факт.
     */
    Send_OK_Package(affectedRows, user_id, PacketNomber+1, local_buf, this);
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

        local_buf->stm.users_queue_delete->execute((long int)time(NULL), dev_id, user_id);
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
        "id",
        "time",
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

    int idExprPos = local_buf->sql.expressionsPositions[2];
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

        local_buf->stm.pipe_messages_select->execute(dev_id, pipe_name, 99);
        while(!local_buf->stm.pipe_messages_select->fetch())
        {
            if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = local_buf->stm.pipe_messages_select->result_id;
            if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = (long)local_buf->stm.pipe_messages_select->result_time;
            if(local_buf->sql.useColumn(2)) local_buf->sql.getValue(countRows, 2) = pipe_name;
            if(local_buf->sql.useColumn(3)) local_buf->sql.getValue(countRows, 3) = countRows;
            if(local_buf->sql.useColumn(4)) local_buf->sql.getValue(countRows, 4) = local_buf->stm.pipe_messages_select->result_event;
            if(local_buf->sql.useColumn(5)) local_buf->sql.getValue(countRows, 5) = local_buf->stm.pipe_messages_select->result_message;
            if(local_buf->sql.useColumn(6)) local_buf->sql.getValue(countRows, 6) = (long)local_buf->stm.pipe_messages_select->result_user_id;

            countRows++;
        }
        local_buf->stm.pipe_messages_select->free();
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_insert_into_pipes_messages(thread_data* local_buf, unsigned int PacketNomber)
{
    TagLoger::log(Log_MySqlServer, 0, " >MySql_connection::insert_into_pipes_messages\n");

    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "id",
        "time",
        "name",
        "index",
        "event",
        "message",
        "user_id"
    };

    if(!local_buf->sql.prepare_columns_for_insert(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[2] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `name` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[4] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `event` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.columPositions[5] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `message` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    char* pipe_name = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]]);
    pipe_name[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]].tokLen] = 0;
    if(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]].tokLen > PIPE_NAME_LEN)
    {
        Send_Err_Package(SQL_ERR_OVERFLOW, "Channel name is too long", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]].tokLen < 3)
    {
        Send_Err_Package(SQL_ERR_EMPTY, "Channel name is too short", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(!AZ09test(pipe_name, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]].tokLen))
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "The channel name can contain only the characters A-Za-z0-9_-", PacketNomber+1, local_buf, this);
        return 0;
    }

    char* pipe_event = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[4]]);
    pipe_event[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[4]].tokLen] = 0;

    if(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[4]].tokLen > EVENT_NAME_LEN)
    {
        Send_Err_Package(SQL_ERR_OVERFLOW, "Channel event_name is too long", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(!AZ09test(pipe_event, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[4]].tokLen))
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "The channel name can contain only the characters A-Za-z0-9_-", PacketNomber+1, local_buf, this);
        return 0;
    }

    char* message = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[5]]);
    message[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[5]].tokLen] = 0;

    local_buf->answer_buf.lock();
    //json_escape_string(message, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[5]].tokLen, local_buf->answer_buf.getData());
    mysql_real_escape_string(local_buf->db.getLink(), local_buf->answer_buf.getData(), message, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[5]].tokLen);


    char serverData[EVENT_NAME_LEN+64];
    snprintf(serverData, EVENT_NAME_LEN+64, "\"event_name\":\"%s\"", pipe_event);

    TagLoger::log(Log_MySqlServer, 0, "message:%s\n", local_buf->answer_buf.getData());

    PipeLog::addToLog(local_buf, dev_id, pipe_name, pipe_event, 0, message, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[5]].tokLen);
    internalApi::send_event_to_pipe(local_buf, pipe_name, local_buf->answer_buf.getData(), dev_id, serverData);

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
        "id",
        "time",
        "name",
        "index",
        "event",
        "message",
        "user_id"
    };

    if(!local_buf->sql.prepare_where_expressions(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.expressionsPositions[0] == -1 && local_buf->sql.expressionsPositions[2] == -1)
    {
        Send_Err_Package(SQL_ERR_WHERE_EXPRESSIONS, "Selection without transferring the requested values of the primary key is not supported", PacketNomber+1, local_buf, this);
        return 0;
    }

    if(local_buf->sql.expressionsPositions[2] != -1)
    {
        int idExprPos = local_buf->sql.expressionsPositions[2];
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

            local_buf->stm.pipe_messages_delete->execute(0x7FFFFFFF-1, dev_id, pipe_name); // максимальное значение unixtime
            countRows++;
        }
    }

    if(local_buf->sql.expressionsPositions[0] != -1)
    {
        int idExprPos = local_buf->sql.expressionsPositions[0];
        int countRows = 0;
        for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
        {
            if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
            {
                // Значения закончились
                break;
            }

            char* msg_uuid = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo);
            int uuidLen = local_buf->qInfo.where.whereExprValue[idExprPos][i].tokLen;
            msg_uuid[uuidLen] = 0;
            if(uuidLen < 3 || uuidLen > PIPE_NAME_LEN || !AZ09test(msg_uuid, uuidLen))
            {
                continue;
            }

            local_buf->stm.pipe_messages_delete_by_message_id->execute(msg_uuid, dev_id);
            countRows++;
        }
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



        CP<Pipe> pipe = devManager::instance()->getDevInfo(dev_id)->findPipe(std::string(pipe_name));

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

        auto it = pipe->subscribers->begin();
        while(it)
        {
            int val =  it->data;
            TagLoger::log(Log_MySqlServer, 0, "pipe[%s] client_id - %d\n",pipe_name, val);

            CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(val);
            if(r && r->web_user_dev_id == dev_id)
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


// pipes
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


        CP<Pipe> pipe = devManager::instance()->getDevInfo(dev_id)->findPipe(std::string(pipe_name));
        int pipe_size = 0;
        if(!pipe.isNULL())
        {
            pipe_size = pipe->size();
        }

        TagLoger::log(Log_MySqlServer, 0, "text>%s\n",pipe_name);
        if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = pipe_name;
        if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = pipe_size;

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

        local_buf->stm.pipes_settings_select->execute(dev_id, pipe_name);
        if(local_buf->stm.pipes_settings_select->fetch())
        {
            local_buf->stm.pipes_settings_select->free();
            continue;
        }

        int result_length = local_buf->stm.pipes_settings_select->result_length;
        local_buf->stm.pipes_settings_select->free();

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

    PipeSettings pipe_settings(dev_id, pipe_name);

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

        local_buf->db.query_format("DELETE FROM `pipes_settings` WHERE `dev_id` = %d and `name` = '%s' ;", dev_id, pipe_name);
        local_buf->stm.pipe_messages_delete->execute(0x7FFFFFFF-1, dev_id, pipe_name); // максимальное значение unixtime

        countRows++;
    }

    /**
     * Для операций удаления affectedRows возвращатся не будет в целях оптимизации.
     */
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
    return 0;
}

int MySql_connection::sql_insert_into_conference(thread_data* local_buf, unsigned int PacketNomber)
{
    TagLoger::log(Log_MySqlServer, 0, " >MySql_connection::conference\n");

    // @todo заменить на более читаемый код на базе перечислений или чего то подобного
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",         // имя конференции (только цифры)
        "user_id",      // пользователь
        "caller_id",    // инициатор звонка
        "message",      // сообщение
        "profile",      // Режим  video_*, audio_* [!Проверить что будет если у конференции одно имя но разные режимы]
        "stream"        // Не пусто и не 0 если активирован режим стриминга [это поле относится к пользователю а не конференции так как зависит от mode ]
         
    };
    // Поле tabUUID передается или отслеживается через текст сообщения `message`

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

    if(local_buf->sql.columPositions[1] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `user_id` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    int caller_id = 0;
    if(local_buf->sql.columPositions[2] >= 0)
    {
        caller_id = local_buf->qInfo.tokToInt(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[2]]);
    }

    if(local_buf->sql.columPositions[3] < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "field `message` is required", PacketNomber+1, local_buf, this);
        return 0;
    }

    int user_id = local_buf->qInfo.tokToInt(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[1]]);
    if(user_id < 0)
    {
        Send_Err_Package(SQL_ERR_INVALID_DATA, "The user_id field must be non-negative", PacketNomber+1, local_buf, this);
        return 0;
    }

    int stream = local_buf->qInfo.tokToInt(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[5]]);;

    int nameLength = local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]].tokLen;
    char* name = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[0]]);
    name[nameLength] = 0;
 
    char default_profile[] = "default";
    char* profile = default_profile;
    if(local_buf->sql.columPositions[4] >= 0)
    {
        profile = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[4]]);
        profile[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[4]].tokLen] = 0;
    }

    auto sipcluster = local_buf->fs_eslCluster;
    
    if(sipcluster.empty())
    {
        Send_Err_Package(SQL_ERR_INTERNAL_SERVER, "field in ini file not set option `host` in section `sip`", PacketNomber+1, local_buf, this);
        return 0;
    }

    // Дополнение нулями dev_id до 6 знаков
    
    std::string sipNumber;

    sipNumber.append(std::to_string(dev_id));
    sipNumber.append("*");
    sipNumber.append(name); // Нода определяется исходя из имени конференции
    sipNumber.append("*");
    sipNumber.append(profile);

    TagLoger::log(Log_MySqlServer, 0, " >sipNumber=%s", sipNumber.data());

    int serverPort = 7443;
    std::string serverName;

    long serverNumber = dev_id*1000000;
    for(int i = 0; i< nameLength; i++)
    {
        serverNumber += name[i]*(10*i);
    }
    int serverIndex = serverNumber % sipcluster.size();
 
    serverName = sipcluster[serverIndex]->getWSHost();
    serverPort = sipcluster[serverIndex]->getWSPort();
      
    char callKey[37];
    bzero(callKey, 37);
    uuid37(callKey);

    char srcHash[256];
    sprintf(srcHash, "%64s_%64s_%d_%d_%32s", sipNumber.data(), serverName.data(), serverPort, dev_id,  appConf::instance()->get_chars("sip", "pipesalt"));

    unsigned char sha1_data[20];
    bzero(sha1_data, 20);
    sha1::calc(srcHash,  strlen(srcHash) ,sha1_data);


    char callPipe[255];
    snprintf(callPipe, 255, "web_syscall_v1_%s", base64_encode(sha1_data, 20).data());
    for(int i =0; i<255; i++)
    {
        if(callPipe[i] == 0)
        {
            break;
        }
        else if(callPipe[i] < 'A' || callPipe[i] > 'z' )
        {
            callPipe[i] = 'A';
        }
    }


    char* message = local_buf->qInfo.tokStart(local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]]);
    message[local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]].tokLen] = 0;

    local_buf->answer_buf.lock();
    mysql_real_escape_string(local_buf->db.getLink(), local_buf->answer_buf.getData(), message, local_buf->qInfo.arg_insert.values[local_buf->sql.columPositions[3]].tokLen);

    std::string nodeId;
    nodeId.append(serverName);
    nodeId.append(":");
    nodeId.append(std::to_string(serverPort));

    // Запись в бд позволит узнавать кто и на каких нодах должен говорить.
    // Задачача таблицы conference в бд это хранить соответсыие имени конференции к ноде FS на которой идёт разговор (сомнительно)
    // Задачей номер 2 можно считать накопление информации для биллинга. (сомнительно)
    // !!!! Важно что таблица может быть использована для контроля доступа чтоб пользователь звонил на свой намер а не куда попало.
    local_buf->stm.conference_insert->execute(
                                                dev_id,
                                                name,
                                                user_id,
                                                caller_id,
                                                local_buf->answer_buf.getData(),
                                                strlen(local_buf->answer_buf.getData()),
                                                profile,
                                                "",
                                                nodeId.data());
    /*
        $msg = json_encode(
            array(
                "user_id" => $user_id,
                "room_id" => $room_id,
                "tabUUID" => $tabUUID,
                "type" => $type,
                "conference" => $isConference,
                "sys" => array(
                    "conference" => $isConference,
                    "serverName" => $serverName,
                    "callPipe" => $callPipe,
                    "serverPort" => $serverPort,
                    "sipNumber" => $sipNumber,
                    "callKey" => md5(uniqid(true)),
                    "type" => $type,                // 'audio' | 'video'
                    "caller_id" => $user_id         // Идентификатор того кто начал звонок
                )
            )
        );
    */

    char* msgData = new char[appConf::instance()->get_int("main", "buf_size")]; // @fixme Проверить насколько правильно расчитан объём выделяемой памяти на случай пакета длинее "main"->"buf_size"
    bzero(msgData, appConf::instance()->get_int("main", "buf_size"));

    snprintf(msgData, appConf::instance()->get_int("main", "buf_size"),
            "{\"message\":\"%s\",\"sys\":{\"conference\":true,\"serverName\":\"%s\",\"serverPort\":\"%d\",\"callKey\":\"%s\",\"callPipe\":\"%s\",\"sipNumber\":\"%s\",\"caller_id\":\"%d\",\"mode\":\"%s\",\"conference_name\":\"%s\",\"stream\":%d}}",
            local_buf->answer_buf.getData(),
            serverName.data(),
            serverPort,
            callKey,
            callPipe,
            sipNumber.data(),
            caller_id,
            profile,
            name,
            stream);

    local_buf->answer_buf.unlock();

    local_buf->answer_buf.lock();
    mysql_real_escape_string(local_buf->db.getLink(), local_buf->answer_buf.getData(), msgData, strlen(msgData));

    TagLoger::error(Log_MySqlServer, 0, " >conference answer=%s", local_buf->answer_buf.getData());
    internalApi::cluster_send_to_user(local_buf, dev_id, user_id, "sys_sipCall", local_buf->answer_buf.getData());

    local_buf->answer_buf.unlock();

    Send_OK_Package(1, 0, PacketNomber+1, local_buf, this);
    return 0;
}

int MySql_connection::sql_select_from_conference(thread_data* local_buf, unsigned int PacketNomber)
{
    // @todo заменить на более читаемый код на базе перечислений или чего то подобного
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",         // имя конференции (только цифры)
        "user_id",      // пользователь
        "caller_id",    // инициатор звонка
        "message",      // сообщение
        "profile",      // Режим  video_*, audio_*
        "stream",       // Не пусто и не 0 если активирован режим стриминга [это поле относится к пользователю а не конференции так как зависит от mode ]
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

        char* room_name = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo);
        int nameLen = local_buf->qInfo.where.whereExprValue[idExprPos][i].tokLen;
        room_name[nameLen] = 0;
        if(nameLen < 1 || nameLen > PIPE_NAME_LEN || !AZ09test(room_name, nameLen))
        {
            continue;
        }

        TagLoger::log(Log_MySqlServer, 0, "dev_id=%d text>%s\n",dev_id, room_name);

        int rs = local_buf->stm.conference_select->execute(dev_id, room_name);
        
        while(true)
        {
            rs = local_buf->stm.conference_select->fetch();
            if(rs)
            {
                local_buf->stm.conference_select->free();
                break;
            }

            TagLoger::log(Log_MySqlServer, 0, "show for room_name=%s\n",room_name);

            printf("result_message=%s\n", local_buf->stm.conference_select->result_message);
            printf("result_profile=%s\n", local_buf->stm.conference_select->result_profile);
            printf("result_stream=%s\n", local_buf->stm.conference_select->result_stream);

            if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = room_name;
            if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = (long)local_buf->stm.conference_select->result_user_id;
            if(local_buf->sql.useColumn(2)) local_buf->sql.getValue(countRows, 2) = (long)local_buf->stm.conference_select->result_caller_id;
            if(local_buf->sql.useColumn(3)) local_buf->sql.getValue(countRows, 3) = (const char*)local_buf->stm.conference_select->result_message;
            if(local_buf->sql.useColumn(4)) local_buf->sql.getValue(countRows, 4) = (const char*)local_buf->stm.conference_select->result_profile;
            if(local_buf->sql.useColumn(5)) local_buf->sql.getValue(countRows, 5) = (const char*)local_buf->stm.conference_select->result_stream;
          
            countRows++;
        }
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_delete_from_conference(thread_data* local_buf, unsigned int PacketNomber)
{
    //Удаляет конференцию и всех участников.
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",         // имя конференции (только цифры)
        "user_id",      // пользователь
        "caller_id",    // инициатор звонка
        "message",      // сообщение
        "profile",      // Режим  video_*, audio_*
        "stream",       // Не пусто и не 0 если активирован режим стриминга [это поле относится к пользователю а не конференции так как зависит от mode ]
    };

    if(!local_buf->sql.prepare_where_expressions(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];
    int userIdExprPos = local_buf->sql.expressionsPositions[1];

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

        char* room_name = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo);
        int nameLen = local_buf->qInfo.where.whereExprValue[idExprPos][i].tokLen;
        room_name[nameLen] = 0;
        if(nameLen < 1 || nameLen > PIPE_NAME_LEN || !AZ09test(room_name, nameLen))
        {
            continue;
        }

        TagLoger::log(Log_MySqlServer, 0, "dev_id=%d text>%s\n",dev_id, room_name);
 
        for(int j=0; j< MAX_EXPRESSIONS_VALUES; j++)
        {
            if(local_buf->qInfo.where.whereExprValue[userIdExprPos][j].isNull())
            {
                // Значения закончились
                break;
            }

            if(local_buf->stm.conference_delete->execute(dev_id, room_name, local_buf->qInfo.where.whereExprValue[userIdExprPos][j].ToInt(local_buf->qInfo)) < 0)
            {
                continue;
            }
            countRows++;

        } 
    }

    /**
     * Для операций удаления affectedRows возвращатся не будет в целях оптимизации.
     */
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);
    return 0; 
}


int MySql_connection::sql_insert_into_conference_members(thread_data* local_buf, unsigned int PacketNomber)
{
    Send_Err_Package(SQL_ERR_READ_ONLY, "insert into `conference_members` is not ready yet", PacketNomber+1, local_buf, this);
    return 0;
}

int MySql_connection::sql_select_from_conference_members(thread_data* local_buf, unsigned int PacketNomber)
{
    // @todo заменить на более читаемый код на базе перечислений или чего то подобного
    const static char* columDef[MAX_COLUMNS_COUNT] = {
        "name",         // имя конференции (только цифры)
        "user_id",      // пользователь
        //"mode",
        "join_time",
        "last_talking",
        //"energy",
        //"volume_in",
        //"volume_out",
        //"output_volume",
/*
        "can_hear",
        "can_see",
        "can_speak",
        "talking",
        "has_video",
        "has_floor",
        "mute_detect",*/
    };

    /**
    <?xml version="1.0"?>
    <conferences>
      <conference name="000015000362-FSdefaultDomain" member-count="1" ghost-count="0" rate="8000" uuid="01a16dd2-0954-4bed-9381-f4735ffcb85a" running="true" answered="true" enforce_min="true" dynamic="true" exit_sound="true" enter_sound="true" recording="true" run_time="1372">
        <members>
          <member type="recording_node">
            <record_path>rtmp://ecort-n2.comet.su/cam1/000015000362-FSdefaultDomain</record_path>
            <join_time type="UNIX-epoch">1513829830</join_time>
          </member>
          <member type="caller">
            <id>5</id>
            <flags>
              <can_hear>true</can_hear>
              <can_see>true</can_see>
              <can_speak>true</can_speak>
              <mute_detect>false</mute_detect>
              <talking>false</talking>
              <has_video>true</has_video>
              <video_bridge>false</video_bridge>
              <has_floor>true</has_floor>
              <is_moderator>false</is_moderator>
              <end_conference>false</end_conference>
              <is_ghost>false</is_ghost>
            </flags>
            <uuid>e481f1b9-793f-434a-b6e2-ea6a80ef1a98</uuid>
            <caller_id_name>0000151</caller_id_name>
            <caller_id_number>0000151</caller_id_number>
            <join_time>1372</join_time>
            <last_talking>167</last_talking>
            <energy>100</energy>
            <volume_in>0</volume_in>
            <volume_out>0</volume_out>
            <output-volume>0</output-volume>
          </member>
        </members>
      </conference>
    </conferences>
     */
    if(!local_buf->sql.prepare_columns_for_select(columDef, local_buf->qInfo))
    {
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, PacketNomber+1, local_buf, this);
        return 0;
    }

    int idExprPos = local_buf->sql.expressionsPositions[0];

    int countRows = 0;
    if(idExprPos == -1)
    {
        // Получить список всех конференций
        
        fs_esl *link;
        auto it = local_buf->fs_eslCluster.begin();
        while(it != local_buf->fs_eslCluster.end())
        {
            link = *it;

            TagLoger::warn(Log_MySqlServer, 0, "compare for room_name=ALL node=%s and node=%s\n", link->getId().data(), local_buf->stm.conference_select->result_node);
             
            link->exec("api conference xml_list\n\n");
            if (link->getHandle().last_sr_event && link->getHandle().last_sr_event->body)
            {
                TagLoger::debug(Log_MySqlServer, 0, "command `api conference xml_list` body=%s\n", link->getHandle().last_sr_event->body); 
                
                tinyxml2::XMLDocument xmlDoc;
                xmlDoc.Parse(link->getHandle().last_sr_event->body, strlen(link->getHandle().last_sr_event->body));

                tinyxml2::XMLElement *levelElement = xmlDoc.FirstChildElement();
                if (levelElement == nullptr)
                {
                    continue;
                }

                for (tinyxml2::XMLElement* child = levelElement->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
                {
                    TagLoger::debug(Log_MySqlServer, 0, "conference name=%s, time=%s\n", child->Attribute("name"), child->Attribute("run_time"));

                    std::string room_dev_id;
                    std::string curent_room_name;
                    std::string room_mode;
                    const char* startpos =  child->Attribute("name");
 
                    int numberlen = strlen(startpos);
                    for(int k = 0; k< numberlen; k++)
                    {
                        if(startpos[k] == '*')
                        {
                            room_dev_id.append(startpos, k);
                            startpos += k + 1;
                            break;
                        }
 
                    }

                    try{
                         if(std::stoi(room_dev_id.data()) != dev_id)
                         {
                            continue;
                         }
                    }catch(...)
                    {
                        TagLoger::error(Log_MySqlServer, 0, "\x1b[1;31mget_int exeption for room_dev_id=%s\x1b[0m\n", room_dev_id.data());
                        continue;
                    }

                    numberlen = strlen(startpos); 
                    for(int k = 0; k< numberlen; k++)
                    {
                        if(startpos[k] == '*' || startpos[k] == '-')
                        {
                            curent_room_name.append(startpos, k);
                            startpos += k + 1;
                            break;
                        } 
                    }
 
                    tinyxml2::XMLElement* members = child->FirstChildElement();
                    for (tinyxml2::XMLElement* member = members->FirstChildElement(); member != NULL; member = member->NextSiblingElement())
                    {
                        const char* type = member->Attribute("type");
                        if(type == nullptr)
                        {
                            continue;
                        }
                        else if(strcmp(type, "recording_node") == 0)
                        {
                            TagLoger::debug(Log_MySqlServer, 0, "\tmember type=%s, record_path=%s\n", type, member->FirstChildElement("record_path")->GetText());
                        }
                        else if(strcmp(type, "caller") == 0)
                        {
                            TagLoger::debug(Log_MySqlServer, 0, "\tmember type=%s, id=%s, join_time=%s, last_talking=%s\n", type,
                                    member->FirstChildElement("id")->GetText(),
                                    member->FirstChildElement("join_time")->GetText(),
                                    member->FirstChildElement("last_talking")->GetText());
 
                            int test_dev_id = 0;
                            int test_user_id = 0;

                            const char* caller_id_name = member->FirstChildElement("caller_id_name")->GetText();
                            if(caller_id_name == NULL)
                            {
                                continue;
                            }

                            sscanf(caller_id_name, "%6d%d", &test_dev_id, &test_user_id);
                            if(test_dev_id != dev_id)
                            {
                                continue;
                            }


                            if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = curent_room_name.data();
                            //if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = room_mode;
                            if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = test_user_id;
                            if(local_buf->sql.useColumn(2)) local_buf->sql.getValue(countRows, 2) = member->FirstChildElement("join_time")->GetText();
                            if(local_buf->sql.useColumn(3)) local_buf->sql.getValue(countRows, 3) = member->FirstChildElement("last_talking")->GetText();
                            //if(local_buf->sql.useColumn(4)) local_buf->sql.getValue(countRows, 4) = member->FirstChildElement("energy")->GetText();
                            //if(local_buf->sql.useColumn(5)) local_buf->sql.getValue(countRows, 5) = member->FirstChildElement("volume_in")->GetText();
                            //if(local_buf->sql.useColumn(6)) local_buf->sql.getValue(countRows, 6) = member->FirstChildElement("volume_out")->GetText();
                            //if(local_buf->sql.useColumn(7)) local_buf->sql.getValue(countRows, 7) = member->FirstChildElement("output-volume")->GetText();
                            countRows++;
                        }
                    }
                }
            }
            else
            {
                TagLoger::warn(Log_MySqlServer, 0, "conference error command `api conference xml_list` on server=%s\n", link->getId().data()); 
            }

            break;
        }
         
    }
    else
    {
        for(int i=0; i< MAX_EXPRESSIONS_VALUES; i++)
        {
            if(local_buf->qInfo.where.whereExprValue[idExprPos][i].isNull())
            {
                // Значения закончились
                break;
            }

            char* room_name = local_buf->qInfo.where.whereExprValue[idExprPos][i].Start(local_buf->qInfo);
            int nameLen = local_buf->qInfo.where.whereExprValue[idExprPos][i].tokLen;
            room_name[nameLen] = 0;
            if(nameLen < 1 || nameLen > PIPE_NAME_LEN || !AZ09test(room_name, nameLen))
            {
                continue;
            }

            TagLoger::log(Log_MySqlServer, 0, "dev_id=%d text>%s\n",dev_id, room_name);

            local_buf->stm.conference_select_nodes_for_room->execute(dev_id, room_name);
            while(!local_buf->stm.conference_select_nodes_for_room->fetch())
            {
                TagLoger::log(Log_MySqlServer, 0, "call for room_name>%s\n",room_name); 
                fs_esl *link;
                auto it = local_buf->fs_eslCluster.begin();
                while(it != local_buf->fs_eslCluster.end())
                {
                    link = *it;

                    TagLoger::warn(Log_MySqlServer, 0, "compare for room_name=%s node=%s and node=%s\n",room_name, link->getId().data(), local_buf->stm.conference_select_nodes_for_room->result_node);

                    if(link->getId().compare(local_buf->stm.conference_select_nodes_for_room->result_node) != 0)
                    {
                        it++;
                        continue;
                    }

                    link->exec("api conference xml_list\n\n");
                    if (link->getHandle().last_sr_event && link->getHandle().last_sr_event->body)
                    {
                        TagLoger::debug(Log_MySqlServer, 0, "command `api conference xml_list` body=%s\n", link->getHandle().last_sr_event->body); 

                        tinyxml2::XMLDocument xmlDoc;
                        xmlDoc.Parse(link->getHandle().last_sr_event->body, strlen(link->getHandle().last_sr_event->body));

                        tinyxml2::XMLElement *levelElement = xmlDoc.FirstChildElement();
                        if (levelElement == nullptr)
                        {
                            continue;
                        }

                        for (tinyxml2::XMLElement* child = levelElement->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
                        {
                            TagLoger::debug(Log_MySqlServer, 0, "conference name=%s, time=%s\n", child->Attribute("name"), child->Attribute("run_time"));

                            std::string room_dev_id;
                            std::string curent_room_name;
                            std::string room_mode;
                            const char* startpos =  child->Attribute("name");
 
                            int numberlen = strlen(startpos);
                            for(int k = 0; k< numberlen; k++)
                            {
                                if(startpos[k] == '*')
                                {
                                    room_dev_id.append(startpos, k);
                                    startpos += k + 1;
                                    break;
                                }
 
                            }

                            try{
                                 if(std::stoi(room_dev_id.data()) != dev_id)
                                 {
                                    continue;
                                 }
                            }catch(...)
                            {
                                TagLoger::error(Log_MySqlServer, 0, "\x1b[1;31mget_int exeption for room_dev_id=%s\x1b[0m\n", room_dev_id.data());
                                continue;
                            }

                            numberlen = strlen(startpos); 
                            for(int k = 0; k< numberlen; k++)
                            {
                                if(startpos[k] == '*' || startpos[k] == '-')
                                {
                                    curent_room_name.append(startpos, k);
                                    startpos += k + 1;
                                    break;
                                } 
                            }
 
                            if(curent_room_name.compare(room_name) != 0)
                            {
                                continue;
                            }

                            tinyxml2::XMLElement* members = child->FirstChildElement();
                            for (tinyxml2::XMLElement* member = members->FirstChildElement(); member != NULL; member = member->NextSiblingElement())
                            {
                                const char* type = member->Attribute("type");
                                if(type == nullptr)
                                {
                                    continue;
                                }
                                else if(strcmp(type, "recording_node") == 0)
                                {
                                    TagLoger::debug(Log_MySqlServer, 0, "\tmember type=%s, record_path=%s\n", type, member->FirstChildElement("record_path")->GetText());
                                }
                                else if(strcmp(type, "caller") == 0)
                                {
                                    TagLoger::debug(Log_MySqlServer, 0, "\tmember type=%s, id=%s, join_time=%s, last_talking=%s\n", type,
                                            member->FirstChildElement("id")->GetText(),
                                            member->FirstChildElement("join_time")->GetText(),
                                            member->FirstChildElement("last_talking")->GetText());
                                    
                                    int test_dev_id = 0;
                                    int test_user_id = 0;
                                    
                                    const char* caller_id_name = member->FirstChildElement("caller_id_name")->GetText();
                                    if(caller_id_name == NULL)
                                    {
                                        continue;
                                    }
                                    
                                    sscanf(caller_id_name, "%6d%d", &test_dev_id, &test_user_id);
                                    if(test_dev_id != dev_id)
                                    {
                                        continue;
                                    }
                                     

                                    if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = curent_room_name.data();
                                    //if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = room_mode;
                                    if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = test_user_id;
                                    if(local_buf->sql.useColumn(2)) local_buf->sql.getValue(countRows, 2) = member->FirstChildElement("join_time")->GetText();
                                    if(local_buf->sql.useColumn(3)) local_buf->sql.getValue(countRows, 3) = member->FirstChildElement("last_talking")->GetText();
                                    //if(local_buf->sql.useColumn(4)) local_buf->sql.getValue(countRows, 4) = member->FirstChildElement("energy")->GetText();
                                    //if(local_buf->sql.useColumn(5)) local_buf->sql.getValue(countRows, 5) = member->FirstChildElement("volume_in")->GetText();
                                    //if(local_buf->sql.useColumn(6)) local_buf->sql.getValue(countRows, 6) = member->FirstChildElement("volume_out")->GetText();
                                    //if(local_buf->sql.useColumn(7)) local_buf->sql.getValue(countRows, 7) = member->FirstChildElement("output-volume")->GetText();
                                    countRows++;
                                }
                            }
                        }
                    }
                    else
                    {
                        TagLoger::warn(Log_MySqlServer, 0, "conference error command `api conference xml_list` on server=%s\n", link->getId().data()); 
                    }

                    break;
                }
            }
            local_buf->stm.conference_select_nodes_for_room->free();
        }
    }
    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

int MySql_connection::sql_delete_from_conference_members(thread_data* local_buf, unsigned int PacketNomber)
{
    Send_Err_Package(SQL_ERR_READ_ONLY, "delete from `conference_members` is not ready yet", PacketNomber+1, local_buf, this);
    return 0;
}


int MySql_connection::sql_insert_into_dialogs(thread_data* local_buf, unsigned int PacketNomber)
{
    Send_Err_Package(SQL_ERR_READ_ONLY, "Table `dialogs` is not ready yet", PacketNomber+1, local_buf, this);
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

    char pakbuf[200];
    bzero(pakbuf, 200);

    char*p = pakbuf;

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

    int dataLen = p - pakbuf;
    TagLoger::log(Log_MySqlServer, 0, "itit len:%d\n", dataLen);

    int t = dataLen - 4;
    memcpy(pakbuf,  &t, 3); //  Длина пакета

    if(TagLoger::isLog(Log_MySqlServer, TAGLOG_DEBUG))
    {
        for(int i=0; i < dataLen; i++)
        {
            if(i % 8 == 0)  TagLoger::log(Log_MySqlServer, 0, "!\n%d:\t", i);
            char c = (char)pakbuf[i];
            if(c < 32 ) c = '.';

            TagLoger::log(Log_MySqlServer, 0, "!%02x[%c] ",  (unsigned char)pakbuf[i], c);
        }
        TagLoger::log(Log_MySqlServer, 0, "!\n ");
    }

    clientState = STATE_SEND_HANDSHAKE;
    TagLoger::log(Log_MySqlServer, 0, "Connection will not be closed [4] [MySql_connection]\n");
    return web_write(pakbuf, dataLen);
}

int MySql_connection::set_offline(thread_data* local_buf)
{
    if(!isOnLine)
    {
        return web_close();
    }

    TagLoger::log(Log_MySqlServer, 0, " >MySql_connection::set_online false dev_id=%d\n", dev_id);

    devManager::instance()->getDevInfo(dev_id)->decrBackendOnline();
    dev_id = 0;
    api_version = 0;
    start_online_time = 0;
    cometqlcluster = 0;
     
    isAuthUser = false;

    isOnLine = false;
    return web_close();
}

/**
  * Хочу научится летать на самолёте
  * Играть в пентбол
  * Квадрокоптер с камерой
  * 3D Принтер
  * "We can fly"
  */
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
