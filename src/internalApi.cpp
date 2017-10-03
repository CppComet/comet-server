// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "internalApi.h"

#include "mystring.h"
#include "user_index.h"
#include "Client_connection.h" 
#include "devManager.h"

int internalApi::send_to_user(thread_data* local_buf, int user_id, const char* pipe_event, const char* msg_data)
{  
    char server_data[EVENT_NAME_LEN+64];
    snprintf(server_data, EVENT_NAME_LEN+64, "\"event_name\":\"%s\",\"SendToUser\":true", pipe_event);

    return send_to_user(local_buf, user_id, pipe_event, msg_data, server_data);
}

int internalApi::send_to_user(thread_data* local_buf, int user_id, const char* pipe_event, const char* msg_data, const char* server_data)
{ 
    int *conection_id = devManager::instance()->getDevInfo()->index->get_conection_id(local_buf, user_id);
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
            if(r)
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
        TagLoger::log(Log_MySqlServer, 0, "Connection ID not found\n");
    }
 
    if(isSend != 0)
    { 
        char uid[37];
        bzero(uid, 37);
        uid37(uid);

        local_buf->stm.users_queue_insert.execute(uid, (long int)time(NULL), user_id, pipe_event, msg_data, strlen(msg_data));
    } 
    return isSend;
}

/**
 * Отправляет событие в канал
 * @param local_buf
 * @param pipe_name
 * @param msg_data
 * @return
 */
int internalApi::send_event_to_pipe(thread_data* local_buf, const char* pipe_name, const char* msg_data, const char* server_data)
{

    CP<Pipe> pipe = devManager::instance()->getDevInfo()->findPipe(std::string(pipe_name));

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
        if(r)
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
