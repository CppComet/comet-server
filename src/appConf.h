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
#include "ini_parser/ini_parser.hpp"

/**
 * Класс для чтения конфигурационного файла и хранения настроек.
 */
class appConf: public ini_parser
{
    
    static appConf* inst;
public:
    bool isHelp = false;
    
    /**
     * Данные для кластера высокой нагрузки
     * Ip адреса нод
     */
    char hl_ip[MAX_HL_CLUSTER_SIZE][100];
    /**
     * Данные для кластера высокой нагрузки
     * Пароли нод
     */
    char hl_pw[MAX_HL_CLUSTER_SIZE][DEV_KEY_LEN+1];
    /**
     * Данные для кластера высокой нагрузки
     * Порты нод
     */
    int hl_port[MAX_HL_CLUSTER_SIZE];
    
public:
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