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


appConf::appConf():ini_parser()
{     
    bzero(hl_port,MAX_HL_CLUSTER_SIZE);
    
    for(int i=0; i< MAX_HL_CLUSTER_SIZE; i++)
    { 
        bzero(hl_ip[i],100);
        bzero(hl_pw[i],DEV_KEY_LEN);
    } 
}
     
bool appConf::initFromFile(const char *fileName)
{
    if(!parse(fileName))
    {
        return false;
    }
    
    // Для обратной совместимости с конфигами прошлых версий
    if(is_property_exists("log", "benchmark"))
    {
        sections["log"]["statistics"] = sections["log"]["benchmark"];  
    }
    
    // Для обратной совместимости с конфигами прошлых версий
    if(is_property_exists("cometql", "benchmark"))
    {
        sections["cometql"]["statistics"] = sections["cometql"]["benchmark"];  
    }
    
    // Для обратной совместимости с конфигами прошлых версий
    if(is_property_exists("ws", "benchmark"))
    {
        sections["ws"]["statistics"] = sections["ws"]["benchmark"];  
    }
    
    // Для обратной совместимости с конфигами прошлых версий
    if(is_property_exists("benchmark", "to_log"))
    {
        sections["statistics"]["to_log"] = sections["benchmark"]["to_log"];  
    }
      
    if(!is_property_exists("statistics", "interval"))
    {
        sections["statistics"]["interval"] = "3600";  
    }
     
    if(!is_property_exists("statistics", "allow"))
    {
        sections["statistics"]["allow"] = "true";  
    }
    
    
    if(!is_property_exists("main", "send_user_online_events"))
    {
        sections["main"]["send_user_online_events"] = "true";  
    }
    
    if(!is_property_exists("main", "send_user_offline_events"))
    {
        sections["main"]["send_user_offline_events"] = "true";  
    }
    
    if(!is_property_exists("main", "save_users_last_online_time"))
    {
        sections["main"]["save_users_last_online_time"] = "true";  
    }
    
    if(!is_property_exists("main", "pidfile"))
    {
        sections["main"]["pidfile"] = "/var/run/cppcomet.pid";  
    }
    
    if(!is_property_exists("main", "api_version"))
    {
        sections["main"]["api_version"] = "1";  
    }
    
    /*
    ; Объём буфера для сообщения
    ; Много ставить не надо, комет сервер не расчитан на передачу больших сообщений
    buf_size = 10384

    ; Максимальное значение размера памяти для Client_connection::fragment_buf
    ; Должно быть больше 0
    ; По достижении этого значения соединение просто закрывается
    fragment_buf_max_size = 10384

    ; Начальное значение размера памяти для Client_connection::fragment_buf Должно быть больше 0
    fragment_buf_size = 600 

    ; Объём буфера для подготовки ответного сообщения
    answer_buf_size = 1000000*/
  
    if(!is_property_exists("sip", "pipesalt"))
    {
        sections["sip"]["pipesalt"] = "pipesalt";  
    }
    
    if(!is_property_exists("sip", "port"))
    {
        sections["sip"]["port"] = "7443";  
    }
         
    if(!is_property_exists("main", "buf_size"))
    {
        sections["main"]["buf_size"] = "10384";  
    }
        
    if(!is_property_exists("db", "buf_size"))
    {
        sections["db"]["buf_size"] = "10384";  
    }
        
    if(!is_property_exists("main", "answer_buf_size"))
    {
        sections["main"]["answer_buf_size"] = "1000000";  
    }   
    
    if(!is_property_exists("main", "fragment_buf_size"))
    {
        sections["main"]["fragment_buf_size"] = "600";  
    }   
    
    if(!is_property_exists("main", "fragment_buf_max_size"))
    {
        sections["main"]["fragment_buf_max_size"] = "10384";  
    }
            
    if(!is_property_exists("main", "password"))
    {
        sections["main"]["password"] = MAIN_PASSWORD;  
    }
     
    
    if(!is_property_exists("main", "node_name"))
    {
        sections["main"]["node_name"] = NODE_NAME;  
    }
     
    if(!is_property_exists("main", "dev_index_size"))
    {
        sections["main"]["dev_index_size"] = MAIN_DEV_INDEX_SIZE;  
    }
    
    if(!is_property_exists("http", "base_dir"))
    { 
        sections["http"]["base_dir"] = HTTP_BASE_DIR;   
    }
    
        
     
    if(!is_property_exists("ws", "port"))
    {
        sections["ws"]["port"] = "8087"; 
    } 
     
     
    if(!is_property_exists("comeql", "port"))
    {
        sections["comeql"]["port"] = "3300"; 
    } 
      
    if(!is_property_exists("freeswitch", "port"))
    {
        sections["freeswitch"]["port"] = "8082"; 
    } 
         
    if(!is_property_exists("cometqlproxy", "port"))
    {
        sections["cometqlproxy"]["port"] = "3301"; 
    } 
    
    if(!is_property_exists("log", "level"))
    {
        sections["log"]["level"] = "200"; 
    } 
        
    /**
     *  mysql db 
     */
    
    if(!is_property_exists("db", "connection_attempts"))
    {
        sections["db"]["connection_attempts"] = "10"; 
    } 
    
    if(!is_property_exists("db", "host"))
    {
        // Ошибочка, параметр обязателен
        TagLoger::error(Log_appConf, 0, "\x1b[1;31mCppComet in file comet.ini in section [db] param `host` is required\x1b[0m");
        return false;
    }
    else if(!is_property_exists("db", "port"))
    {
        // Ошибочка, параметр обязателен
        TagLoger::error(Log_appConf, 0, "\x1b[1;31mCppComet in file comet.ini in section [db] param `port` is required\x1b[0m");
        return false;
    }
    else if(!is_property_exists("db", "password"))
    {
        // Ошибочка, параметр обязателен
        TagLoger::error(Log_appConf, 0, "\x1b[1;31mCppComet in file comet.ini in section [db] param `password` is required\x1b[0m");
        return false;
    }
    else if(!is_property_exists("db", "user"))
    {
        // Ошибочка, параметр обязателен
        TagLoger::error(Log_appConf, 0, "\x1b[1;31mCppComet in file comet.ini in section [db] param `user` is required\x1b[0m");
        return false;
    }
    else if(!is_property_exists("db", "name"))
    {
        // Ошибочка, параметр обязателен
        TagLoger::error(Log_appConf, 0, "\x1b[1;31mCppComet in file comet.ini in section [db] param `name` is required\x1b[0m");
        return false;
    }  
    return true;
}

