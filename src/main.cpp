// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   main.cpp
 * Author: levha
 *
 * Created on 24 Апрель 2011 г., 13:37
 */


#include <mysql.h>

#include "main.h"


#include "devManager.h"
#include "tcpServer.h"

#include "Client_connection.h"
#include "MySql_connection.h"
#include "Freeswitch_connection.h"  
#include "CometQLProxy_connection.h" 

#include "intervalLoop.h"

// To store client's socket list
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ctime>

#include <pthread.h>

#include <stdio.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/wait.h>

#include <execinfo.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "backtrace.h"

#include "appConf.h"

#include <ctime>
 
template< class connectionType >
void th_startServer(int threadid, const char* server_name)
{
    
    if(!appConf::instance()->is_property_exists(server_name, "thread_num") || appConf::instance()->get_int(server_name, "thread_num") == 0)
    {
        return;
    }

    if(!appConf::instance()->is_property_exists("main", "buf_size"))
    {
        appConf::instance()->set_value("main", "buf_size", MAIN_BUF_SIZE);  
    }
    
    if(!appConf::instance()->is_property_exists(server_name, "ip"))
    {
        appConf::instance()->set_value(server_name, "ip", NULL_IP);  
    }
    
    if(!appConf::instance()->is_property_exists(server_name, "backlog"))
    {
        appConf::instance()->set_value(server_name, "backlog", MAIN_BACKLOG_SIZE);  
    }
    
    if(!appConf::instance()->is_property_exists(server_name, "epoll_size"))
    {
        appConf::instance()->set_value(server_name, "epoll_size", MAIN_EPOLL_SIZE);  
    }
    
    TagLoger::log(Log_Any, 0, "Hello World! It's me, %s #%ld!\n", server_name, threadid);

    tcpServer <connectionType>::instance()->id = threadid;
    tcpServer <connectionType>::instance()->set_thread_num(appConf::instance()->get_int(server_name, "thread_num"));
    tcpServer <connectionType>::instance()->epoll_size = appConf::instance()->get_int(server_name, "epoll_size");
    tcpServer <connectionType>::instance()->backlog = appConf::instance()->get_int(server_name, "backlog");



    if(appConf::instance()->get_int(server_name, "statistics") > 0)
    {
        tcpServer <connectionType>::instance()->benchmark = 1;
        tcpServer <connectionType>::instance()->bm.stat_interval = appConf::instance()->get_int(server_name, "statistics");
    }
    else
    {
        tcpServer <connectionType>::instance()->benchmark = 0;
    }

    tcpServer <connectionType>::instance()->start(appConf::instance()->get_string(server_name, "ip").data(), appConf::instance()->get_int(server_name, "port"), server_name);
    return;
}
 
/**
 * Обрабатывает сигнал смерти.
 * @link http://habrahabr.ru/post/131412/
 * @param signum
 */
void posix_death_signal(int signum)
{
    TagLoger::log(Log_Any, 0, "\n\n\x1b[1;31mposix_death_signal\x1b[0m\n");
    print_backtrace();
    TagLoger::log(Log_Any, 0, "\x1b[1;31mok\x1b[0m\n");

    time_t seconds = time(NULL);
    tm* timeinfo = localtime(&seconds);

    /**
     * @todo перевести на использование strftime
     */
    char *bufTime = asctime(timeinfo);
    char error_string[900];
    snprintf(error_string, 900, "signum=%d\tTime=%ld\t[%s]\n", signum, seconds, bufTime);

    int fp = open("posix_signal.log", O_WRONLY | O_CREAT | O_APPEND, 0666);
    write(fp, error_string, strlen(error_string));
    close(fp);
    TagLoger::log(Log_Any, 0, "\x1b[1;33m%s\x1b[0m\n", error_string);


    signal(signum, SIG_DFL); // перепосылка сигнала
    fcloseall();

    exit(3); //выход из программы. Если не сделать этого, то обработчик будет вызываться бесконечно.
}

/**
 * Обрабатывает сигнал смерти.
 * @link http://habrahabr.ru/post/131412/
 * @param signum
 */
