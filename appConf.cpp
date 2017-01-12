// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   Client_connection.cpp
 * Author: levha
 *
 * Created on 11 Октябь 2014 г., 16:58
 */
#include <strings.h>

#include "appConf.h"
#include "mystring.h"


appConf* appConf::inst = NULL;


appConf::appConf()
{    
    TagLoger::setTagLevel(Log_appConf, LOG_ALL);
    
    bzero(client_ip,100); 
    bzero(mysql_ip,100);
    bzero(freeswitch_ip,100);
    
    bzero(redis_host,100);
    bzero(redis_pw,100); 
    bzero(node_name,100);
     
    bzero(base_dir,255);
    strcpy(base_dir, "/var/www/star.comet");
    
    bzero(rootPassword,DEV_KEY_LEN); 
    snprintf(client_ip, 100,NULL_IP);  
    snprintf(mysql_ip, 100,NULL_IP); 
    snprintf(freeswitch_ip, 100,NULL_IP);
    
    snprintf(redis_host, 100,REDIS_HOST);   
      
    snprintf(node_name, 100,NODE_NAME);  
    
    
    bzero(hl_port,MAX_HL_CLUSTER_SIZE);
    
    for(int i=0; i< MAX_HL_CLUSTER_SIZE; i++)
    { 
        bzero(hl_ip[i],100);
        bzero(hl_pw[i],DEV_KEY_LEN);
    }
    
    bzero(db_host,100);
    bzero(db_pw,100);
    bzero(db_user,100);
    bzero(db_name,100);   
}
    
void appConf::print(void)
{

    TagLoger::log(Log_appConf, 0, "Настройки при старте\n");
    // ps aux | grep 'mirror_fantasies_server\s*--' | grep 'victor   [0-9]*' -E -o | grep '[0-9]*' -o | xargs pmap -d | grep "^mapped:" | grep 'private: [0-9]+' -E -o | grep '[0-9]+' -o -E

    TagLoger::log(Log_appConf, 0, "node_name %s\n",node_name); 

    TagLoger::log(Log_appConf, 0, "client_port %d\n",client_port); 
    TagLoger::log(Log_appConf, 0, "mysql_port %d\n",mysql_port);

    TagLoger::log(Log_appConf, 0, "client_ip %s\n",client_ip); 
    TagLoger::log(Log_appConf, 0, "mysql_ip %s\n",mysql_ip);
      

}

