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
#include "CometQLProxy_connection.h"

#include "CometQL.h"
#include "y.tab.h"
#include "sha1.h"

#include "devManager.h"
#include <ctime>

#include <openssl/sha.h>

#include "sha1.h"

// ; @todo Сделать агрегацию данных под каждый sql select запрос
// ; @todo Написать тесты на работу в кластере
// ; @todo Не транслировать ошибку если хоть одна из нод запрос обработатала


CometQLProxy_connection::CometQLProxy_connection():MySql_connection()
{
    //printf("create CometQLProxy_connection\n");
}

CometQLProxy_connection::~CometQLProxy_connection()
{
    //printf("delete CometQLProxy_connection\n");
}

int CometQLProxy_connection::proxy_query(int node, thread_data* local_buf, unsigned int PacketNomber)
{
    if(local_buf->qInfo.command == TOK_SELECT || local_buf->qInfo.command == TOK_SHOW)
    {
        return proxy_union_select(node, local_buf->qInfo.StartQury, local_buf, PacketNomber);
    }
    else if(local_buf->qInfo.command == TOK_INSERT || local_buf->qInfo.command == TOK_DELETE)
    {
        return proxy_insert(node, local_buf->qInfo.StartQury, local_buf, PacketNomber);
    }
}

/**
 * Выполняет union select на нодах кластера
 * @param node
 * @param query
 * @param local_buf
 * @param PacketNomber
 * @return
 */
