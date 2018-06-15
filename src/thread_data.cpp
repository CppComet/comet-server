// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "thread_data.h"


bool fs_esl::init(std::string connectionString){
   
    int pos = 0;
    int nextPos = 0;

    std::string Server = "localhost"; 
    std::string Pwd = "CueCon";
     
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
        else if(paramName.compare("Pwd") == 0)
        {
            Pwd = paramValue; 
        } 
        else if(paramName.compare("WShost") == 0)
        {
            WShost = paramValue; 
        } 
        else if(paramName.compare("Port") == 0)
        {
            try{
                //printf("get_long [%s] %s=%s\n", section.data(), name.data(), sections.at(section).at(name).data());
                port = std::stoi(paramValue);
            }catch(...)
            {
                printf("\x1b[1;31mexeption in parsing Port value Port=%s\x1b[0m\n", paramValue.data());
                return false;
            } 
        }
        else if(paramName.compare("WSport") == 0)
        {
            try{
                //printf("get_long [%s] %s=%s\n", section.data(), name.data(), sections.at(section).at(name).data());
                WSport = std::stoi(paramValue);
            }catch(...)
            {
                printf("\x1b[1;31mexeption in parsing WSport value WSport=%s\x1b[0m\n", paramValue.data());
                return false;
            } 
        }
        else if(StreamServer.compare("StreamServer") == 0)
        {
            StreamServer = paramValue; 
        } 
    }
    
    if(WShost.empty())
    {
        WShost = Server;
    }
     
    host = Server.data(); 
    password = Pwd.data();
    inited = true;
    timeout = 1;
    
    id = host;
    id.append(":");
    id.append(std::to_string(WSport));
    
    return true; 
}

esl_handle_t fs_esl::getHandle()
{
    return handle;
}

void fs_esl::connect()
{
    conected = true;
    esl_connect(&handle, host.data(), port, NULL, password.data());
}

void fs_esl::disconnect()
{
    if(conected)
    {
        esl_disconnect(&handle);
    }
}

void fs_esl::exec(const char* command)
{
    if(!conected)
    {
        connect();
    }
    
    esl_send_recv(&handle, command);
}


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
    
    auto fsCluster = app->get_list("sip", "freeswitch");
    if(!fsCluster.empty())
    {
        int id = 0;
        TagLoger::log(Log_Any, LogColorGreen, "Starting SIP-FS cluster on %d nodes", fsCluster.size());
        auto it = fsCluster.begin();
        while(it != fsCluster.end())
        {
            id++;
            std::string name;
            name.append("FS-cluster-").append(std::to_string(id));

            fs_esl *link = new fs_esl(name);
            fs_eslCluster.push_back(link);
            if(!link->init(it->data()))
            {
                TagLoger::error(Log_Any, LogColorRed, "Error, Proxy-CometQL connection %s does not establish", it->data());
            }

            it++;
        }
        TagLoger::log(Log_Any, LogColorGreen, "Starting Proxy-CometQL cluster on %d nodes complte", fs_eslCluster.size());
    }
    else
    {
        TagLoger::log(Log_Any, LogColorBase, "section [sip] value [freeswitch] is empty");
    }
    
    
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
            //link->connect();

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
