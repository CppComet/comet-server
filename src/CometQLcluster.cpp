// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "CometQLcluster.h"


const char sinc_header[] = "==SINC=="; 
int CometQLClusterQueryData::read(char* queryString)
{
    if(memcmp(sinc_header, queryString, strlen(sinc_header)) != 0)
    {
        bzero(this, sizeof(CometQLClusterQueryData));
        return 0;
    }

    memcpy(this, queryString + strlen(sinc_header), sizeof(CometQLClusterQueryData));
    
    //printf("CometQLClusterQueryData::read[%d+%d] node=%d, startTime=%d, queryCount=%d, dev_id=%d\n", sizeof(CometQLClusterQueryData) , strlen(sinc_header), node, startTime, queryCount, dev_id);
    
    return sizeof(CometQLClusterQueryData) + strlen(sinc_header);
}

int CometQLClusterQueryData::wright(char* queryString)
{
    memcpy(queryString, sinc_header, strlen(sinc_header)); 
    memcpy(queryString + strlen(sinc_header), this, sizeof(CometQLClusterQueryData)); 
    return sizeof(CometQLClusterQueryData) + strlen(sinc_header);
}


int CometQLcluster::read(char* queryString)
{
    int d = queryData.read(queryString);
    if(d > 0)
    {
        // Определяем длину запроса без учёта ; и любых символов с кодом <= 32
        int qlen = strlen(queryString+d);
        //printf("queryString[%d+%d]=%s\n", d, qlen, queryString+d);
        
        for(int j = qlen+d; j >= d; j--)
        {
            if(queryString[j] > 32 && queryString[j] != ';')
            {
                qlen = j+1;
                break;
            }
        }

        queryLen = qlen;
        bzero(query, appConf::instance()->get_int("main", "buf_size"));
        memcpy(query, queryString, qlen+1);
        //printf("query[%d]=%s\n", queryLen, query);
    }

    return d;
}

void CometQLcluster::replicateQuery()
{ 
    if(queryLen == 0)
    {
        return;
    }
    
    TagLoger::log(Log_CometQLCluster, 0, "replicateQuery:%s", query); 
    for(int i = 0; i < MAX_HL_CLUSTER_SIZE; i++)
    {
        if(appConf::instance()->hl_port[i] == 0)
        {
            break;
        }
         
        if(mysql_real_query(&mysql[i], query, queryLen) != 0)
        {
            TagLoger::warn(Log_CometQLCluster, 0, "\x1b[1;31mCometQL запрос не выполнен\n%s\n%s\nnode=%d ip=%s:%d pw=%s\x1b[0m", 
                    mysql_error(&mysql[i]),
                    query,
                    i,
                    appConf::instance()->hl_ip[i],
                    appConf::instance()->hl_port[i],
                    appConf::instance()->hl_pw[i]); 
        }
    }
    
    queryLen = 0;
}


void CometQLcluster::start()
{ 
    if(appConf::instance()->hl_port[0] == 0)
    {
        TagLoger::log(Log_CometQLCluster, 0, "Запуск CometQL кластера отменён\n");
        return;
    }
    
    TagLoger::log(Log_CometQLCluster, 0, "Запуск CometQL кластера\n");
    
    for(int i = 0; i < MAX_HL_CLUSTER_SIZE; i++)
    {
        if(appConf::instance()->hl_port[i] == 0)
        {
            break;
        }
        TagLoger::log(Log_CometQLCluster, 0, "CometQL кластер node=%d ip=%s:%d pw=%s\n", i, appConf::instance()->hl_ip[i], appConf::instance()->hl_port[i], appConf::instance()->hl_pw[i]); 
        
        mysql_init(&mysql[i]);
        mysql_real_connect(&mysql[i], appConf::instance()->hl_ip[i], "15", appConf::instance()->hl_pw[i], "CometQL_v1", appConf::instance()->hl_port[i], NULL, 0);
        
        if(mysql_errno(&mysql[i]))
        {
            TagLoger::error(Log_CometQLCluster, 0, "\x1b[1;31mCometQL соединение не уствановлено\n%s\nnode=%d ip=%s:%d pw=%s\x1b[0m", mysql_error(&mysql[i]),
                    i, appConf::instance()->hl_ip[i], appConf::instance()->hl_port[i], appConf::instance()->hl_pw[i]); 
        }
        else
        {
            TagLoger::error(Log_CometQLCluster, 0, "\x1b[1;32mCometQL соединение уствановлено\nnode=%d ip=%s:%d pw=%s\x1b[0m", 
                    i, appConf::instance()->hl_ip[i], appConf::instance()->hl_port[i], appConf::instance()->hl_pw[i]); 
            
        }
    } 
}