int CometQLProxy_connection::proxy_union_select(int node, const char* query, thread_data* local_buf, unsigned int PacketNomber)
{
    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();

    bool isSendHeader = false;
    int num_fields = 0;
    MySqlResulValue value[MAX_COLUMNS_COUNT];
    MYSQL_ROW row;

    dbLink *link;
    if(node != PROXY_TO_ALL)
    {
        if(node == PROXY_TO_RANDOM)
        {
            // Не задана node выполнить запрос на случайной.
            node = random() % local_buf->proxyCluster.size();

            TagLoger::log(Log_CometQLCluster, 0, "CometQLProxy query:`%s` send to node=%d from %d\n", query, node, local_buf->proxyCluster.size());
            // Если задана node то выполнить запрос на конкретной ноде а не на всех нодах.
            link = local_buf->proxyCluster[node];
            if(!link->query(query))
            {
                // @todo Проверять код ошибки и не паниковать если по коду ясно что проблема в самом запросе а не соединении.
                // @todo Проверять что если ошибка сетевая или что то ещё то повторять попытку.
                TagLoger::error(Log_CometQLCluster, 0, "CometQLProxy warn[6] on node=%s in query=%s\n", link->name(), query);

                bool isSuccess = false;
                // Если ошибка то ещё раз отправим запрос но на другую ноду
                auto it = local_buf->proxyCluster.begin();
                while(it != local_buf->proxyCluster.end())
                {
                    link = *it;
                    if(link->query(query))
                    {
                        isSuccess = true;
                        break;
                    }
                    // @todo Проверять код ошибки и не паниковать если по коду ясно что проблема в самом запросе а не соединении.
                    TagLoger::warn(Log_CometQLCluster, 0, "CometQLProxy warn[9] on node=%s in query=%s\n", link->name(), query);
                    it++;
                }

                if(!link)
                {
                    TagLoger::error(Log_CometQLCluster, 0, "CometQLProxy error[10] on node=PROXY_TO_ALL(%d) in query=%s\n", node, query);
                    local_buf->qInfo.setError("Error[1] in node link", SQL_ERR_INTERNAL_SERVER);
                    local_buf->answer_buf.unlock();
                    Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, ++PacketNomber, local_buf, this);
                    return 0;
                }

                if(!isSuccess)
                {
                    local_buf->qInfo.setError(mysql_error(link->getLink()), mysql_errno(link->getLink()));
                    local_buf->answer_buf.unlock();
                    Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, ++PacketNomber, local_buf, this);
                    return 0;
                }
            }
        }
        else
        {
            TagLoger::log(Log_CometQLCluster, 0, "CometQLProxy query:`%s` send to node=%d from %d\n", query, node, local_buf->proxyCluster.size());
            // Если задана node то выполнить запрос на конкретной ноде а не на всех нодах.
            link = local_buf->proxyCluster[node];
            if(!link->query(query))
            {
                // @todo Проверять код ошибки и не паниковать если по коду ясно что проблема в самом запросе а не соединении.
                // @todo Проверять что если ошибка сетевая или что то ещё то повторять попытку.
                TagLoger::error(Log_CometQLCluster, 0, "CometQLProxy error[6] on node=%d in query=%s\n", node, query);
                local_buf->qInfo.setError(mysql_error(link->getLink()), mysql_errno(link->getLink()));
                local_buf->answer_buf.unlock();
                Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, ++PacketNomber, local_buf, this);
                return 0;
            }
        }

        auto result = mysql_store_result(link->getLink());

        MYSQL_FIELD *field;
        while((field = mysql_fetch_field(result)))
        {
            printf("field name %s\n", field->name);
            local_buf->sql.columns[num_fields] = field->name;
            num_fields++;
            if(num_fields >= MAX_COLUMNS_COUNT)
            {
                break;
            }
        }

        answer += HeadAnswer(num_fields, local_buf->sql.columns, PacketNomber, answer);
        isSendHeader = true;

        while((row = mysql_fetch_row(result)))
        {
            for(int i = 0; i < num_fields; i++)
            {
                printf("[%s]", row[i] ? row[i] : "NULL");
                value[i] = row[i];
            }
            printf("\n");
            answer += RowPackage(num_fields, value, ++PacketNomber, answer);
        }
        mysql_free_result(result);
    }
    else
    {
        auto it = local_buf->proxyCluster.begin();

        dbLink* link;
        bool isSuccess = false;
        while(it != local_buf->proxyCluster.end())
        {
            link = *it;

            if(!link->query(query))
            {
                // @todo Не паниковать если ошибка на одной ноде из нескольких, и не рубить из за этого весь запрос.
                // @todo Проверять код ошибки и не паниковать если по коду ясно что проблема в самом запросе а не соединении.
                TagLoger::error(Log_CometQLCluster, 0, "CometQLProxy warn[11] on node=%s in query=%s\n", link->name(), query);
                it++;
                continue;
            }

            isSuccess = true;
            auto result = mysql_store_result(link->getLink());

            if(!isSendHeader)
            {
                MYSQL_FIELD *field;
                while((field = mysql_fetch_field(result)))
                {
                    printf("field name %s\n", field->name);
                    local_buf->sql.columns[num_fields] = field->name;
                    num_fields++;
                    if(num_fields >= MAX_COLUMNS_COUNT)
                    {
                        break;
                    }
                }

                answer += HeadAnswer(num_fields, local_buf->sql.columns, PacketNomber, answer);
                isSendHeader = true;
            }

            while((row = mysql_fetch_row(result)))
            {
                for(int i = 0; i < num_fields; i++)
                {
                    printf("[%s]", row[i] ? row[i] : "NULL");
                    value[i] = row[i];
                }
                printf("\n");
                answer += RowPackage(num_fields, value, ++PacketNomber, answer);
            }
            mysql_free_result(result);
            it++;
        }

        if(!link)
        {
            TagLoger::error(Log_CometQLCluster, 0, "CometQLProxy error[10] on node=PROXY_TO_ALL(%d) in query=%s\n", node, query);
            local_buf->qInfo.setError("Error[1] in node link", SQL_ERR_INTERNAL_SERVER);
            local_buf->answer_buf.unlock();
            Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, ++PacketNomber, local_buf, this);
            return 0;
        }
        
        if(!isSuccess)
        {
            TagLoger::error(Log_CometQLCluster, 0, "CometQLProxy error[5] on node=PROXY_TO_ALL(%d) in query=%s\n", node, query);
            local_buf->qInfo.setError(mysql_error(link->getLink()), mysql_errno(link->getLink()));
            local_buf->answer_buf.unlock();
            Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, ++PacketNomber, local_buf, this);
            return 0;
        }
    }


    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());
    local_buf->answer_buf.unlock();

    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package
    return 0;
}

/**
 * @todo Надо добавлять к запросу uuid чтоб потом на клиенте не приходило дубликоатов если кластер будет работать на более чем одном коннекте от клиента
 * @param node
 * @param query
 * @param local_buf
 * @param PacketNomber
 * @return 0
 */