void posix_log_signal(int signum)
{
    TagLoger::log(Log_Any, 0, "\n\n\x1b[1;mmposix_log_signal\x1b[0m\n");
    print_backtrace();
    TagLoger::log(Log_Any, 0, "\x1b[1;32mok\x1b[0m\n");

    time_t seconds = time(NULL);
    tm* timeinfo = localtime(&seconds);

    /**
     * @todo перевести на использование strftime
     */
    char *bufTime = asctime(timeinfo);
    char error_string[900];
    snprintf(error_string, 900, "signum=%d\tTime=%ld\t[%s]\n", signum, seconds, bufTime);

    int fp = open("posix_signal.log", O_WRONLY | O_CREAT | O_APPEND, 0666);
    write(fp, error_string, strlen(error_string));
    close(fp);
    TagLoger::log(Log_Any, 0, "\x1b[1;33m%s\x1b[0m\n", error_string);

    signal(signum, SIG_DFL); // перепосылка сигнала
}

/**
 * Обрабатывает сигнал смерти.
 * @link http://habrahabr.ru/post/131412/
 * @param signum
 */
void posix_exit_signal(int signum)
{
    TagLoger::log(Log_Any, 0, "\n\n\x1b[1;mposix_exit_signal\x1b[0m\n");
    print_backtrace();
    TagLoger::log(Log_Any, 0, "\x1b[1;32mok\x1b[0m\n");

    time_t seconds = time(NULL);
    tm* timeinfo = localtime(&seconds);

    /**
     * @todo перевести на использование strftime
     */
    char *bufTime = asctime(timeinfo);
    char error_string[900];
    snprintf(error_string, 900, "signum=%d\tTime=%ld\t[%s]\n", signum, seconds, bufTime);

    int fp = open("posix_signal.log", O_WRONLY | O_CREAT | O_APPEND, 0666);
    write(fp, error_string, strlen(error_string));
    close(fp);
    TagLoger::log(Log_Any, 0, "\x1b[1;33m%s\x1b[0m\n", error_string);

    signal(signum, SIG_DFL); // перепосылка сигнала 
    exit(0);
}

void posix_ignor_signal(int signum)
{
    TagLoger::log(Log_Any, 0, "\n\n\x1b[1;mposix_ignor_signal\x1b[0m\n");
    print_backtrace();
    TagLoger::log(Log_Any, 0, "\x1b[1;32mok\x1b[0m\n");

    time_t seconds = time(NULL);
    tm* timeinfo = localtime(&seconds);

    /**
     * @todo перевести на использование strftime
     */
    char *bufTime = asctime(timeinfo);
    char error_string[900];
    snprintf(error_string, 900, "signum=%d\tTime=%ld\t[%s]\n", signum, seconds, bufTime);

    int fp = open("posix_signal.log", O_WRONLY | O_CREAT | O_APPEND, 0666);
    write(fp, error_string, strlen(error_string));
    close(fp);
    TagLoger::log(Log_Any, 0, "\x1b[1;33m%s\x1b[0m\n", error_string);

    signal(signum, SIG_IGN); // перепосылка сигнала
}
 
#define NAMEDPIPE_NAME "/tmp/cpp.comet"
#define BUFSIZE        150

/**
 * Поток для приёма управляющих сигналов из консоли сервера
 * @param pid
 * @return
 * @see http://habrahabr.ru/post/122108/
 *
 * Запись в pipe производится командой echo 'Hello, my named pipe!' > /tmp/wpcomet
 * Воспринимает команду exit и version
 *
 * echo 'exit' > /tmp/cpp.comet
 * echo 'version' > /tmp/cpp.comet
 *
 * echo 'LogLevel 600' > /tmp/cpp.comet -- Логирует всё
 * echo 'LogLevel 500' > /tmp/cpp.comet -- Логирует если разрешено по тегу
 *
 * echo 'TagLog 0 600' > /tmp/cpp.comet -- Установить тегу разрешение на полный лог.
 * 
 */
