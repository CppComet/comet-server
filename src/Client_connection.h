// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* 
 * File:   Client_connection.h
 * Author: levha
 *
 * Created on 11 Май 2011 г., 23:58
 */

#ifndef CLIENT_CONNECTION_H
#define	CLIENT_CONNECTION_H

   #include <stdio.h>
   #include <cstdlib>
   #include <strings.h>
   #include <string.h>
   #include <sys/socket.h>
   #include <vector>
   #include <map>

   #include "main.h"
   #include "CP.h"
   #include "connection.h"

class Client_connection;
   #include "user_index.h"


#define	REQUEST_NULL            0
#define	REQUEST_OPTIONS         1
#define	REQUEST_GET             2
#define	REQUEST_LONG_POLLING    3
#define	REQUEST_POST            3 // Все LONG_POLLING отправляются как POST запросы
#define	REQUEST_WS              4

/**
 * Максимально количество подписок на каналы
 * @todo вынести в конфигурационный файл | а лучше и экономичнее выделять память по мере надобности
 */
#define	MAX_SUBSCRIPTION_PIPE   32
#define	SUBSCRIPTION_DATA_LEN   MAX_SUBSCRIPTION_PIPE*(PIPE_NAME_LEN+4)

#define	MESSAGE_TEXT            0x81
#define	MESSAGE_CLOSE           0x88

#define	USER_UUID_LEN      32

#define	USER_AGENT_LEN     250
#define	USER_HOST_LEN      250
#define	USER_LANGUAGE_LEN  64

#define GETBIT(x,pos)   ( ((x) & ( 0x1 << (pos) )) !=0 )

/**
 * Обрабатывает сообщения от браузеров
 */
class Client_connection:public connection
{
    friend class tcpServer<Client_connection>;
      
    /**
     * Подписки на каналы
     * @note Содержит указатели на память принадлежащию масиву subscriptions_data
     * 
     * @todo Заменить на динамически выделяемые данные [opt1 -MAX_SUBSCRIPTION_PIPE*ws_online]
     */
    char *subscriptions[MAX_SUBSCRIPTION_PIPE]; 

    /**
     * Подписки на каналы
     * @todo Заменить на динамически выделяемые данные [opt1 -SUBSCRIPTION_DATA_LEN*ws_online]
     */
    char subscriptions_data[SUBSCRIPTION_DATA_LEN]; 

      
    /**
     * Для хранения фрагментов сообщений которые пришли не полностью.
     */
    bufferController fragment_buf;
    bool hasFragmentInBuffer = false;
      
    /**
     * Отправляет всё что находится в логе канала.
     * @param local_buf
     * @param pipe_name
     * @return
     * 
     * Некоторые каналы могут иметь лог сообщений, если такой лог есть то при подписке на канал отправляются все сообщения из лга.
     * Сообщения лога хранятся в редисе с ключём user_queue_to_dev_{devId}__{pipe_name}
     * Если сообщений больше чем MAX_LOG_LENGTH то новые сообщения затирают наиболее старые
     */
    int send_pipe_log(thread_data* local_buf, char* pipe_name, const char* MarkerName);
    
    /**
     * Возвращает количество подписчиков в канале (доступно только для каналов web_)
     * @param local_buf
     * @param pipe_name
     * @param MarkerName
     * @return 
     */
    int send_pipe_count(thread_data* local_buf, char* pipe_name, const char* MarkerName);
    
    /**
     * Извлекает из очереди сообщения и отправляет их пользователю.
     * Это позволяет повысить надёжность доставки сообщений авторизованым пользователям когда доставка сообщений осуществляется пользователю по его id
     * @param client
     * @param len
     * @param local_buf
     * @return 
     */
    int msg_queue_send(int client, int len, thread_data* local_buf);
       
    /**
     * Обрабатывает сообщения полученые из JS API по websockets 
     * Выполняется при подключении и рукопожатии websockets
     * Производит авторизацию и разбор прочего пришедшего из заголовка.
     * @param client
     * @param len
     * @param local_buf
     * @tudo Перепроверить и поправить механизм для работы с строкой сессии разной длины
     */
    int web_socket_request(int client, int len, thread_data* local_buf);
    int options(int client, int len, thread_data* local_buf);
    int get_request(int client, int len, thread_data* local_buf);
    int get_info_request(int client, int len, thread_data* local_buf);
    int get_favicon_request(int client, int len, thread_data* local_buf);
    int http404_answer(int client, int len, thread_data* local_buf);
    int http403_answer(int client, int len, thread_data* local_buf);
    int get_custom_request(int client, int len, thread_data* local_buf);
    int cultivate_custom_request(int client, const char* message, int len, thread_data* local_buf);
    
    int web_socket_request_message(int client, int len, thread_data* local_buf);
    int web_socket_request_message(int client, int len, thread_data* local_buf, int start_position);
    
