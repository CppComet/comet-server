// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "thread_data.h"


void thread_data::setThreadStatus(char c)
{
    if(bm != NULL)
    {
        bm->set_th_status(thread_id, c);
    }
}


thread_data::thread_data( appConf* app):buf(app->get_int("main", "buf_size")), messge_buf(app->get_int("main", "buf_size")), answer_buf(app->get_int("main", "answer_buf_size")),sql(),bm(NULL)
{
    /*bzero(tmp_bufdata, ARRAY_BUFFER_SIZE);
    bzero(tmp_bufdataSize, ARRAY_BUFFER_SIZE);
    bzero(tmp_bufdataPrt, ARRAY_BUFFER_SIZE);

    tmp_bufdata[0] = new char[app->buf_size*2];
    bzero(tmp_bufdata[0], app->buf_size*2);

    tmp_bufdataSize[0] = app->buf_size*2;*/

    db.setStmMapper(&stm);
    //db.reconnect_on_error = false;
    db.init(app->get_chars("db", "host"), app->get_chars("db", "user"), app->get_chars("db", "password"), app->get_chars("db", "name"), app->get_int("db", "port"));
    db.connect();

    // cometqlproxy кластер для рассылки сообщений приходившех с cometqlproxy
    auto proxycluster = app->get_list("cometqlproxy", "cluster");
    if(!proxycluster.empty())
    {
        int id = 0;
        TagLoger::log(Log_Any, LogColorGreen, "Starting Proxy-CometQL cluster on %d nodes", proxycluster.size());
        auto it = proxycluster.begin();
        while(it != proxycluster.end())
        {
            id++;
            std::string name;
            name.append("cometqlcluster-").append(std::to_string(id));

            dbLink *link = new dbLink(name);
            proxyCluster.push_back(link);
            if(!link->init(it->data()))
            {
                TagLoger::error(Log_Any, LogColorRed, "Error, Proxy-CometQL connection %s does not establish", it->data());
            }

            it++;
        }
        TagLoger::log(Log_Any, LogColorGreen, "Starting Proxy-CometQL cluster on %d nodes complte", proxycluster.size());
    }
    else
    { 
        TagLoger::log(Log_Any, LogColorBase, "section [cometqlproxy] value [cluster] is empty");
    }

    // WS кластер для рассылки сообщений приходившех с вебсокетов
    auto wscluster = app->get_list("ws", "cluster"); 
    if(!wscluster.empty())
    {
        int id = 0;
        TagLoger::log(Log_Any, LogColorGreen, "Starting WS-CometQL cluster on %d nodes", wscluster.size());
        auto it = wscluster.begin();
        while(it != wscluster.end())
        {
            id++;
            std::string name;
            name.append("wscluster-").append(std::to_string(id));
            dbLink *link = new dbLink(name);
            wsCluster.push_back(link);
            if(!link->init(it->data()))
            {
                TagLoger::error(Log_Any, LogColorRed, "Error, WS-CometQL connection %s does not establish", it->data());
            }

            it++;
        }
        TagLoger::log(Log_Any, LogColorGreen, "Starting WS-CometQL cluster on %d nodes complte", wscluster.size());
    }
    else
    { 
        TagLoger::log(Log_Any, LogColorBase, "section [ws] value [cluster] is empty");
    }
}