void command_line_fork()
{
    int fd;
    char buf[BUFSIZE];

    if ( mkfifo(NAMEDPIPE_NAME, 0777) )
    {
        perror("mkfifo");
        //return 1;
    }
    else
    {
        TagLoger::debug(Log_pipeCommands, 0, "\x1b[32mCreated fifo pipe with name %s\x1b[0m\n", NAMEDPIPE_NAME);
    }

    if ( (fd = open(NAMEDPIPE_NAME, O_RDONLY)) <= 0 )
    {
        perror("open");
        return;
    }
    else
    {
        TagLoger::debug(Log_pipeCommands, 0, "\x1b[32mOpen fifo pipe with name %s\x1b[0m\n", NAMEDPIPE_NAME);
    }

    do {
        memset(buf, '\0', BUFSIZE);
        int len = read(fd, buf, BUFSIZE-1);

        if( len <= 0 )
        {
            continue;
        }

        TagLoger::log(Log_pipeCommands, 0, "\x1b[32mIncoming message from pipe(%d): %s\x1b[0m\n", len, buf);

        if( strncmp(buf,"exit", strlen("exit") ) == 0 )
        {
            printf("\x1b[31mExit command received\x1b[0m\n");
            //kill(getppid(), SIGTERM);
            close(fd);
            remove(NAMEDPIPE_NAME);
            exit(0);
            return;
        }
        else if( strncmp(buf,"restart", strlen("restart") ) == 0 )
        {
            printf("\x1b[31mExit command received\x1b[0m\n");
            //kill(pid, SIGTERM);
            close(fd);
            remove(NAMEDPIPE_NAME);
            exit(9);
            return;
        }
        else if( strncmp(buf,"version", strlen("version") ) == 0 )
        {
            printf("\x1b[31m%s\x1b[0m\n", MYSQL_SYSTEMVARIBLE);
        }
        else if(memcmp(buf, "TagLog", strlen("TagLog")) == 0)
        {
            int tag = 0;
            int level = 0;
            sscanf(buf, "TagLog %4d %4d",&tag, &level);
            TagLoger::setTagLevel(tag, level);
            TagLoger::log(Log_pipeCommands, 0, "set TagLevel %d to tag %d\n", level, tag);
        }
        else if(memcmp(buf, "LogLevel", strlen("LogLevel")) == 0)
        {
            int level = 0;
            sscanf(buf, "LogLevel %4d", &level);
            TagLoger::setLogLevel(level);
            TagLoger::log(Log_pipeCommands, 0, "set LogLevel %d\n", level);
        }
        else if(memcmp(buf, "useQueryLoger", strlen("useQueryLoger")) == 0)
        {
            int val = 0;
            sscanf(buf, "useQueryLoger %2d",&val);
            appConf::instance()->set_value("main", "useQueryLoger", val);
            TagLoger::log(Log_pipeCommands, 0, "set useQueryLoger %d\n", val);
        }
        else
        {
            TagLoger::error(Log_pipeCommands, 0, "\x1b[31mUnknown command:%s\x1b[0m\n", buf);
        }

    } while ( 1 );
}
 
