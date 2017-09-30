// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   Client_connection.h
 * Author: levha
 *
 * Created on 01 Май 2015 г., 13:20
 */

#include <stdio.h>
#include <cstdlib>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>

#include "CP.h"
#include "connection.h"


#include "tcpServer.h"


#include <vector>
#include <map>

#include "MySqlProtocol.h"
#include "CometQL.h"
#include "mystring.h" 

class CometQLProxy_connection;
class MySql_connection;

#include "MySql_connection.h" 

#ifndef COMETQLPROXY_CONNECTION_H
#define	COMETQLPROXY_CONNECTION_H
   
using namespace std;
 
/**
 * Класс одного соединения с сервером
 * Содержит функции обработки запросов от бекенда
 * 
 * Для кластеризации мы не ожидаем ответ от оповещаемых нод при вставке данных
 * А просто говорим ок ещё до того как все ноды кластера были оповещены о запросе.
 * 
 * Так как конкретная нода на вставке тоже может вернуть ответ до реального выполнения запроса
 * то запросы будут отдаватся быстро а код потом ещё будет работать
 * 
 * Для повышения скорости инсертов можно иметь более одного соединения между каждой из нод кластера
 * Тогда два подряд запроса будут выполнятся параллельно
 * 
 * Свой механизм проксирования запросов позволил бы переотправлять запросы в слачии ошибки
 * 
 * Два варианта кластерезации:
 *   - Высокая доступность 
 *   - Высокая производительность
 * 
 * Высокая доступность: (Реализован)
 * реализуется путём работы двух и более серверов с общей базой данных
 * или кластером баз данных в режиме мастер-мастер. 
 * А отправка запросов осуществляется на любой из серверов, можно через haproxy
 * Js подключается к обоим серверам и принимает сообщения от них двоих.
 * 
 * Плюсы: если вырубить N - 1 серверов из кластера размером N серверов работа системы сохранится
 * Минусы: избыточная нагрузка, система выдержит не более чем самый слабый из серверов
 * 
 * 
 * 
 * Высокая производительность:
 * Реализуется кластерезацией на уровне комет сервера
 * Все запросы отправляются на все сервера кластера в не зависимости от того на какой из серверов запрос поступил.
 * 
 * Бд у всех серверов должна быть общей или быть кластером в режиме мастер-мастер
 * Или быть у каждого своей но с каким то дополнительным скриптом для синхронизации после падений
 * 
 * Js подключается к одному из серверов группы.
 *  Режим 1: Пользователи распределяются случайно между доступными серверами в группе
 *  Режим 2: Анонимные пользователи распределяются случайно. Авторизованные равномерно по серверам.
 * 
 * 
 * Режим 1:
 * Плюсы: Нагрузка распределяется равномерно по кластеру, Падение одного сервера отключит часть пользователей максимум на несколько секунд
 * Минусы: Масштабирование НЕ линейное так как cometQL запросы отправляются на все сервера группы
 * 
 * Режим 2:
 * Плюсы: Нагрузка распределяется равномерно по кластеру, масштабирование линейное так как cometQL запросы не отправляются на те сервера группы на которых точно нет адресата
 * Минусы: Падение одного сервера отключит часть пользователей
 * 
 * Режим 3:
 * Комбинация из режима 2 и режима 1 доступная на кластере в четыре ноды и больше
 * 
 * 
 * Оптимизации:
 * Мы можем для инсертов не отдавать результат а говорить принято.
 * Тогда задержки для сторонних сайтов будут минимальные
 * 
 * Для селектов же всё таки придётся обращатся ко всем серверам группы.
 * 
 */
class CometQLProxy_connection:public MySql_connection
{
    friend class tcpServer<CometQLProxy_connection>;
 
public:
       
    CometQLProxy_connection();
    virtual ~CometQLProxy_connection();
 
protected:
   
    int proxy_select(int node, const char* query, thread_data* local_buf, unsigned int PacketNomber);
    int proxy_insert(int node, const char* query, thread_data* local_buf, unsigned int PacketNomber);
    //int proxy_delete(const char* query, thread_data* local_buf, unsigned int PacketNomber); 
    
    int proxy_query(int node, thread_data* local_buf, unsigned int PacketNomber);
    
    virtual int query_router(thread_data* local_buf, int PacketNomber);
};

#endif	/* COMETQLPROXY_CONNECTION_H */

