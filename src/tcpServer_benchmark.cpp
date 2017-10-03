
#include "tcpServer_benchmark.h"
#include "Client_connection.h"
#include "MySql_connection.h"
#include "Freeswitch_connection.h" 
#include "CometQLProxy_connection.h"
#include "devManager.h"
#include "mystring.h"



time_t tcpServer_benchmark::start_time = time(0);
 
void tcpServer_benchmark::start(int Servewr_id, int th_Num, const char* server_name)
{
    bzero(name, sizeof(char) * 299);
    memcpy(name, server_name, strlen(server_name));

    TagLoger::log(Log_benchmark, 0, "Starting benchmark %d [%s]\n",Servewr_id, name);
    if(th_id == 0)
    {
        th_num = th_Num;
        th_status = new char[th_num];
        th_status_count = new int[th_num];
        th_ps_status_count = new float[th_num];
        th_tps_status_count = new int[th_num];

        for(int i = 0; i< th_num; i++)
        {
            th_status[i] = '-';
            th_status_count[i] = 0; 
            th_ps_status_count[i] = 0;
            th_tps_status_count[i] = 0; 
        }
        conections = 0;
        server_id = Servewr_id;

        //pthread_create(&th_id, NULL, stat_loop , (void *)this);
        intervalLoop::instance()->add((intervalLoopObject*)this);
    }
}

/**
 * Отправка данных статистики через curl
 * @param data данные
 * @return
 */
bool usage_statistics::send(std::string data)
{
    char cli[600];
    std::string cmd;
    cmd.append("curl -d \"").append(data).append("\" --connect-timeout 1 --max-time 1 -H \"Content-Type: text/plain\" -X POST http://statistics.comet-server.ru/api/statistics > /dev/null 2>&1");
    if(exec(cmd.data(), cli, 600))
    {
        return true;
    }

    //TagLoger::log(Log_benchmark, 0, "usage_statistics::send [%s]\n",cli);
    return false;
}

