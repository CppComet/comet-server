// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "internalApi.h"

#include "mystring.h"
#include "user_index.h"
#include "Client_connection.h"
#include "devManager.h"

int internalApi::send_to_user(thread_data* local_buf, int dev_id, int user_id, const char* pipe_event, const char* msg_data)
{
    char server_data[EVENT_NAME_LEN+64];
    snprintf(server_data, EVENT_NAME_LEN+64, "\"event_name\":\"%s\",\"SendToUser\":true", pipe_event);

    return send_to_user(local_buf, dev_id, user_id, pipe_event, msg_data, server_data);
}

int internalApi::send_to_user(thread_data* local_buf, int dev_id, int user_id, const char* pipe_event, const char* msg_data, const char* server_data)
{
    int *conection_id = devManager::instance()->getDevInfo(dev_id)->index->get_conection_id(local_buf, user_id);
    int isSend = -1;
    if(conection_id != NULL)
    {
        for(int i=0; i< MAX_CONECTION_ON_USER_ID; i++)
        {
            if( conection_id[i] < 1 )
            {
                continue;
            }

            CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(conection_id[i]);
            if(r && r->web_user_dev_id == dev_id)
            {
                isSend = r->message(local_buf, msg_data, NULL, MESSAGE_TEXT, server_data);
                TagLoger::log(Log_MySqlServer, 0, "Connection object is found i=%d, conection_id=%d\n", i, conection_id[i]);
            }
            else
            {
                TagLoger::log(Log_MySqlServer, 0, "Connection object is not found i=%d, conection_id=%d\n", i, conection_id[i]);
            }
        }
    }
    else
    {
        TagLoger::log(Log_MySqlServer, 0, "Connection ID not found for dev_id=%d, user_id=%d\n", dev_id, user_id);
    }

    if(isSend != 0)
    {
        char uid[37];
        bzero(uid, 37);
        uuid37(uid);

        local_buf->stm.users_queue_insert->execute(uid, (long int)time(NULL), dev_id, user_id, pipe_event, msg_data, strlen(msg_data));
    }
    return isSend;
}

int internalApi::local_send_to_user(thread_data* local_buf, int dev_id, int user_id, const char* pipe_event, const char* msg_data)
{
    //char server_data[EVENT_NAME_LEN+64];
    //snprintf(server_data, EVENT_NAME_LEN+64, "\"event_name\":\"%s\",\"SendToUser\":true", pipe_event);
    std::string server_data; 
    server_data.append("\"event_name\":\"").append(pipe_event).append("\",\"SendToUser\":true");

    int *conection_id = devManager::instance()->getDevInfo(dev_id)->index->get_conection_id(local_buf, user_id);
    int isSend = -1;
    if(conection_id != NULL)
    {
        for(int i=0; i< MAX_CONECTION_ON_USER_ID; i++)
        {
            if( conection_id[i] < 1 )
            {
                continue;
            }

            CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(conection_id[i]);
            if(r && r->web_user_dev_id == dev_id)
            {
                if(r->message(local_buf, msg_data, NULL, MESSAGE_TEXT, server_data.data()) != -1)
                {
                    isSend = 0;
                }
                TagLoger::log(Log_MySqlServer, 0, "Connection object is found i=%d, conection_id=%d\n", i, conection_id[i]);
            }
            else
            {
                TagLoger::log(Log_MySqlServer, 0, "Connection object is not found i=%d, conection_id=%d\n", i, conection_id[i]);
            }
        }
    }
    else
    {
        TagLoger::log(Log_MySqlServer, 0, "Connection ID not found for dev_id=%d, user_id=%d\n", dev_id, user_id);
    }
 
    return isSend;
}

/**  
 * @param local_buf
 * @param dev_id
 * @param user_id
 * @param pipe_event
 * @param msg_data
 * @return 
 */
int internalApi::cluster_send_to_user(thread_data* local_buf, int dev_id, int user_id, const char* pipe_event, const char* msg_data)
{
    char uuid[37];
    bzero(uuid, 37);
    uuid37(uuid);

    int needSave = local_send_to_user(local_buf, dev_id, user_id, pipe_event, msg_data);
     
    if(local_buf->isWSClusterActive())
    {
        auto it = local_buf->wsCluster.begin();
        while(it != local_buf->wsCluster.end())
        {
            auto link = *it;

            // @todo Проверять что если ошибка сетевая или что то ещё то повторять попытку.
            if(link->query_format("cometqlcluster_v1 set dev_id=%d; INSERT INTO users_messages (uuid, id, event, message)VALUES('%s', '%d', %s', '%s');",
                    dev_id,
                    uuid,
                    user_id,
                    pipe_event,
                    msg_data))
            {  
                if(mysql_insert_id((MYSQL*)link) > 0)
                {
                    needSave = 0;
                } 
            }
            it++;
        }

        local_buf->answer_buf.unlock();
    }

    if(needSave == 0)
    {
        local_buf->stm.users_queue_insert->execute(uuid, (long int)time(NULL), dev_id, user_id, pipe_event, msg_data, strlen(msg_data));
    }
    return 0;
}

/**
 * Отправляет событие в канал
 * @param local_buf
 * @param pipe_name
 * @param msg_data
 * @param dev_id
 * @return
 */
int internalApi::send_event_to_pipe(thread_data* local_buf, const char* pipe_name, const char* msg_data, int dev_id, const char* server_data)
{

    CP<Pipe> pipe = devManager::instance()->getDevInfo(dev_id)->findPipe(std::string(pipe_name));

    if(pipe.isNULL())
    {
        // internal server error
        TagLoger::log(Log_Any, 0, "pipe[%s] == NULL\n",pipe_name);
        return 0;
    }
    else if(pipe->empty() )
    {
        // internal server error
        TagLoger::log(Log_Any, 0, "pipe[%s] == empty\n",pipe_name);
        return 0;
    }

    int count_connection = pipe->size();
    TagLoger::log(Log_Any, 0, "pipe[%s] num_users = %d\n",pipe_name, count_connection);

    int num_fail = 0;
    int num_msg = 0;
    auto it = pipe->subscribers->begin();
    while(it)
    {
        CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(it->data);
        if(r && r->web_user_dev_id == dev_id)
        {
            num_msg++;
            int res = r->message(local_buf, msg_data, pipe_name, MESSAGE_TEXT, server_data);
            TagLoger::log(Log_Any, 0, "R->message = %d\n" , res);
        }
        else
        {
            pipe->erase(it->data);
            num_fail++;
        }

        it = it->Next();
    }

    return num_msg;
}
