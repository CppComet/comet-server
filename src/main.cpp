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
 

/**
 * ЗАЛЕПА
 */
time_t tcpServer_benchmark::start_time = time(0);
 
/**
 * Для обработки сообщений от браузеров
 * @todo Вынести в конфигурационный файл настройки логирования и прочее
 * @param threadid
 * @return
 */
void th_clientServer(int threadid)
{
    if(appConf::instance()->client_thread_num == 0)
    {
        return;
    }

    TagLoger::log(Log_Any, 0, "Hello World! It's me, th_clientServer #%ld!\n", threadid);

    tcpServer <Client_connection>::instance()->id = 1;
    tcpServer <Client_connection>::instance()->set_thread_num(appConf::instance()->client_thread_num);
    tcpServer <Client_connection>::instance()->epoll_size = appConf::instance()->client_epoll_size;
    tcpServer <Client_connection>::instance()->backlog = appConf::instance()->client_backlog;



    if(appConf::instance()->client_benchmark > 0)
    {
        tcpServer <Client_connection>::instance()->benchmark = 1;
        tcpServer <Client_connection>::instance()->bm.stat_interval = appConf::instance()->client_benchmark;
    }
    else
    {
        tcpServer <Client_connection>::instance()->benchmark = 0;
    }

    tcpServer <Client_connection>::instance()->start(appConf::instance()->client_ip, appConf::instance()->client_port, "ClientServer");
    return;
}

/**
 * Для обработки сообщений от серверов
 * @todo Вынести в конфигурационный файл настройки логирования и прочее
 * @param threadid
 * @return
 */
void th_MySqlServer(int threadid)
{
    if(appConf::instance()->mysql_thread_num == 0)
    {
        return;
    }

    TagLoger::log(Log_Any, 0, "Hello World! It's me, th_ServerServer #%ld!\n", threadid);

    MySql_connection::initTables();


    pthread_mutex_init(&MySql_connection::QLParsing_mutex,NULL);

    tcpServer <MySql_connection>::instance()->id = 3;
    tcpServer <MySql_connection>::instance()->set_thread_num(appConf::instance()->mysql_thread_num);
    tcpServer <MySql_connection>::instance()->epoll_size = appConf::instance()->mysql_epoll_size;
    tcpServer <MySql_connection>::instance()->backlog = appConf::instance()->mysql_backlog;

    if(appConf::instance()->mysql_benchmark > 0)
    {
        tcpServer <MySql_connection>::instance()->benchmark = 1;
        tcpServer <MySql_connection>::instance()->bm.stat_interval = appConf::instance()->mysql_benchmark;
    }
    else
    {
        tcpServer <MySql_connection>::instance()->benchmark = 0;
    }

    tcpServer <MySql_connection>::instance()->start(appConf::instance()->mysql_ip, appConf::instance()->mysql_port, "MySqlServer");

    MySql_connection::addIntervalRoutine();

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
    TagLoger::log(Log_Any, 0, "\n\n\x1b[1;posix_log_signal\x1b[0m\n");
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

    signal(signum, SIG_DFL); // перепосылка сигнала
}

void posix_ignor_signal(int signum)
{
    TagLoger::log(Log_Any, 0, "\n\n\x1b[1;posix_ignor_signal\x1b[0m\n");
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

    signal(signum, SIG_IGN); // перепосылка сигнала
}

#define NAMEDPIPE_NAME "/tmp/star.comet"
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
 * echo 'exit' > /tmp/star.comet
 * echo 'version' > /tmp/star.comet
 *
 * echo 'LogLevel 600' > /tmp/star.comet -- Логирует всё
 * echo 'LogLevel 500' > /tmp/star.comet -- Логирует если разрешено по тегу
 *
 * echo 'TagLog 0 600' > /tmp/star.comet -- Установить тегу разрешение на полный лог.
 *
 * @todo было бы круто добавить команды изменения уровня логирования на лету
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
        TagLoger::log(Log_pipeCommands, 0, "\x1b[32mСоздан fifo pipe с именем %s\x1b[0m\n", NAMEDPIPE_NAME);
    }

    if ( (fd = open(NAMEDPIPE_NAME, O_RDONLY)) <= 0 )
    {
        perror("open");
        return;
    }
    else
    {
        TagLoger::log(Log_pipeCommands, 0, "\x1b[32mОткрыт fifo pipe с именем %s\x1b[0m\n", NAMEDPIPE_NAME);
    }

    do {
        memset(buf, '\0', BUFSIZE);
        int len = read(fd, buf, BUFSIZE-1);

        if( len <= 0 )
        {
            continue;
        }

        TagLoger::log(Log_pipeCommands, 0, "\x1b[32mВходящее сообщение через pipe(%d): %s\x1b[0m\n", len, buf);

        if( strncmp(buf,"exit", strlen("exit") ) == 0 )
        {
            printf("\x1b[31mПолучена команда выхода\x1b[0m\n");
            //kill(pid, SIGTERM);
            close(fd);
            remove(NAMEDPIPE_NAME);
            exit(0);
            return;
        }
        else if( strncmp(buf,"version", strlen("version") ) == 0 )
        {
            printf("\x1b[31mCPPcomet v.1.38\x1b[0m\n");
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
            appConf::instance()->useQueryLoger = val;
            TagLoger::log(Log_pipeCommands, 0, "set useQueryLoger %d\n", val);
        }
        else
        {
            TagLoger::error(Log_pipeCommands, 0, "\x1b[31mНе известная команда:%s\x1b[0m\n", buf);
        }

    } while ( 1 );
}