void usage_statistics::start()
{
    intervalLoop::instance()->add([](int uptime, thread_data* local_buf)
    {
        /**
         * Интервал между сбрасыванием статистики
         * Будет примерно interval отправок статистики в сутки
         */
        int interval = appConf::instance()->get_int("statistics", "interval");
        if( interval == 0)
        {
            return;
        }
         
        if(!appConf::instance()->get_bool("statistics", "allow"))
        {
            return;
        }

        if( uptime != 60 && uptime != 600 && uptime % interval != 0)
        {
            return;
        }

        std::string exportdata;

        exportdata.append("SERVERNAME=").append(MYSQL_SERVERNAME).append("&");
        exportdata.append("SERVERNAME_FULL=").append(MYSQL_SYSTEMVARIBLE).append("&");

        exportdata.append("host=").append(appConf::instance()->get_string("ws", "host")).append("&");
        exportdata.append("email=").append(appConf::instance()->get_string("statistics", "email")).append("&");

        FILE* meminfoFp = fopen("/proc/meminfo", "r");
        char name[200];
        char value[255];
        char tmpNull[200];
        int len;
        int i = 0;
        while(fscanf(meminfoFp, "%64s %64s %64s", name, value, tmpNull) != EOF && i < 34)
        {
            i++;
            len = strlen(name);
            if(len > 0)
            {
                name[len - 1] = 0;
            }
            exportdata.append(name).append("=").append(value).append("&");
        }
        fclose(meminfoFp);


        FILE* loadavgFp = fopen("/proc/loadavg", "r");
        float loadavg_1, loadavg_2, loadavg_3;
        int running_processes, total_processes;

        fscanf(loadavgFp, "%5f %5f %5f %5d/%5d", &loadavg_1, &loadavg_2, &loadavg_3, &running_processes, &total_processes);
        fclose(loadavgFp);

        snprintf(value, 255, "%.2f", loadavg_1);
        exportdata.append("load_avg_1=").append(value).append("&");

        snprintf(value, 255, "%.2f", loadavg_1);
        exportdata.append("load_avg_2=").append(value).append("&");

        snprintf(value, 255, "%.2f", loadavg_1);
        exportdata.append("load_avg_3=").append(value).append("&");

        snprintf(value, 255, "%d", running_processes);
        exportdata.append("running_processes=").append(value).append("&");

        snprintf(value, 255, "%d", total_processes);
        exportdata.append("total_processes=").append(value).append("&");

        // http://linuxinsight.com/proc_uptime.html
        float os_uptime, os_idle;
        FILE* uptimeFp = fopen("/proc/uptime", "r");
        fscanf(uptimeFp, "%f %f", &os_uptime, &os_idle);
        fclose(uptimeFp);

        snprintf(value, 255, "%f", os_uptime);
        exportdata.append("os_uptime=").append(value).append("&");

        snprintf(value, 255, "%f", os_idle);
        exportdata.append("os_idle=").append(value).append("&");


        snprintf(value, 255, "%ld", tcpServer <MySql_connection>::instance()->bm.get_uptime());
        exportdata.append("uptime=").append(value).append("&");


        snprintf(value, 255, "%d", tcpServer <Client_connection>::instance()->bm.stat_interval);
        exportdata.append("frontend_stat_interval=").append(value).append("&");

        snprintf(value, 255, "%d", tcpServer <MySql_connection>::instance()->bm.stat_interval);
        exportdata.append("backend_stat_interval=").append(value).append("&");

        snprintf(value, 255, "frontend_online=%d&", tcpServer <Client_connection>::instance()->bm.getConections());
        exportdata.append(value);

        snprintf(value, 255, "backend_online=%d&", tcpServer <MySql_connection>::instance()->bm.getConections());
        exportdata.append(value);

        // messages
        snprintf(value, 255, "frontend_messages=%d&", tcpServer <Client_connection>::instance()->bm.getHandleMessage());
        exportdata.append(value);

        snprintf(value, 255, "backend_messages=%d&", tcpServer <MySql_connection>::instance()->bm.getHandleMessage());
        exportdata.append(value);


        // add_client
        snprintf(value, 255, "frontend_add_client=%d&", tcpServer <Client_connection>::instance()->bm.getAddClient());
        exportdata.append(value);

        snprintf(value, 255, "backend_add_client=%d&", tcpServer <MySql_connection>::instance()->bm.getAddClient());
        exportdata.append(value);


        // delete_client
        snprintf(value, 255, "frontend_delete_client=%d&", tcpServer <Client_connection>::instance()->bm.getDeleteClient());
        exportdata.append(value);

        snprintf(value, 255, "backend_delete_client=%d&", tcpServer <MySql_connection>::instance()->bm.getDeleteClient());
        exportdata.append(value);


        // messages_ps
        snprintf(value, 255, "frontend_messages_ps=%f&", (float)tcpServer <Client_connection>::instance()->bm.getPsHandleMessage());
        exportdata.append(value);

        snprintf(value, 255, "backend_messages_ps=%f&", (float)tcpServer <MySql_connection>::instance()->bm.getPsHandleMessage());
        exportdata.append(value);


        // addClient_ps
        snprintf(value, 255, "frontend_add_client_ps=%f&", (float)tcpServer <Client_connection>::instance()->bm.getPsAddClient());
        exportdata.append(value);

        snprintf(value, 255, "backend_add_client_ps=%f&", (float)tcpServer <MySql_connection>::instance()->bm.getPsAddClient());
        exportdata.append(value);


        // deleteClient_ps
        snprintf(value, 255, "frontend_delete_client_ps=%f&", (float)tcpServer <Client_connection>::instance()->bm.getPsDeleteClient());
        exportdata.append(value);

        snprintf(value, 255, "backend_delete_client_ps=%f&", (float)tcpServer <MySql_connection>::instance()->bm.getPsDeleteClient());
        exportdata.append(value);


        snprintf(value, 255, "network_events=%f&", devManager::instance()->getPsNetworkEvents());
        exportdata.append(value);



        snprintf(value, 255, "MySql_connection_th_num=%d&", tcpServer <MySql_connection>::instance()->bm.get_th_num());
        exportdata.append(value);

        snprintf(value, 255, "Client_connection_th_num=%d&", tcpServer <Client_connection>::instance()->bm.get_th_num());
        exportdata.append(value);

        snprintf(value, 255, "Freeswitch_connection_th_num=%d&", tcpServer <Freeswitch_connection>::instance()->bm.get_th_num());
        exportdata.append(value);
 
        snprintf(value, 255, "CometQLProxy_connection_th_num=%d&", tcpServer <CometQLProxy_connection>::instance()->bm.get_th_num());
        exportdata.append(value);
        
        for(int i =0; i < tcpServer <MySql_connection>::instance()->bm.get_th_num(); i++)
        {
            snprintf(value, 255, "MySql_connection_th=%d,%c,%d&", i, (char)tcpServer <MySql_connection>::instance()->bm.get_th_status(i), tcpServer <MySql_connection>::instance()->bm.get_th_count(i));
            exportdata.append(value);
        }

        for(int i =0; i < tcpServer <Client_connection>::instance()->bm.get_th_num(); i++)
        {
            snprintf(value, 255, "Client_connection_th=%d,%c,%d&", i, (char)tcpServer <Client_connection>::instance()->bm.get_th_status(i), tcpServer <Client_connection>::instance()->bm.get_th_count(i));
            exportdata.append(value);
        }

        for(int i =0; i < tcpServer <Freeswitch_connection>::instance()->bm.get_th_num(); i++)
        {
            snprintf(value, 255, "Freeswitch_connection_th=%d,%c,%d&", i, (char)tcpServer <Freeswitch_connection>::instance()->bm.get_th_status(i), tcpServer <Freeswitch_connection>::instance()->bm.get_th_count(i));
            exportdata.append(value);
        }
 
        for(int i =0; i < tcpServer <CometQLProxy_connection>::instance()->bm.get_th_num(); i++)
        {
            snprintf(value, 255, "CometQLProxy_connection=%d,%c,%d&", i, (char)tcpServer <CometQLProxy_connection>::instance()->bm.get_th_status(i), tcpServer <CometQLProxy_connection>::instance()->bm.get_th_count(i));
            exportdata.append(value);
        }
        exportdata.append("end=true");
        send(exportdata);
    });
}