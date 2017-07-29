
#include "tcpServer_benchmark.h"


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
