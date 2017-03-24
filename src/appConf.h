// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   Client_connection.cpp
 * Author: levha
 *
 * Created on 11 Октябь 2014 г., 16:58
 */
#ifndef APP_CONF_H
#define	APP_CONF_H

#include <stdio.h>
#include <cstdlib> 
#include <string.h>
#include <errno.h>
#include <unistd.h>

class appConf;
#include "main.h"
  

/**
 * Класс для чтения конфигурационного файла и хранения настроек.
 */
class appConf
{
    static appConf* inst;
    
public:
    int client_thread_num = 3; 
    int mysql_thread_num = 3; 

    int client_benchmark = 10; 
    int mysql_benchmark = 10; 
     
    /**
     * Надо ли писать в лог запросы от CometQL
     */
    int useQueryLoger = 1;
    
    /**
     * Надо ли использовать сохранение в файл
     */
    int benchmarkToFile = false;
    /**
     * Надо ли использовать сохранение в редис
     */
    int benchmarkToRedis = false;
    /**
     * Надо ли использовать сохранение в лог
     */
    int benchmarkToLog = false;
    
    /**
     * Сбрасывать файл на диск каждые benchmark_file_save секунд
     */
    int benchmark_file_save = 30;
 
    int client_epoll_size = 10000;
    int mysql_epoll_size  = 10000; 
   
    int client_backlog = 1024;
    int mysql_backlog  = 1024; 
     
    char client_ip[100]; 
    char cometql_ip[100]; 
    
    char node_name[100];
     
    char root_password[DEV_KEY_LEN+1]; 
     
    int client_port = 80; 
    int cometql_port = 3306; 

    int buf_size = 10384;
    
    /**
     * Объём буфера для подготовки ответного сообщения
     */
    int answer_buf_size = 1000000;
    
    /**
     * Начальное значение размера памяти для Client_connection::fragment_buf
     * Должно быть больше 0
     */
    int fragment_buf_size = 600;
    
    /**
     * Максимальное значение размера памяти для Client_connection::fragment_buf
     * Должно быть больше 0
     * По достижении этого значения соединение просто закрывается
     */    
    int fragment_buf_max_size = 10384;

    /**
     * Максимальный Uptime соединения после которого оно считается зависшем и закрывается
     * Служит для коректировки статистики
     */
    int maxUptime = 10*60;
    
    bool isHelp = false;
    
    bool isTest = false;
     
    char db_host[100];
    char db_pw[100];
    char db_user[100];
    char db_name[100]; 
    int db_port = 3305;
    
    char base_dir[255];

    appConf();
    void print(void);
    bool init(int argc, char *argv[]);
    bool initFromFile(const char *fileName);

    static appConf* instance()
    {
       if (appConf::inst == 0)
       {
             appConf::inst = new appConf();
       }
       return appConf::inst;
    }

};

#endif	/* APP_CONF_H */