bool appConf::initFromFile(const char *fileName)
{
    FILE* f = fopen(fileName, "r");
    if( f == NULL)
    {
        return -1;
    }
    
    char line[1000];
    bzero(line, 999);
    char key[500];
    char *val;
    
    int hl_ip_NodeIndex = 0;
    int hl_pw_NodeIndex = 0;
    int hl_port_NodeIndex = 0;
    
    TagLoger::log(Log_appConf, 0, "Чтение конфига\n");
    
    int i =0;
    while (fgets(line, 999, f) != NULL)
    {
        bzero(key, 500);
        sscanf(line, "%499s", key);
        int pos = str_find(line, ' ');
        if(pos < 1)
        {
            continue;
        }
        
        val = line + pos + 1;
        pos = str_find(val, '#');
        if(pos > 0)
        {
            val[pos] = '\0';
        }
        
        if(memcmp(key, "client_thread_num", strlen("client_thread_num")) == 0)
        {
            sscanf(val, "%4d",&client_thread_num);
            TagLoger::log(Log_appConf, 0, "set client_thread_num %d\n", client_thread_num);
        } 
        else if(memcmp(key, "client_benchmark", strlen("client_benchmark")) == 0)
        {
            sscanf(val,"%6d",&client_benchmark);
            TagLoger::log(Log_appConf, 0, "set client_benchmark %d\n", client_benchmark);
        } 
        else if(memcmp(key, "client_port", strlen("client_port")) == 0)
        {
            sscanf(val,"%6d",&client_port);
            TagLoger::log(Log_appConf, 0, "set client_port %d\n", client_port);
        } 
        else if(memcmp(key, "client_ip", strlen("client_ip")) == 0)
        {
            bzero(client_ip,100);
            sscanf(val,"%20s",client_ip);
            TagLoger::log(Log_appConf, 0, "set client_ip %s\n", client_ip);
        }
        else if(memcmp(key, "buf_size", strlen("buf_size")) == 0)
        {
            sscanf(val,"%6d",&buf_size);
            TagLoger::log(Log_appConf, 0, "set buf_size %d\n", buf_size);
        } 
        else if(memcmp(key, "base_dir", strlen("base_dir")) == 0)
        {
            bzero(base_dir,255);
            sscanf(val, "%250s", base_dir);
            TagLoger::log(Log_appConf, 0, "set base_dir %s\n", base_dir);
        }
        else if(memcmp(key, "mysql_ip", strlen("mysql_ip")) == 0)
        {
            bzero(mysql_ip,100);
            sscanf(val,"%20s",mysql_ip);
            TagLoger::log(Log_appConf, 0, "set mysql_ip %s\n", mysql_ip);
        }
        else if(memcmp(key, "redis_port", strlen("redis_port")) == 0)
        {
            sscanf(val,"%6d",&redis_port);
            TagLoger::log(Log_appConf, 0, "set redis_port %d\n", redis_port);
        }
        else if(memcmp(key, "redis_host", strlen("redis_host")) == 0)
        {
            bzero(redis_host,100);
            sscanf(val,"%20s",redis_host);
            TagLoger::log(Log_appConf, 0, "set redis_host %s\n", redis_host);
        }
        else if(memcmp(key, "redis_pw", strlen("redis_pw")) == 0)
        {
            redis_use_pw = 1;
            bzero(redis_pw,100);
            sscanf(val,"%99s",redis_pw);
            TagLoger::log(Log_appConf, 0, "set redis_pw %s\n", redis_pw);
            TagLoger::log(Log_appConf, 0, "set redis_use_pw %d\n", redis_use_pw);
        } 
        else if(memcmp(key, "redis_use_pw", strlen("redis_use_pw")) == 0)
        {
            sscanf(val,"%1d",&redis_use_pw);
            TagLoger::log(Log_appConf, 0, "set redis_use_pw %d\n", redis_use_pw);
        }
        else if(memcmp(key, "redis_db", strlen("redis_db")) == 0)
        {
            sscanf(val,"%4d",&redis_db);
            TagLoger::log(Log_appConf, 0, "set redis_db %d\n", redis_db);
        } 
        else if(memcmp(key, "password", strlen("password")) == 0)
        {
            bzero(rootPassword,DEV_KEY_LEN);
            sscanf(val,"%64s",rootPassword); // должно быть равно DEV_KEY_LEN
            TagLoger::log(Log_appConf, 0, "set password %s\n", rootPassword);
        } 
        else if(memcmp(key, "node_name", strlen("node_name")) == 0)
        { 
            bzero(node_name,100);
            sscanf(val,"%99s",node_name);
            TagLoger::log(Log_appConf, 0, "set node_name %s\n", node_name); 
        }  
        else if(memcmp(key, "mysql_port", strlen("mysql_port")) == 0)
        {
            sscanf(val,"%7d",&mysql_port);
            TagLoger::log(Log_appConf, 0, "set mysql_port %d\n", mysql_port);
        }
        else if(memcmp(key, "mysql_benchmark", strlen("mysql_benchmark")) == 0)
        {
            sscanf(val,"%7d",&mysql_benchmark);
            TagLoger::log(Log_appConf, 0, "set mysql_benchmark %d\n", mysql_benchmark);
        }
        else if(memcmp(key, "mysql_thread_num", strlen("mysql_thread_num")) == 0)
        {
            sscanf(val,"%7d",&mysql_thread_num);
            TagLoger::log(Log_appConf, 0, 0, "set mysql_thread_num %d\n", mysql_thread_num);
        }
        else if(memcmp(key, "TagLog", strlen("TagLog")) == 0)
        {
            int tag = 0;
            int level = 0;
            sscanf(val,"%4d %4d",&tag, &level);
            TagLoger::setTagLevel(tag, level);
        }
        else if(memcmp(key, "LogLevel", strlen("LogLevel")) == 0)
        {
            int level = 0;
            sscanf(val,"%4d", &level);
            TagLoger::setLogLevel(level);
            TagLoger::setAllTagLevel(level);
        }
        else if(memcmp(key, "benchmark_to_log", strlen("benchmark_to_log")) == 0)
        {
            sscanf(val,"%1d",&benchmarkToLog);
            TagLoger::log(Log_appConf, 0, "set benchmark_to_log %d\n", benchmarkToLog);
        }
        else if(memcmp(key, "benchmark_to_file", strlen("benchmark_to_file")) == 0)
        {
            sscanf(val,"%1d",&benchmarkToFile);
            TagLoger::log(Log_appConf, 0, "set benchmark_to_file %d\n", benchmarkToFile);
        }
        else if(memcmp(key, "benchmark_to_redis", strlen("benchmark_to_redis")) == 0)
        {
            sscanf(val,"%1d",&benchmarkToRedis);
            TagLoger::log(Log_appConf, 0, "set benchmark_to_redis %d\n", benchmarkToRedis);
        }
        else if(memcmp(key, "benchmark_file_save", strlen("benchmark_file_save")) == 0)
        {
            sscanf(val,"%7d",&benchmark_file_save);
            TagLoger::log(Log_appConf, 0, "set benchmark_file_save %d\n", benchmark_file_save);
        }
        
        
        /**
         *  mysql hl кластер 
         */
        else if(memcmp(key, "hl_ip", strlen("hl_ip")) == 0)
        {
            bzero(hl_ip[hl_ip_NodeIndex],100);
            sscanf(val,"%20s",hl_ip[hl_ip_NodeIndex]);
            TagLoger::log(Log_appConf, 0, "set hl_ip[%d] %s\n", hl_ip_NodeIndex, hl_ip[hl_ip_NodeIndex]);
            hl_ip_NodeIndex++;
            
        }
        else if(memcmp(key, "hl_pw", strlen("hl_pw")) == 0)
        {
            bzero(hl_pw[hl_pw_NodeIndex],DEV_KEY_LEN);
            sscanf(val,"%64s",hl_pw[hl_pw_NodeIndex]);
            TagLoger::log(Log_appConf, 0, "set hl_ip[%d] %s\n", hl_pw_NodeIndex, hl_pw[hl_pw_NodeIndex]);
            hl_pw_NodeIndex++; 
        }
        else if(memcmp(key, "hl_port", strlen("hl_port")) == 0)
        {
            hl_port[hl_port_NodeIndex] = 0;
            sscanf(val, "%7d", &hl_port[hl_port_NodeIndex]);
            TagLoger::log(Log_appConf, 0, "set hl_port[%d] %d\n", hl_port_NodeIndex, hl_port[hl_port_NodeIndex]);
            hl_port_NodeIndex++; 
        }
        
        /**
         *  mysql db 
         */
        else if(memcmp(key, "db_host", strlen("db_host")) == 0)
        {
            bzero(db_host,100);
            sscanf(val,"%20s",db_host);
            TagLoger::log(Log_appConf, 0, "set db_host %s\n", db_host);  
        }
        else if(memcmp(key, "db_pw", strlen("db_pw")) == 0)
        {
            bzero(db_pw,100);
            sscanf(val,"%90s",db_pw);
            TagLoger::log(Log_appConf, 0, "set db_pw %s\n", db_pw);  
        }
        else if(memcmp(key, "db_user", strlen("db_user")) == 0)
        {
            bzero(db_user,100);
            sscanf(val,"%20s",db_user);
            TagLoger::log(Log_appConf, 0, "set db_user %s\n", db_user);  
        }
        else if(memcmp(key, "db_name", strlen("db_name")) == 0)
        {
            bzero(db_name,100);
            sscanf(val,"%20s",db_name);
            TagLoger::log(Log_appConf, 0, "set db_name %s\n", db_name);  
        }
        else if(memcmp(key, "db_port", strlen("db_port")) == 0)
        { 
            sscanf(val,"%5d", &db_port);
            TagLoger::log(Log_appConf, 0, "set db_port %d\n", db_port);  
        }
        else if(memcmp(key, "useQueryLoger", strlen("useQueryLoger")) == 0)
        {
            sscanf(val,"%2d",&useQueryLoger);
            TagLoger::log(Log_appConf, 0, "set useQueryLoger %d\n", useQueryLoger);  
        }
        /**
         *  FS db 
         */ 
        if(memcmp(key, "freeswitch_thread_num", strlen("freeswitch_thread_num")) == 0)
        {
            sscanf(val, "%4d",&freeswitch_thread_num);
            TagLoger::log(Log_appConf, 0, "set freeswitch_thread_num %d\n", freeswitch_thread_num);
        }
        else if(memcmp(key, "freeswitch_benchmark", strlen("freeswitch_benchmark")) == 0)
        {
            sscanf(val,"%7d",&freeswitch_benchmark);
            TagLoger::log(Log_appConf, 0, "set freeswitch_benchmark %d\n", freeswitch_benchmark);
        }
        else if(memcmp(key, "freeswitch_ip", strlen("freeswitch_ip")) == 0)
        {
            bzero(freeswitch_ip,100);
            sscanf(val,"%20s",freeswitch_ip);
            TagLoger::log(Log_appConf, 0, "set freeswitch_ip %s\n", freeswitch_ip);
        }
        else if(memcmp(key, "freeswitch_port", strlen("freeswitch_port")) == 0)
        {
            sscanf(val,"%6d",&freeswitch_port);
            TagLoger::log(Log_appConf, 0, "set freeswitch_port %d\n", freeswitch_port);
        } 
        
        // backlog
        else if(memcmp(key, "freeswitch_backlog", strlen("freeswitch_backlog")) == 0)
        {
            sscanf(val,"%6d",&freeswitch_backlog);
            TagLoger::log(Log_appConf, 0, "set freeswitch_backlog %d\n", freeswitch_backlog);
        } 
        else if(memcmp(key, "client_backlog", strlen("client_backlog")) == 0)
        {
            sscanf(val,"%6d",&client_backlog);
            TagLoger::log(Log_appConf, 0, "set client_backlog %d\n", client_backlog);
        }
        else if(memcmp(key, "mysql_backlog", strlen("mysql_backlog")) == 0)
        {
            sscanf(val,"%6d",&mysql_backlog);
            TagLoger::log(Log_appConf, 0, "set mysql_backlog %d\n", mysql_backlog);
        }
        
        
        else if(memcmp(key, "dev_index_size", strlen("dev_index_size")) == 0)
        {
            sscanf(val,"%6d",&dev_index_size);
            TagLoger::log(Log_appConf, 0, "set dev_index_size %d\n", dev_index_size);
        }
    }
    
    fclose(f);
}

bool appConf::init(int argc, char *argv[])
{
    if(argc > 1)
    {
        for(int i=0; i<argc; i++)
        {
            if(strlen(argv[i]) >= strlen("--help")  && memcmp(argv[i],"--help",strlen("--help"))==0)
            {
                    printf("Справка:\n \thttp://comet-server.ru\n \tsupport@comet-server.ru\n"); 
                    isHelp = true;
                    return true;
            } 
        }
        return true;
    }
    else
    {
        return false;
    }
}