int CometQLProxy_connection::proxy_insert(int node, const char* query, thread_data* local_buf, unsigned int PacketNomber)
{
    /**
     * Для операций удаления affectedRows возвращатся не будет в целях оптимизации.
     */
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this);

    if(node != PROXY_TO_ALL)
    {
        if(node == PROXY_TO_RANDOM)
        {
            // Не задана node выполнить запрос на случайной.
            node = random() % local_buf->proxyCluster.size();
            auto link = local_buf->proxyCluster[node];
            if(!link->query(query))
            {
                // @todo Проверять код ошибки и не паниковать если по коду ясно что проблема в самом запросе а не соединении.
                TagLoger::warn(Log_CometQLCluster, 0, "CometQLProxy error[1] on node=%s in query=%s\n", link->name(), query);
                // Если ошибка то ещё раз отправим запрос но на другую ноду
                auto it = local_buf->proxyCluster.begin();
                while(it != local_buf->proxyCluster.end())
                {
                    if((*it)->query(query))
                    {
                        break;
                    }
                    // @todo Проверять код ошибки и не паниковать если по коду ясно что проблема в самом запросе а не соединении.
                    TagLoger::warn(Log_CometQLCluster, 0, "CometQLProxy error[2] on node=%s in query=%s\n", link->name(), query);
                    it++;
                }
            }
        }
        else
        {
            // задана node то выполнить запрос на конкретной ноде
            auto link = local_buf->proxyCluster[node];

            if(!link->query(query))
            {
                // Если ошибка то не повезло
                // @todo Проверять код ошибки и не паниковать если по коду ясно что проблема в самом запросе а не соединении.
                TagLoger::error(Log_CometQLCluster, 0, "CometQLProxy error[3] on node=%s in query=%s\n", link->name(), query);
            }
        }

        return 0;
    }

    auto it = local_buf->proxyCluster.begin();
    while(it != local_buf->proxyCluster.end())
    {
        auto link = *it;

        if(!link->query(query))
        {
            // @todo Проверять код ошибки и не паниковать если по коду ясно что проблема в самом запросе а не соединении.
            TagLoger::error(Log_CometQLCluster, 0, "CometQLProxy error[4] on node=%s in query=%s\n", link->name(), query);
        }
        it++;
    }
    return 0;
}

