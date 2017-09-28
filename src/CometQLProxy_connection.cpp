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
#include "CometQLcluster.h"
#include "devManager.h"
#include <ctime>

#include <openssl/sha.h>

#include "sha1.h"

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
        return proxy_select(node, local_buf->qInfo.StartQury, local_buf, PacketNomber);
    }
    else if(local_buf->qInfo.command == TOK_INSERT || local_buf->qInfo.command == TOK_DELETE)
    {
        return proxy_insert(node, local_buf->qInfo.StartQury, local_buf, PacketNomber);
    }
}

int CometQLProxy_connection::proxy_select(int node, const char* query, thread_data* local_buf, unsigned int PacketNomber)
{
    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();

    auto it = local_buf->cometCluster.begin();
    
    bool isSendHeader = false;
    int num_fields = 0;
    MySqlResulValue value[MAX_COLUMNS_COUNT];
    MYSQL_ROW row; 
    
    if(node != -1)
    {
        if(node < 0)
        {
            node = local_buf->cometCluster.size() % random();
        }
        
        // Если задана node то выполнить запрос на конкретной ноде а не на всех нодах.
        auto link = local_buf->cometCluster[node];
        if(!link->query(query))
        {
            // @todo Проверять что если ошибка сетевая или что то ещё то повторять попытку.
            local_buf->qInfo.setError(mysql_error(link->getLink()), mysql_errno(link->getLink()));
            Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, ++PacketNomber, local_buf, this);
            return 0;
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

        answer += HeadAnswer(num_fields, local_buf->sql.columns, ++PacketNomber, answer);
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
    }
    else
    {
        while(it != local_buf->cometCluster.end())
        {
            auto link = *it;

            if(!link->query(query))
            {
                local_buf->qInfo.setError(mysql_error(link->getLink()), mysql_errno(link->getLink()));
                Send_Err_Package(local_buf->qInfo.errorCode, local_buf->qInfo.errorText, ++PacketNomber, local_buf, this);
                return 0;
            }

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

                answer += HeadAnswer(num_fields, local_buf->sql.columns, ++PacketNomber, answer);
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
        }
    }
    
    
    web_write(local_buf->answer_buf.getData(), answer - local_buf->answer_buf.getData());
    local_buf->answer_buf.unlock();

    Send_EOF_Package(++PacketNomber, local_buf, this); // Send_EOF_Package
    return 0;
}

int CometQLProxy_connection::proxy_insert(int node, const char* query, thread_data* local_buf, unsigned int PacketNomber)
{
    /**
     * Для операций удаления affectedRows возвращатся не будет в целях оптимизации.
     */
    Send_OK_Package(0, 0, PacketNomber+1, local_buf, this); 
    
    if(node != -1)
    {
        if(node < 0)
        {
            node = local_buf->cometCluster.size() % random();
        }
        
        // Если задана node то выполнить запрос на конкретной ноде а не на всех нодах.
        auto link = local_buf->cometCluster[node];
        
        // @todo Проверять что если ошибка сетевая или что то ещё то повторять попытку.
        link->query(query);
        return 0;
    }
    
    auto it = local_buf->cometCluster.begin(); 
    while(it != local_buf->cometCluster.end())
    {
        auto link = *it;
        
        // @todo Проверять что если ошибка сетевая или что то ещё то повторять попытку.
        link->query(query);
    } 
    return 0;
}
 