bool file_exists(const std::string& name)
{
    if (FILE *file = fopen(name.c_str(), "r"))
    {
        fclose(file);
        return true;
    } 
    else 
    {
        return false;
    }   
}

bool appConf::init(int argc, char *argv[])
{
    int conf_path = -1;
    if(argc > 1)
    {
        for(int i=0; i<argc; i++)
        {
            if(strcmp(argv[i],"--help") == 0)
            {
                    printf("Help:\n \thttp://comet-server.com\n \tsupport@comet-server.com\n"); 
                    isHelp = true;
                    return false;
            }
            
            if(strcmp(argv[i],"--conf") == 0)
            { 
                    conf_path = i+1;
                    i++;
                    
                    if(argc <= i)
                    {
                        printf("Error: argument --conf mast have valid value.\n");
                        return false;
                    }
            }
            
            if(strcmp(argv[i],"--cli") == 0)
            {  
                useCLI = true;
            }
        }
    }
     
    if(conf_path == -1 && file_exists("/etc/comet-server/comet.ini"))
    {
        printf("Config file: /etc/comet-server/comet.ini\n"); 
        return initFromFile("/etc/comet-server/comet.ini"); 
    }
    else if(conf_path == -1 && file_exists("./comet.ini"))
    {
        printf("Config file: ./comet.ini\n"); 
        return initFromFile("comet.ini"); 
    }
    
    printf("Config file: %s\n", argv[conf_path]); 
    return initFromFile(argv[conf_path]);  
}