#include "devManager.h"
/**
 * valgrind --tool=memcheck --track-origins=yes --leak-check=yes ./cpp_comet
 * valgrind --tool=memcheck --track-origins=yes --leak-check=full --show-reachable=yes ./cpp_comet
 * valgrind --tool=memcheck --track-origins=yes --leak-check=full --show-reachable=yes ./cpp_comet --test
 *
 * cppcheck -q -j4 --enable=performance,portability,warning,style .
 *
 * systemVaribleName
 * seregatv.ru
 * mysql -hn2-app.comet.su -u1 -pr3VYAQPxxsgu6P9FxkqH9SjELFEK0PGDWCaY1xCd0xkhmBDHh8hlPV0jgZhhPPBg -DCometQL_v1 -P3307
 * mysql -hn3-app.comet.su -u1 -pr3VYAQPxxsgu6P9FxkqH9SjELFEK0PGDWCaY1xCd0xkhmBDHh8hlPV0jgZhhPPBg -DCometQL_v1 -P3307
 * mysql -happ.comet-server.ru -u2 -pECMp5sWkR6FOjepgT5UN8gDx1dbNLUM3YBriK1KoVhn1MSmc6P7J0k40qhyoA39I -DCometQL_v1 -P3306
 * mysql -hchat.inparadise.info -uroot -pjT7P20e5EUd8DXK3RgIJrthrDGrtvUzUqFc2ZEgL4LSb5Ml7nwGdfVmS5N3o0zAx -DCometQL_v1 -P3308
 *
 * delete FROM `log_query` where query = "show status" or  query = "show global status"
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{
    /*
    CP<fastListItem<int> > next1(NULL);
    printf("isNULL %d\n", (int)next1.isNULL());

    CP<fastListItem<int> > next2;
    printf("isNULL %d\n", (int)next2.isNULL());


    fastList<int> map;

    map.append(1);
    map.append(100);
    map.append(100);
    map.append(2);

    int i = 0;
    auto it = map.first;
    while(it)
    {
        printf("%d - %d\n", i, it->data);
        i++;
        it = it->Next();
    }

    map.remove(100);
    map.remove(100);
    map.remove(200);
    i = 0;
    it = map.first;
    while(it)
    {
        printf("%d - %d\n", i, it->data);
        i++;
        it = it->Next();
    }


    return 0;*/

    TagTimer::start(Time_start);
    TagLoger::log(Log_Any, 0, "Старт сервера pid:%d, getrusage:%d\n", getpid());

    // Назначает обработчик на сигнал смерти
    signal(SIGSEGV, posix_death_signal);

    signal(SIGFPE,  posix_log_signal); //  сигнал, посылаемый процессу, при попытке выполнения ошибочной арифметической операции. ( может быть перехвачен или проигнорирован программой.)
    signal(SIGHUP,  posix_log_signal); //  сигнал, посылаемый процессу для уведомления о потере соединения с управляющим терминалом пользователя. ( может быть перехвачен или проигнорирован программой. )
    signal(SIGILL,  posix_log_signal); //  сигнал, посылаемый процессу при попытке выполнить неправильно сформированную, несуществующую или привилегированную инструкцию. (или попытке выполнения инструкции, требующей специальных привилегий. ) ( может быть перехвачен или проигнорирован)
    signal(SIGINT,  posix_log_signal); //  сигнал для остановки процесса пользователем с терминала.
    signal(SIGQUIT, posix_log_signal); //  сигнал, для остановки процесса пользователем, комбинацией «quit» на терминале.
    signal(SIGTERM, posix_log_signal); // сигнал, для запроса завершения процесса. (В отличие от SIGKILL этот сигнал может быть обработан или проигнорирован программой.)

    signal(SIGUSR1, posix_log_signal); // пользовательские сигналы По умолчанию, сигналы SIGUSR1 и SIGUSR2 завершают выполнение процесса.
    signal(SIGUSR2, posix_log_signal); // пользовательские сигналы По умолчанию, сигналы SIGUSR1 и SIGUSR2 завершают выполнение процесса.

    signal(SIGBUS,  posix_log_signal); //  сигнал, сигнализирующий об ошибке шины, при обращении к физической памяти. ( может быть перехвачен или проигнорирован)
    signal(SIGSYS,  posix_log_signal); // сигнал, предназначенный для посылки программе, при попытке передать неправильный аргумент в системный вызов.
    signal(SIGXCPU, posix_log_signal); //  сигнал, посылаемый компьютерной программе, превышающей лимит процессорного времени.
    signal(SIGXFSZ, posix_log_signal); //  сигнал, посылаемый процессу при превышении открытым файлом максимально допустимого размера.

    // signal(SIGABRT, posix_log_signal);

    //signal(SIGALRM, posix_death_signal);
    //signal(SIGKILL, posix_death_signal);// не может быть перехвачен или проигнорирован
    signal(SIGPIPE, posix_ignor_signal); //  сигнал, посылаемый процессу при записи в соединение (пайп, сокет) при отсутствии или обрыве соединения с другой (читающей) стороной.

    //signal(SIGCHLD, posix_death_signal); // сигнал, посылаемый при изменении статуса дочернего процесса (завершен, приостановлен или возобновлен).
    //signal(SIGCONT, posix_death_signal);  //  для возобновления выполнения процесса, ранее остановленного сигналом SIGSTOP
    //signal(SIGSTOP, posix_death_signal); // SIGSTOP не может быть обработан программой или проигнорирован.
    //signal(SIGTSTP, posix_death_signal); //  сигнал, посылаемый c терминала для приостановки выполнения процесса (обычно — комбинацией Ctrl-Z) (может быть обработан программой или проигнорирован.)
    //signal(SIGTTIN, posix_death_signal); //  сигнал, посылаемый фоновой задаче при попытке чтения с управляющего терминала. При получении сигнала задача приостанавливается до получения SIGCONT
    //signal(SIGTTOU, posix_death_signal); //  сигнал, посылаемый фоновой задаче при попытке вывода на управляющий терминал.
    //signal(SIGPOLL, posix_death_signal); //  сигнал, посылаемый процессу для информирования об изменении состояния выполняющегося в фоне асинхронного ввода-вывода.
    //signal(SIGPROF, posix_death_signal); //  сигнал, посылаемый процессу по истечении времени заданном в таймере профилирования.
    //signal(SIGTRAP, posix_death_signal); //  сигнал, посылаемый для информирования отладчика о возникновении интересующего события.
    //signal(SIGURG,  posix_death_signal); //  сигнал, посылаемый процессу при появлении на сокете доступных для чтения срочных (англ. urgent) данных.
    //signal(SIGVTALRM, posix_death_signal); //  сигнал, посылаемый процессу по истечении времени заданном в «виртуальном» таймере.


    // Чтение конфига и ключей запуска
    appConf::instance()->initFromFile("comet.conf");
    appConf::instance()->init(argc, argv);

    if(appConf::instance()->isHelp) return 0; // Выйти если был ключ --help
    appConf::instance()->print();

    /* Временно отключаем mysql*/
    dbLink mydb;
    mydb.init(appConf::instance()->db_host, appConf::instance()->db_user, appConf::instance()->db_pw, appConf::instance()->db_name, appConf::instance()->db_port);
    if(!mydb.connect())
    {
        TagLoger::error(Log_Any, 0, "\x1b[1;31mMySQL соединение не уставновлено\x1b[0m");
    }

    if(!mydb.query_format("INSERT INTO `log_event`(`id`, `text`) VALUES (NULL,'start-%s');", appConf::instance()->node_name))
    {
        return 0;
    }

    /*stm_users_queue_insert users_queue_insert;
    users_queue_insert.prepare(mydb);
    users_queue_insert.insert("start1", 10, 2, 3, "event1", "message1", strlen("message1"));
    users_queue_insert.insert("start2", 20, 2, 3, "event2", "message2", strlen("message2"));
    users_queue_insert.insert("start3", 30, 2, 4, "event3", "message3", strlen("message3"));
    users_queue_insert.insert("start4", 40, 2, 4, "event4", "message4", strlen("message4"));
    */

    /*
    stm_users_queue_select users_queue_select;
    users_queue_select.prepare(mydb);

    users_queue_select.select(2, 3, 10);
    int i =0;
    while(!users_queue_select.fetch())
    {
        printf("%d: %d - %s\n", i, users_queue_select.result_message_length, users_queue_select.result_message);
        i++;
    }
    users_queue_select.free();
    return 0;*/


    //stmMapper stmq(mydb.getLink());
    //stmq.call(1, "query text", strlen("query text"));

    /* @FixMe решить вопрос экранирования данных  - mysql_real_escape_string();
    if(!mydb.query("INSERT INTO `log_query`(`id`, `dev_id`, `query`) VALUES (NULL, %d, '%s');", "start"))
    {
        return 0;
    }*/

    #ifdef monoURL
        devManager::instance()->getDevInfo()->setDevUrl(monoURL);
    #endif

    intervalLoop::instance()->start();

    // Запуск потока обработки сообщений от браузеров
    //pthread_t c_threads;
    //long c_t = 100;
    //pthread_create(&c_threads, NULL, th_clientServer, (void *)c_t);
    th_clientServer(100);

    //pthread_t ql_sthreads;
    //long ql_t = 300;
    //pthread_create(&ql_sthreads, NULL, th_MySqlServer, (void *)ql_t);
    th_MySqlServer(300);
 
    command_line_fork();

    TagTimer::end(Time_start);
    pthread_exit(NULL);

  return 0;
}