int CometQLProxy_connection::query_router(thread_data* local_buf, int PacketNomber)
{
    if(local_buf->qInfo.command == TOK_SHOW)
    {
        TagLoger::log(Log_MySqlServer, 0, "cmd show:%d\n", local_buf->qInfo.arg_show.command);
        if(local_buf->qInfo.arg_show.command == TOK_DATABASES)
        {
            return proxy_query(-2, local_buf,PacketNomber); // return sql_show_databases(local_buf,PacketNomber);
        }
        else if(test_api_version(local_buf,PacketNomber))
        {
            if(local_buf->qInfo.arg_show.command == TOK_TABLES)
            {
                return proxy_query(-2, local_buf,PacketNomber); //return sql_show_tables(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_COLUMNS)
            {
                return proxy_query(-2, local_buf,PacketNomber); //return sql_show_columns(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_STATUS)
            {
                return proxy_query(-1, local_buf,PacketNomber); //return sql_show_status(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_PROCESSLIST)
            {
                return proxy_query(-1, local_buf,PacketNomber); //return sql_show_processlist(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_VARIABLES)
            {
                return proxy_query(-1, local_buf,PacketNomber); //return sql_show_variables(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.arg_show.command == TOK_TABLE_STATUS)
            {
                return proxy_query(-1, local_buf,PacketNomber); //return sql_show_table_status(local_buf,PacketNomber);
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
            return proxy_query(-1, local_buf,PacketNomber); //return sql_select_database_name(local_buf,PacketNomber);
        }
        else if(local_buf->qInfo.arg_select.command == VAL_SYSTEM_VARIBLE)
        {
            return proxy_query(-1, local_buf,PacketNomber); //return sql_select_systemvarible(local_buf,PacketNomber);
        }
        else if(test_api_version(local_buf,PacketNomber))
        {
            if(local_buf->qInfo.arg_select.command == TOK_FROM)
            {
                if(local_buf->qInfo.tokCompare("users_auth",  local_buf->qInfo.tableName))
                {
                    // @todo выбор от user_id
                    return proxy_query(-2, local_buf,PacketNomber); //return sql_select_from_users_auth(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
                {
                    // @todo выбор от user_id
                    return proxy_query(-2, local_buf,PacketNomber); // return sql_select_from_users_time(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
                {
                    // @todo выбор от user_id
                    return proxy_query(-2, local_buf,PacketNomber); // return sql_select_from_users_messages(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
                {
                    return proxy_query(-1, local_buf,PacketNomber); // return sql_select_from_pipes_messages(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
                {
                    return proxy_query(-1, local_buf,PacketNomber); // return sql_select_from_users_in_pipes(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
                {
                    return proxy_query(-1, local_buf,PacketNomber); // return sql_select_from_pipes(local_buf,PacketNomber);
                }
                else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
                {
                    return proxy_query(-1, local_buf,PacketNomber); // return sql_select_from_pipes_settings(local_buf,PacketNomber);
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
                return proxy_query(-2, local_buf,PacketNomber); // return sql_insert_into_users_auth(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
            {
                // @todo выбор от user_id
                return proxy_query(-2, local_buf,PacketNomber); // return sql_insert_into_users_time(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
            {
                // @todo выбор от user_id
                return proxy_query(-2, local_buf,PacketNomber); // return sql_insert_into_users_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
            {
                return proxy_query(-1, local_buf,PacketNomber); // return sql_insert_into_pipes_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
            {
                return proxy_query(-1, local_buf,PacketNomber); // return sql_insert_into_users_in_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
            {
                return proxy_query(-1, local_buf,PacketNomber); // return sql_insert_into_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
            {
                return proxy_query(-1, local_buf,PacketNomber); // return sql_insert_into_pipes_settings(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("conference",  local_buf->qInfo.tableName))
            {
                return proxy_query(-1, local_buf,PacketNomber); // return sql_insert_into_conference(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("dialogs",  local_buf->qInfo.tableName))
            {
                return proxy_query(-1, local_buf,PacketNomber); // return sql_insert_into_dialogs(local_buf,PacketNomber);
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
                return proxy_query(-1, local_buf,PacketNomber); // return sql_delete_from_users_auth(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_time",  local_buf->qInfo.tableName))
            {
                // @todo выбор от user_id
                return proxy_query(-2, local_buf,PacketNomber); // return sql_delete_from_users_time(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_messages",  local_buf->qInfo.tableName))
            {
                // @todo выбор от user_id
                return proxy_query(-2, local_buf,PacketNomber); // return sql_delete_from_users_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_messages",  local_buf->qInfo.tableName))
            {
                return proxy_query(-2, local_buf,PacketNomber); // return sql_delete_from_pipes_messages(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("users_in_pipes",  local_buf->qInfo.tableName))
            {
                return proxy_query(-2, local_buf,PacketNomber); // return sql_delete_from_users_in_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes",  local_buf->qInfo.tableName))
            {
                return proxy_query(-2, local_buf,PacketNomber); // return sql_delete_from_pipes(local_buf,PacketNomber);
            }
            else if(local_buf->qInfo.tokCompare("pipes_settings",  local_buf->qInfo.tableName))
            {
                return proxy_query(-2, local_buf,PacketNomber); // return sql_delete_from_pipes_settings(local_buf,PacketNomber);
            }
            else
            {
                return Send_Err_Package(SQL_ERR_NOT_EXIST, "Table doesn't exist", PacketNomber+1, local_buf, this);
            }
        }
    }
    else
    {
        TagLoger::log(Log_MySqlServer, 0, "cmd undefined:%d %s\n", local_buf->qInfo.arg_select.command, local_buf->qInfo.StartQury);
        return Send_OK_Package(PacketNomber+1, local_buf, this);
    }
    
    return 0;
}