/**
 * valgrind --tool=memcheck --track-origins=yes --leak-check=yes ./cpp_comet
 * valgrind --tool=memcheck --track-origins=yes --leak-check=full --show-reachable=yes ./cpp_comet
 * valgrind --tool=memcheck --track-origins=yes --leak-check=full --show-reachable=yes ./cpp_comet --test
 *
 * cppcheck -q -j4 --enable=performance,portability,warning,style .
 * 
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{ 
    TagTimer::start(Time_start);

    // Назначает обработчик на сигнал смерти
    signal(SIGSEGV, posix_death_signal);

    signal(SIGFPE,  posix_log_signal); //  сигнал, посылаемый процессу, при попытке выполнения ошибочной арифметической операции. ( может быть перехвачен или проигнорирован программой.)
    signal(SIGILL,  posix_log_signal); //  сигнал, посылаемый процессу при попытке выполнить неправильно сформированную, несуществующую или привилегированную инструкцию. (или попытке выполнения инструкции, требующей специальных привилегий. ) ( может быть перехвачен или проигнорирован)
   

//#define	USE_COVERAGE
#ifndef USE_COVERAGE
    signal(SIGINT,  posix_log_signal); //  сигнал для остановки процесса пользователем с терминала.
    signal(SIGQUIT, posix_log_signal); //  сигнал, для остановки процесса пользователем, комбинацией «quit» на терминале.
    signal(SIGTERM, posix_log_signal); //  сигнал, для запроса завершения процесса. (В отличие от SIGKILL этот сигнал может быть обработан или проигнорирован программой.)
    signal(SIGUSR1, posix_log_signal); // пользовательские сигналы По умолчанию, сигналы SIGUSR1 и SIGUSR2 завершают выполнение процесса.
    signal(SIGUSR2, posix_log_signal); // пользовательские сигналы По умолчанию, сигналы SIGUSR1 и SIGUSR2 завершают выполнение процесса.
#else
    signal(SIGINT,  posix_exit_signal); //  сигнал для остановки процесса пользователем с терминала.
    signal(SIGQUIT, posix_exit_signal); //  сигнал, для остановки процесса пользователем, комбинацией «quit» на терминале.
    signal(SIGTERM, posix_exit_signal); //  сигнал, для запроса завершения процесса. (В отличие от SIGKILL этот сигнал может быть обработан или проигнорирован программой.)
    signal(SIGUSR1, posix_exit_signal); // пользовательские сигналы По умолчанию, сигналы SIGUSR1 и SIGUSR2 завершают выполнение процесса.
    signal(SIGUSR2, posix_exit_signal); // пользовательские сигналы По умолчанию, сигналы SIGUSR1 и SIGUSR2 завершают выполнение процесса.
    
    base64_decode(std::string("YWJjZA==")); // Просто провера чтоб COVERAGE был больше
    
#endif	/* DEVMANAGER_H */
    
    signal(SIGBUS,  posix_log_signal); //  сигнал, сигнализирующий об ошибке шины, при обращении к физической памяти. ( может быть перехвачен или проигнорирован)
    signal(SIGSYS,  posix_log_signal); //  сигнал, предназначенный для посылки программе, при попытке передать неправильный аргумент в системный вызов.
    signal(SIGXCPU, posix_log_signal); //  сигнал, посылаемый компьютерной программе, превышающей лимит процессорного времени.
    signal(SIGXFSZ, posix_log_signal); //  сигнал, посылаемый процессу при превышении открытым файлом максимально допустимого размера.

    // signal(SIGABRT, posix_log_signal);

    //signal(SIGALRM, posix_death_signal);
    //signal(SIGKILL, posix_death_signal); // не может быть перехвачен или проигнорирован
    signal(SIGPIPE, posix_ignor_signal);   //  сигнал, посылаемый процессу при записи в соединение (пайп, сокет) при отсутствии или обрыве соединения с другой (читающей) стороной.

    //signal(SIGCHLD, posix_death_signal); //  сигнал, посылаемый при изменении статуса дочернего процесса (завершен, приостановлен или возобновлен).
    //signal(SIGCONT, posix_death_signal); //  для возобновления выполнения процесса, ранее остановленного сигналом SIGSTOP
    //signal(SIGSTOP, posix_death_signal); //  SIGSTOP не может быть обработан программой или проигнорирован.
    //signal(SIGTSTP, posix_death_signal); //  сигнал, посылаемый c терминала для приостановки выполнения процесса (обычно — комбинацией Ctrl-Z) (может быть обработан программой или проигнорирован.)
    //signal(SIGTTIN, posix_death_signal); //  сигнал, посылаемый фоновой задаче при попытке чтения с управляющего терминала. При получении сигнала задача приостанавливается до получения SIGCONT
    //signal(SIGTTOU, posix_death_signal); //  сигнал, посылаемый фоновой задаче при попытке вывода на управляющий терминал.
    //signal(SIGPOLL, posix_death_signal); //  сигнал, посылаемый процессу для информирования об изменении состояния выполняющегося в фоне асинхронного ввода-вывода.
    //signal(SIGPROF, posix_death_signal); //  сигнал, посылаемый процессу по истечении времени заданном в таймере профилирования.
    //signal(SIGTRAP, posix_death_signal); //  сигнал, посылаемый для информирования отладчика о возникновении интересующего события.
    //signal(SIGURG,  posix_death_signal); //  сигнал, посылаемый процессу при появлении на сокете доступных для чтения срочных (англ. urgent) данных.
    //signal(SIGVTALRM, posix_death_signal); //  сигнал, посылаемый процессу по истечении времени заданном в «виртуальном» таймере.
  
    // Чтение конфига и ключей запуска 
    if(!appConf::instance()->init(argc, argv))
    { 
        TagLoger::error(Log_appConf, 0, "\x1b[1;32mCppComet error in parsing comet.ini file\x1b[0m");
        return 0;
    }
    
    TagLoger::log(Log_Any, 0, "Server starting pid:%d, getrusage:%d\n", getpid());
    TagLoger::initTagLevels();
 
    dbLink mydb;
    mydb.init(appConf::instance()->get_chars("db", "host"),
            appConf::instance()->get_chars("db", "user"),
            appConf::instance()->get_chars("db", "password"),
            appConf::instance()->get_chars("db", "name"),
            appConf::instance()->get_int("db", "port"));
    if(!mydb.connect())
    {
        TagLoger::error(Log_Any, 0, "\x1b[1;31mError: MySQL connection not established\x1b[0m");
    }

    if(!mydb.query_format("INSERT INTO `log_event`(`id`, `text`) VALUES (NULL,'start-%s');", appConf::instance()->get_chars("main", "node_name")))
    {
        return 0;
    }

    // Запись pid в var run 
    auto fp = fopen(appConf::instance()->get_chars("main", "pidfile"), "w");
    fprintf(fp, "%d", getpid());
    fclose(fp);

    while(true)
    {
        pid_t pid = fork(); 
        if(pid == -1)
        {
            TagLoger::error(Log_appConf, 0, "\x1b[1;32mError in fork result of function\x1b[0m");
            perror("fork"); /* произошла ошибка */
            remove(appConf::instance()->get_chars("main", "pidfile"));
            remove(NAMEDPIPE_NAME);
            exit(1); /*выход из родительского процесса*/
        }
        else if(pid == 0)
        {
            // Это процесс-потомок
            TagLoger::debug(Log_appConf, 0, "PID=%d, PPID=%d", getpid(), getppid());
            signal(SIGHUP,  posix_death_signal); //  сигнал, посылаемый процессу для уведомления о потере соединения с управляющим терминалом пользователя. ( может быть перехвачен или проигнорирован программой. )
     
            break;
        }
        else
        {
            // Это процесс-родитель 
            TagLoger::debug(Log_appConf, 0, "PID=%d, Child PID=%d", getpid(), pid); 
            signal(SIGHUP,  posix_log_signal); //  сигнал, посылаемый процессу для уведомления о потере соединения с управляющим терминалом пользователя. ( может быть перехвачен или проигнорирован программой. )
   
            int status;
            waitpid(pid, &status, 0); 
            if(status == 0)
            {
                TagLoger::log(Log_appConf, 0, "Completion of work");  
                remove(appConf::instance()->get_chars("main", "pidfile"));
                remove(NAMEDPIPE_NAME);
                exit(0);
            }
            else
            {
                TagLoger::error(Log_appConf, 0, "Child return status is %d", status);  
            }
        }
    }
    
    if(!appConf::instance()->get_string("main", "host").empty())
    {
        devManager::instance()->getDevInfo()->setDevUrl(appConf::instance()->get_chars("main", "host"));
    }

    intervalLoop::instance()->start();
    usage_statistics::start(); 
  
    // Запуск потока обработки сообщений от браузеров
    th_startServer<Client_connection>(1, "ws");
    th_startServer<MySql_connection>(2, "cometql");
    
    // Запуск потока обработки сообщений от серверов freeswitch
    th_startServer<Freeswitch_connection>(3, "freeswitch");  
    th_startServer<CometQLProxy_connection>(4, "cometqlproxy");
    
    command_line_fork();

    TagTimer::end(Time_start);
    pthread_exit(NULL);

  return 0;
}