    /**
     * Подписка на события, регистрирует подписки на события
     * @param client
     * @param len
     * @return
     * @todo Перепроверить на предмет отсутсвия ограничений, запретить использование пробелов и вообще по шаблону [A-za-z0-9]
     * @note Внимание: event_data+len должно нахадится в пределах выделеной памяти под буфер для входящих сообщений
     */ 
    int ws_subscription(thread_data* local_buf, char* event_data,int client, int len);
    
    
    /**
     * Отписка клиента от событий
     * @todo Реализовать удаление каналов на которых нет подписчиков
     */
    int un_subscription(thread_data* local_buf);
     
    int log_statistics(thread_data* local_buf, const char* event_data,int client, int len); 
    int web_pipe_msg_v1(thread_data* local_buf, char* event_data,int client, int len);
    int web_pipe_msg_v2(thread_data* local_buf, char* event_data,int client, int len);
    int get_pipe_log(thread_data* local_buf, char* event_data,int client, int len);
    int get_user_last_online_time(thread_data* local_buf, char* event_data,int client, int len);
    int get_pipe_count(thread_data* local_buf, char* event_data,int client, int len);
    int track_pipe_users(thread_data* local_buf, char* event_data,int client, int len);
    int web_user_data(thread_data* local_buf, char* event_data,int client, int len);
     
    int cgi_call(thread_data* local_buf, char* event_data,int client, int len);
    
    bool online_incr(thread_data* local_buf);
    bool online_decr(thread_data* local_buf);
    
    /**
     * Проверка на валидность имени канала
     * @param pipe_start
     * @return Указатель на первый символ после имени канала и разделителя
     */
    char* checking_channel_name(thread_data* local_buf, const char* pipe_start);
    
    /**
     * Проверка на валидность имени события
     * @param pipe_start
     * @return Указатель на первый символ после имени события и разделителя
     */
    char* checking_event_name(thread_data* local_buf, const char* event_name);
     
    static int request_mutex_init;
     
     
    
    bool isAuthUser = false;  
    int connection_type = REQUEST_NULL;
    
    int client_major_version = 0;
    int client_minor_version = 0;
    
    char* parse_url(int client, int len, thread_data* local_buf);
    int web_write_error(const char* text, thread_data* local_buf);
    int web_write_error(const char* text, int code, thread_data* local_buf);
    int web_socket_receive(thread_data* local_buf);
     
  public:
       
    static int log_level; 
    
    /**
     * id пользователя если он авторизован или ноль если не авторизован.
     */
    int web_user_id = 0; 
    int web_user_dev_id = 0;
    
    /**
     * Самоназвание клиента (для случаев если надо идентифицировать и дифференцировать не авторизованных пользователей)
     * @todo Заменить на динамически выделяемые данные + добавить параметр на отключение парсинга этой строкм [opt1 -USER_UUID_LEN*ws_online]
     */
    char web_user_uuid[USER_UUID_LEN+1];
    
    /** 
     * @todo Заменить на динамически выделяемые данные + добавить параметр на отключение парсинга этой строкм [opt1 -USER_UUID_LEN*ws_online]
     */
    char web_user_agent[USER_AGENT_LEN+1];
    
    /** 
     * @todo Заменить на динамически выделяемые данные + добавить параметр на отключение парсинга этой строкм [opt1 -USER_UUID_LEN*ws_online]
     */
    char web_user_host[USER_HOST_LEN+1];
    
    /** 
     * @todo Заменить на динамически выделяемые данные + добавить параметр на отключение парсинга этой строкм [opt1 -USER_UUID_LEN*ws_online]
     */
    char web_user_language[USER_LANGUAGE_LEN+1];
     
    /**
     * Максимальное количество событий на которые возможно подписатся
     */
    static int max_subscription_event;
    
    /**
     * Предельная длина аргкментов к событию
     */
    static int max_arg_length;
        
    /**
     * Устанавливает размер Dev индекса
     * @return 
     */
    static void setDevIndexSize(int size);
      
    Client_connection();  
    virtual ~Client_connection();
     
    int request(int client, int len, thread_data* local_buf);
    int message(thread_data* local_buf, const char* msg); 
    int message(thread_data* local_buf, const char* msg, const char* name);
    int message(thread_data* local_buf, const char* msg, const char* name, const char message_type, const char* server_data);
    
    /**
     * Отправляет короткие до 127 символов сообщения по вебсокету
     * @param local_buf
     * @param text Текст сообщения
     */
    int short_ws_message(thread_data* local_buf, const char* text);
    
    /**
     * Отправляет короткие до 127 символов сообщения по вебсокету
     * @param local_buf
     * @param text Текст сообщения
     * @param msg_type Тип сообщения 
     */
    int short_ws_message(thread_data* local_buf, const char* text,int msg_type);
    
    
    /**
     * Устанавливает ствтус соединения
     */
    int set_online(thread_data* local_buf );
    int set_offline(thread_data* local_buf );
     
};


#endif	/* CLIENT_CONNECTION_H */