int CometQLProxy_connection::query_router(thread_data* local_buf, int PacketNomber)
{
    if(local_buf->proxyCluster.size() == 0)
    {
        TagLoger::error(Log_CometQLCluster, 0, "CometQLProxy error[7] Cluster.size=%d in query=%s\n", local_buf->proxyCluster.size(), local_buf->qInfo.StartQury);
        local_buf->qInfo.setError("CometQLProxy error Cluster.size=0", SQL_ERR_INTERNAL_SERVER);
        local_buf->answer_buf.unlock();
        Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, ++PacketNomber, local_buf, this);
        return -1;
    }

    if(local_buf->qInfo.command == TOK_SHOW)
    {
        TagLoger::log(Log_CometQLCluster, 0, "CometQLProxy cmd show:%d\n", local_buf->qInfo.arg_show.command);
        if(local_buf->qInfo.arg_show.command == TOK_DATABASES)
        {
            return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_show_databases(local_buf,PacketNomber);
        }
        else if(test_api_version(local_buf,PacketNomber))
        {
            if(local_buf->qInfo.arg_show.command == TOK_TABLES)
            {
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); //return sql_show_tables(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_COLUMNS)
            {
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); //return sql_show_columns(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_STATUS)
            {
                return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); //return sql_show_status(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_PROCESSLIST)
            {
                return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); //return sql_show_processlist(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_VARIABLES)
            {
                return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); //return sql_show_variables(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_TABLE_STATUS)
            {
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); //return sql_show_table_status(local_buf,PacketNomber);
            }
        }

        // [Почти готов] SHOW SESSION VARIABLES LIKE 'lower_case_table_names'
        // SELECT current_user()
        // show create table `users`
    }
    else if(local_buf->qInfo.command == TOK_SELECT)
    {
        TagLoger::log(Log_CometQLCluster, 0, "cmd select:%d\n", local_buf->qInfo.arg_select.command);

        if(local_buf->qInfo.arg_select.command == TOK_DATABASE)
        {
            return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); //return sql_select_database_name(local_buf,PacketNomber);
        }
        else if(local_buf->qInfo.arg_select.command == VAL_SYSTEM_VARIBLE)
        {
            return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); //return sql_select_systemvarible(local_buf,PacketNomber);
        }
        else if(test_api_version(local_buf,PacketNomber))
        {
            if(local_buf->qInfo.arg_select.command == TOK_FROM)
            {
                if(local_buf->qInfo.tokCompare("users_auth",  local_buf->qInfo.tableName))
                {
                    return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); //return sql_select_from_users_auth(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
                {
                    if(appConf::instance()->get_bool("main", "save_users_last_online_time"))
                    {
                        // Будет работать корректно если [main][save_users_last_online_time] = true
                        return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber);
                    }

                    // Если не равно то работать не будет так как данные об онлайне могут не оказаться на нужной ноде.
                    // (Минимум с нод)
                    return sql_select_from_users_time(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
                {
                    return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_select_from_users_messages(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
                {
                    return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_select_from_pipes_messages(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
                {
                    return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); // return sql_select_from_users_in_pipes(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
                {
                    // (Сумма с нод)
                    return sql_select_from_pipes(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
                {
                    return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_select_from_pipes_settings(local_buf,PacketNomber);
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
                // @todo выбор от user_id
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_insert_into_users_auth(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
            {
                // @todo выбор от user_id
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_insert_into_users_time(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
            {
                // @todo выбор от user_id
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_insert_into_users_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
            {
                return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); // return sql_insert_into_pipes_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
            {
                return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); // return sql_insert_into_users_in_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
            {
                return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); // return sql_insert_into_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
            {
                return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); // return sql_insert_into_pipes_settings(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("conference",  local_buf->qInfo.tableName))
            {
                return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); // return sql_insert_into_conference(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("dialogs",  local_buf->qInfo.tableName))
            {
                return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); // return sql_insert_into_dialogs(local_buf,PacketNomber);
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
                // @todo выбор от user_id
                return proxy_query(PROXY_TO_ALL, local_buf,PacketNomber); // return sql_delete_from_users_auth(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
            {
                // @todo выбор от user_id
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_delete_from_users_time(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
            {
                // @todo выбор от user_id
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_delete_from_users_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
            {
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_delete_from_pipes_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
            {
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_delete_from_users_in_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
            {
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_delete_from_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
            {
                return proxy_query(PROXY_TO_RANDOM, local_buf,PacketNomber); // return sql_delete_from_pipes_settings(local_buf,PacketNomber);
            }
            else
            {
                return Send_Err_Package(SQL_ERR_NOT_EXIST, "Table doesn't exist", PacketNomber+1, local_buf, this);
            }
        }
    }
    else
    {
        TagLoger::log(Log_CometQLCluster, 0, "cmd undefined:%d %s\n", local_buf->qInfo.arg_select.command, local_buf->qInfo.StartQury);
        return Send_OK_Package(PacketNomber+1, local_buf, this);
    }

    return 0;
}

// pipes
int CometQLProxy_connection::sql_select_from_pipes(thread_data* local_buf, unsigned int PacketNomber)
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

    MYSQL_ROW row;
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

        int pipe_size = 0;
        auto it = local_buf->proxyCluster.begin();
        while(it != local_buf->proxyCluster.end())
        {
            auto link = *it;
            if(!link->query_format("SELECT users FROM pipes WHERE name = \"%s\" ", pipe_name))
            {
                it++;
                continue; 
            }

            auto result = mysql_store_result(link->getLink());
            while((row = mysql_fetch_row(result)))
            {
                long int_size = 0;
                if(row[0] == NULL)
                {
                    continue;
                }

                try{
                    //printf("get_long [%s] %s=%s\n", section.data(), name.data(), sections.at(section).at(name).data());
                    int_size = std::stoi(row[0]);
                }catch(...)
                {
                    continue;
                }

                pipe_size += int_size;
            }
            mysql_free_result(result);
            it++;
        }

        TagLoger::log(Log_MySqlServer, 0, "text>%s\n",pipe_name);
        if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = pipe_name;
        if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = pipe_size;

        countRows++;
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}

// users_time
int CometQLProxy_connection::sql_select_from_users_time(thread_data* local_buf, unsigned int PacketNomber)
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

    MYSQL_ROW row;
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

        long min_time = -1;
        auto it = local_buf->proxyCluster.begin();
        while(it != local_buf->proxyCluster.end())
        {
            auto link = *it;
            if(!link->query_format("SELECT time FROM users_time WHERE name = %d ", userId))
            {
                it++; 
                continue;
            }

            auto result = mysql_store_result(link->getLink());
            while((row = mysql_fetch_row(result)))
            {
                long time = 0;
                if(row[0] == NULL)
                {
                    continue;
                }

                try{
                    //printf("get_long [%s] %s=%s\n", section.data(), name.data(), sections.at(section).at(name).data());
                    time = std::stol(row[0]);
                }catch(...)
                {
                    continue;
                }

                if(min_time > time)
                {
                    min_time = time;
                }
            }
            mysql_free_result(result);
            it++;
        }

        if(local_buf->sql.useColumn(0)) local_buf->sql.getValue(countRows, 0) = userId;
        if(local_buf->sql.useColumn(1)) local_buf->sql.getValue(countRows, 1) = min_time;

        countRows++;
    }

    local_buf->sql.sendAllRowsAndHeaders(local_buf, PacketNomber, countRows, this);
    return 0;
}
