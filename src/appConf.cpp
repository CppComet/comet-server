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
     
    if(!is_property_exists("http", "base_dir"))
    { 
        sections["http"]["base_dir"] = HTTP_BASE_DIR;   
    }
    
        
     
    if(!is_property_exists("ws", "port"))
    {
        sections["ws"]["port"] = "8080"; 
    } 
     
     
    if(!is_property_exists("comeql", "port"))
    {
        sections["comeql"]["port"] = "8081"; 
    } 
      
    if(!is_property_exists("freeswitch", "port"))
    {
        sections["freeswitch"]["port"] = "8082"; 
    } 
         
    if(!is_property_exists("log", "level"))
    {
        sections["log"]["level"] = "200"; 
    } 
          
    /**
     *  mysql hl кластер 
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
    */

    /**
     *  mysql db 
     */
    if(!is_property_exists("db", "host"))
    {
        // Ошибочка, параметр обязателен
    }
    else if(!is_property_exists("db", "host"))
    {
        // Ошибочка, параметр обязателен
    }
    else if(!is_property_exists("db", "db_pw"))
    {
        // Ошибочка, параметр обязателен
    }
    else if(!is_property_exists("db", "db_user"))
    {
        // Ошибочка, параметр обязателен
    }
    else if(!is_property_exists("db", "db_name"))
    {
        // Ошибочка, параметр обязателен
    }  
    return true;
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
        }
    }
    
    if(conf_path == -1)
    {
        return initFromFile("comet.ini"); 
    }
    
    return initFromFile(argv[conf_path]);  
}

