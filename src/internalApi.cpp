// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "internalApi.h"

#include "mystring.h"
#include "user_index.h"
#include "Client_connection.h"
#include "devManager.h"
  
int internalApi::local_send_to_user(thread_data* local_buf, int dev_id, int user_id, nlohmann::json jmessage)
{
    auto t = TagTimer::mtime();
    local_buf->setThreadStatus('l');
    
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
                if(r->message(local_buf, jmessage) != -1)
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
        TagLoger::log(Log_MySqlServer, 0, "[local_send_to_user]Connection ID not found for dev_id=%d, user_id=%d\n", dev_id, user_id);
    }

    TagTimer::add("internalApi::local_send_to_user", t);
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
int internalApi::cluster_send_to_user(thread_data* local_buf, int dev_id, int user_id, nlohmann::json jmessage)
{
    auto t = TagTimer::mtime();
    local_buf->setThreadStatus('s');
    char uuid[37];
    bzero(uuid, 37);
    uuid37(uuid);

    int needSave = local_send_to_user(local_buf, dev_id, user_id, jmessage);

    if(local_buf->isWSClusterActive())
    {
        auto it = local_buf->wsCluster.begin();
        while(it != local_buf->wsCluster.end())
        {
            auto link = *it;

            std::string event = jmessage["event"].get<std::string>();
            std::string msg_data = jmessage["data"].get<std::string>();

            TagLoger::log(Log_MySqlServer, 0, "[cluster_send_to_user]cometqlcluster_v1 name=%s, dev_id=%d, user_id=%d\n", link->name(), dev_id, user_id);
            
            // @todo Проверять что если ошибка сетевая или что то ещё то повторять попытку.
            if(link->query_format("cometqlcluster_v1 set dev_id=%d; INSERT INTO users_messages (uuid, id, event, message)VALUES('%s', '%d', %s', '%s');",
                    dev_id,
                    uuid,
                    user_id,
                    event.data(),
                    msg_data.data()))
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

    TagLoger::log(Log_MySqlServer, 0, "[cluster_send_to_user]needSave=%d, dev_id=%d, user_id=%d\n", needSave, dev_id, user_id);
    if(needSave != 0)
    {
        std::string event = jmessage["event"].get<std::string>();
        
        std::string type_name(jmessage["data"].type_name());
        std::string msg_data;
        if(type_name.compare("string") == 0)
        {
            msg_data = jmessage["data"].get<std::string>();
        }
        else
        {
            msg_data = jmessage["data"].dump();
        }
         
        local_buf->stm.users_queue_insert->execute(uuid, (long int)time(NULL), dev_id, user_id, event.data(), msg_data.data(), 0);
    }

    TagTimer::add("internalApi::cluster_send_to_user", t);
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
int internalApi::send_event_to_pipe(thread_data* local_buf, int dev_id, nlohmann::json jmessage)
{
    auto t = TagTimer::mtime();

    local_buf->setThreadStatus('P');

    auto dinfo = devManager::instance()->getDevInfo(dev_id);

    local_buf->setThreadStatus('8');

    std::string pipe_name = jmessage["pipe"].get<std::string>();
    
    CP<Pipe> pipe = dinfo->findPipe(pipe_name);

    local_buf->setThreadStatus('7');

    if(pipe.isNULL())
    {
        // internal server error
        TagLoger::log(Log_Any, 0, "pipe[%s] == NULL in dev_id=%d\n",pipe_name.data(), dev_id);
        TagTimer::add("internalApi::send_event_to_pipe", t);
        return 0;
    }
    else if(pipe->empty() )
    {
        // internal server error
        TagLoger::log(Log_Any, 0, "pipe[%s] == empty\n",pipe_name.data());
        TagTimer::add("internalApi::send_event_to_pipe", t);
        return 0;
    }

    int count_connection = pipe->size();
    TagLoger::log(Log_Any, 0, "pipe[%s] num_users = %d\n",pipe_name.data(), count_connection);

    int num_fail = 0;
    int num_msg = 0;
    auto it = pipe->subscribers->begin();
    while(it)
    {
        CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(it->data);
        if(r && r->web_user_dev_id == dev_id)
        {
            num_msg++;
            int res = r->message(local_buf, jmessage);
            TagLoger::log(Log_Any, 0, "R->message = %d\n" , res);
        }
        else
        {
            pipe->erase(it->data);
            num_fail++;
        }

        it = it->Next();
    }

    TagTimer::add("internalApi::send_event_to_pipe", t);
    return num_msg;
}
