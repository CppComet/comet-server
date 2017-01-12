// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   Client_connection.cpp
 * Author: levha
 *
 * Created on 11 Май 2011 г., 23:58
 */

#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <openssl/sha.h>

#include "sha1.h"
#include "base64.h"
#include <string>

#include "mystring.h"
#include "Client_connection.h"
#include "MySql_connection.h"
#include "devManager.h"

#include "internalApi.h"

char* getGETdata(const char* c,int& user_id);

int Client_connection::request_mutex_init = 0;
int Client_connection::max_subscription_event = 50;

int Client_connection::log_level = LOG_ALL;

Client_connection::Client_connection():connection(),fragment_buf(appConf::instance()->fragment_buf_size)
{
    //printf("create Client_connection\n");
    bzero(subscriptions_data, SUBSCRIPTION_DATA_LEN);
    bzero(subscriptions, SUBSCRIPTION_DATA_LEN);
    bzero(web_user_uuid, USER_UUID_LEN+1);
    bzero(web_user_agent, USER_AGENT_LEN+1);
    bzero(web_user_host, USER_HOST_LEN+1);
    bzero(web_user_language, USER_LANGUAGE_LEN+1);
}

Client_connection::~Client_connection(){
    //printf("delete Client_connection\n");
}

/**
 * Отписка клиента от событий
 * @todo Реализовать удаление каналов на которых нет подписчиков
 * @fixMe исправить для недопущения редис инекций
 */
int Client_connection::un_subscription(thread_data* local_buf)
{
    for(int i=0; i< MAX_SUBSCRIPTION_PIPE; i++)
    {
        if(subscriptions[i] !=0 )
        {
            TagLoger::log(Log_ClientServer, 0, "un_subscription[%d]%s\n",i, subscriptions[i]);

            // В замен srem_printf
            devInfo* dev = devManager::instance()->getDevInfo();
            CP<Pipe> pipe = dev->findPipe(std::string(subscriptions[i]));
            if(!pipe.isNULL())
            {
                pipe->erase(fd);
            }

            // Удаление инфы о канале из map
            // Будут наверное накапливатся пустые каналы в памяти хеш мепа. Но пока безопаснее так чем удалять.
            // @fixme @todo Придумать решение
            /*pipe->lock();
            if(pipe->empty())
            {
                dev->deletePipe(std::string(subscriptions[i]));
            }
            pipe->unloc();*/

            if(memcmp(subscriptions[i], "track_", 6) == 0)
            {
                // Канал track_* уведомляет автаматически всех подписчиков о изменении их количества
                char addData[300];
                bzero(addData, 300);
                snprintf(addData, 300, "\"user_id\":\"%d\",\"uuid\":\"%s\",\"event_name\":\"unsubscription\"", web_user_id, web_user_uuid);
                internalApi::send_event_to_pipe(local_buf, subscriptions[i], "{\\\"data\\\":\\\"\\\"}", addData);
            }
        }
        else
        {
            break;
        }
    }

    bzero(subscriptions, MAX_SUBSCRIPTION_PIPE);
    return 0;
}



/**
 * Осуществляет подписку на события в каналах
 * @param local_buf
 * @param event_data Указатель на начало списка каналов
 * @param client
 * @param len
 * @return если ошибка то -1  иначе 0
 *
 * @fixMe исправить для недопущения редис инекций
 */
int Client_connection::ws_subscription(thread_data* local_buf, char* event_data,int client, int len)
{
    int event_data_len = strlen(event_data);
    if(event_data_len > SUBSCRIPTION_DATA_LEN)
    {
            TagLoger::error(Log_ClientServer, 0, "\x1b[1;31mСлишком много буковок[%d]\x1b[0m\n", event_data_len);
            message(local_buf, base64_encode( (const unsigned char*) "{\"error\":\"Слишком много буковок\"}",
                       strlen("{\"error\":\"Слишком много буковок\"}") ).data(), "_answer");
            return -1;
    }

    bzero(subscriptions_data, SUBSCRIPTION_DATA_LEN);
    memcpy(subscriptions_data, event_data, event_data_len);
    char* p = subscriptions_data;

    char* start_subscription_name = p;

    int i = 0;
    p--; // Для того чтоб цикл проверки начался с первого символа.

    while( p < start_subscription_name + event_data_len )
    {
        p++;
        if( !AZ09test(*p) )
        {
            *p = 0;
            if(p - start_subscription_name < 2)
            {
                return 0;
            }

            if( i >= MAX_SUBSCRIPTION_PIPE)
            {
                TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mСлишком много подписок [%d]\x1b[0m\n", i);
                message(local_buf, base64_encode( (const unsigned char*) "{\"error\":\"Слишком много подписок\"}",
                       strlen("{\"error\":\"Слишком много подписок\"}") ).data(), "_answer");
                return 0;
            }

            TagLoger::log(Log_ClientServer, 0, "start_key::[%zu][%s]\n",strlen(start_subscription_name), start_subscription_name);

            // В замен sadd_printf
            devManager::instance()->getDevInfo()->getPipe(std::string(start_subscription_name))->insert(fd);

            subscriptions[i] = start_subscription_name;
            TagLoger::log(Log_ClientServer, 0, "subscriptions[%d]=[%s]\n", i, start_subscription_name);

            if(memcmp(subscriptions[i], "private_", strlen("private_")) == 0 && web_user_id <= 0)
            {
                // Канал типа private_ не позволяет подписаться без авторизации на кометсервере
                continue;
            }

            if(memcmp(subscriptions[i], "track_", strlen("track_")) == 0)
            {
                // Канал track_* уведомляет автаматически всех подписчиков о изменении их количества
                char addData[300];
                bzero(addData, 300);
                snprintf(addData, 300, "\"user_id\":\"%d\",\"uuid\":\"%s\",\"event_name\":\"subscription\"", web_user_id, web_user_uuid);
                internalApi::send_event_to_pipe(local_buf, subscriptions[i], "{\\\"data\\\":\\\"\\\"}", addData);
            }


            if(memcmp(subscriptions[i], "trust_", strlen("trust_")) == 0)
            {
                // Отправка последних 20 сообщений из канала trust_
                local_buf->stm.pipe_messages_select.execute(subscriptions[i], 20);
                while(!local_buf->stm.pipe_messages_select.fetch())
                {
                    char serverData[EVENT_NAME_LEN+256];
                    snprintf(serverData,
                            EVENT_NAME_LEN+256,
                            "\"fromQueue\":true,\"event_name\":\"%s\",\"uuid\":\"%s\"",
                            local_buf->stm.pipe_messages_select.result_event,
                            local_buf->stm.pipe_messages_select.result_id);

                    message(local_buf, local_buf->stm.pipe_messages_select.result_message, NULL, MESSAGE_TEXT, serverData);
                }
                local_buf->stm.pipe_messages_select.free();
            }

            p++;
            i++;
            start_subscription_name = p;
        }

        if(p - start_subscription_name >= PIPE_NAME_LEN)
        {
            TagLoger::error(Log_ClientServer, 0, "\x1b[1;31mСлишком длинное название канала [%s]\x1b[0m\n", start_subscription_name);
            message(local_buf, base64_encode( (const unsigned char*) "{\"error\":\"Слишком длинное название канала\"}",
                       strlen("{\"error\":\"Слишком длинное название канала\"}") ).data(), "_answer");
            return -1;
        }
    }

    return 0;
}

/**
 * Отправляет короткие до 127 символов сообщения по вебсокету
 * @param local_buf
 * @param text Текст сообщения
 */
int Client_connection::short_ws_message(thread_data* local_buf, const char* text)
{
    return short_ws_message(local_buf, text, MESSAGE_TEXT);
}

/**
 * Отправляет короткие до 127 символов сообщения по вебсокету
 * @param local_buf
 * @param text Текст сообщения
 * @param msg_type Тип сообщения
 */
int Client_connection::short_ws_message(thread_data* local_buf, const char* text, int msg_type)
{
    int len = strlen(text);
    if(len > 127)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mshort_ws_message[Len=%d > 127, type=%x|%s]\x1b[0m", len, msg_type, text);
        return -1;
    }

    char messge[125];

    messge[0] = msg_type;
    messge[1] = len & 127;
    int data_start = 2;

    snprintf(messge + data_start, 125 - data_start, "%s", text);

    return web_write(messge, len + data_start);
}

int Client_connection::web_write_error(const char* text, thread_data* local_buf)
{
    return web_write_error( text, 403, local_buf);
}

int Client_connection::web_write_error(const char* text, int code, thread_data* local_buf)
{
    web_socket_receive(local_buf);
    char messge[250];
    snprintf(messge, 250,"{\"error\":\"%d\",\"data\":\"%s\",\"event_name\":\"CometServerError\"}", code, text);
    short_ws_message(local_buf, messge);

    return short_ws_message(local_buf, "  Closed due to error. See last message.");
}


/**
 * Парсит url и возвращает указатель на web_session или 0 в случаии ошибки
 * А ещё проверяет заголовок Origin и отсекает запросы если они отправлены со значением Origin не соответсвующим списку разрешонных доменов.
 * Белый список доменов содержится в redis с ключём dev_host_{devId}
 * Если белый список не задан то проверка не осуществлятся
 *
 * Извлекает данный web_user_id, client_major_version, client_minor_version
 * @param client
 * @param len
 * @return указатель на web_session или 0 в случаии ошибки
 */
char* Client_connection::parse_url(int client, int len, thread_data* local_buf)
{
    char * mytext = local_buf->buf;
    TagLoger::log(Log_ClientServer, 0, " >Client %d создание cpstring msg\n",client);

    int ses_index = str_find(mytext,'=',300);
    if(ses_index == -1)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mНеправильный запрос [Не найден идентификатор ссесии] \x1b[0m\n");
        web_write_error( "Error code 401(Invalid request, session ID not found)", 401, local_buf);
        return 0;
    }

    char * web_session = &mytext[ses_index+1];

    int uid_index = str_find(mytext,'=',2,0,300);
    if(uid_index == -1)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mНеправильный WS запрос [Не найден идентификатор пользователя] \x1b[0m\n");
        web_write_error( "Error code 401(Invalid request, the user ID can not be found)", 401, local_buf);
        return 0;
    }


    web_user_id = read_long(mytext + uid_index,'&');
    if(web_user_id < 0 )
    {
        web_user_id = 0;
    }

    int udev_id_index = str_find(mytext,'=',3,0,300);
    if(udev_id_index == -1)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mНеправильный запрос [Не найден идентификатор dev_id] \x1b[0m\n");
        web_write_error( "Error code 406(Invalid request, no public key found)" , 406, local_buf);
        return 0;
    }
 
    int version_index = str_find(mytext,'=',4,0,300);
    if(version_index == -1)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mНеправильный запрос [Не передана версия api клиента] \x1b[0m\n");
        web_write_error( "Error code 418(Invalid request Not assigned client api version)\n", 418, local_buf);
        return 0;
    }

    int delta = 0;
    client_major_version = read_int(mytext+version_index,'.', &delta);

    if(client_major_version < 0)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mНеправильный запрос [Не передана версия api клиента] \x1b[0m\n");
        web_write_error( "Error code 418(Invalid request Not assigned client api version)\n", 418, local_buf);
        return 0;
    }

    client_minor_version = read_int(mytext+version_index + delta+1,'&');

    if(client_minor_version == -1)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mНеправильный запрос [Не передана версия api клиента] \x1b[0m\n");
        web_write_error( "Error code 418(Invalid request Not assigned client api version)\n", 418, local_buf);
        return 0;
    }

    int web_user_uuid_pos = str_find(mytext,'=',5,0,300);
    if(web_user_uuid_pos != -1 && memcmp(mytext + web_user_uuid_pos - 5, "uuid=", 5) == 0)
    {
        strncpy(web_user_uuid, mytext+web_user_uuid_pos, USER_UUID_LEN);
    }

    bool host_error = true;
    int nHeader = 0;
    char* pos = mytext;
    while(  nHeader< 20 )
    {
        nHeader++;
        int d = str_find(pos,'\n');
        if(d < 0)
        {
            break;
        }
        pos += d+1;

        if(memcmp( pos, "User-Agent:" , 11) == 0)
        {
            int headerLineLength = str_find(pos+11,'\r') - 1;
            strncpy(web_user_agent, pos+11+1, minValue(USER_AGENT_LEN, headerLineLength));
        }
        else if(memcmp( pos, "Origin:" , strlen("Origin:")) == 0 || memcmp( pos, "origin:" , strlen("origin:")) == 0)
        {
            int headerLineLength = str_find(pos+7,'\r') - 1;
            strncpy(web_user_host, pos+7+1, minValue(USER_HOST_LEN, headerLineLength));

            int urls = devManager::instance()->getDevInfo()->countDevUrl();
            if(urls > 0)
            {
                for(int k = 0; k< urls; k++ )
                {
                    const char* urlK = devManager::instance()->getDevInfo()->getDevUrl(k);

                    int len = strlen(urlK);
                    if( memcmp( urlK, pos+7+1, len ) == 0
                        || memcmp( urlK, pos+7+1 + strlen("https://"), len ) == 0
                        || memcmp( urlK, pos+7+1 + strlen("http://"), len ) == 0 )
                    {
                        TagLoger::log(Log_ClientServer, 0, "\x1b[32mOrigin-mysql-host[R:%s Len:%d]:%24s\x1b[0m\n", urlK, len, pos+7+1);
                        host_error = false;
                        break;
                    }
                    else
                    {
                        TagLoger::log(Log_ClientServer, 0, "\x1b[31mOrigin-mysql-host[R:%s Len:%d]:%24s\x1b[0m\n", urlK, len, pos+7+1);
                    }
                }
            }
        }
        else if(memcmp( pos, "Accept-Language:" , 16) == 0)
        {
            int headerLineLength = str_find(pos+16,'\r') - 1;
            strncpy(web_user_language, pos+16+1, minValue(USER_LANGUAGE_LEN, headerLineLength));
        }
    }

    if(host_error && devManager::instance()->getDevInfo()->countDevUrl())
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mНеправильный запрос [Запрос с запрещёного домена] \x1b[0m\n");
        web_write_error( "Error code 423(Access denied, Request from unauthorized domain.)" , 423, local_buf);
        return 0;
    }

    TagLoger::log(Log_ClientServer, 0, "web_user_id[API=%d.%d] web_user_id->%d\n",client_major_version,client_minor_version,  web_user_id);

    return web_session;
}

/**
 * Принимает входящие соединение по вебсокету
 * @param local_buf
 * @return В случаи ошибки ответ отрицательный
 */
int Client_connection::web_socket_receive(thread_data* local_buf)
{
    char * ws_key = strstr(local_buf->buf, "Sec-WebSocket-Key: ");
    if(ws_key == NULL )
    {
        TagLoger::log(Log_ClientServer, 0, "ws_key == NULL\n");
        return -1;
    }

    ws_key+= 19;
    //TagLoger::log(Log_ClientServer, 0, "ws_key:[%s]\n\n", ws_key);

    char* key_end = strstr(ws_key, "\r");
    if(key_end == NULL)
    {
        TagLoger::log(Log_ClientServer, 0, "key_end == NULL\n");
        return -1;
    }

    int key_len = key_end - ws_key;

    if(key_end == NULL || key_len > 50 )
    {
        TagLoger::log(Log_ClientServer, 0, "key_end == NULL || %d\n", key_len);
        return -1;
    }

    char answer_key[100];
    bzero(answer_key, 100);
    memcpy(answer_key, ws_key, key_len);
    memcpy(answer_key + key_len , "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36 );
    TagLoger::log(Log_ClientServer, 0, "answer_key:%s\n", answer_key);

    unsigned char sha1_data[20];
    bzero(sha1_data, 20);

    sha1::calc(answer_key,  key_len + 36 ,sha1_data);

    //TagLoger::log(Log_ClientServer, 0, "sha1_data:%s\n", sha1_data);

    local_buf->answer_buf.lock();
    if( strstr(local_buf->buf, "Sec-WebSocket-Protocol: ") == NULL )
    {
        snprintf(local_buf->answer_buf, local_buf->answer_buf.getSize(), "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\nSec-WebSocket-Version: 13\r\n\r\n", base64_encode( sha1_data, 20).c_str());
    }
    else
    {
        snprintf(local_buf->answer_buf, local_buf->answer_buf.getSize(), "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: chat\r\n\r\n", base64_encode( sha1_data, 20).c_str());
    }// Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits, x-webkit-deflate-frame

    int res = web_write( local_buf->answer_buf );
    local_buf->answer_buf.unlock();
    return res;
}

/**
 * Извлекает из очереди сообщения и отправляет их пользователю.
 * Это позволяет повысить надёжность доставки сообщений авторизованым пользователям когда доставка сообщений осуществляется пользователю по его id
 * @param client
 * @param len
 * @param local_buf
 * @return
 */
int Client_connection::msg_queue_send(int client, int len, thread_data* local_buf)
{
    local_buf->stm.users_queue_select.execute(web_user_id, 10);
    while(!local_buf->stm.users_queue_select.fetch())
    {
        char serverData[EVENT_NAME_LEN+64];
        snprintf(serverData, EVENT_NAME_LEN+64, "\"fromQueue\":true,\"event_name\":\"%s\"", local_buf->stm.users_queue_select.result_event);
        message(local_buf, local_buf->stm.users_queue_select.result_message, NULL, MESSAGE_TEXT, serverData);
    }
    local_buf->stm.users_queue_select.free();
    local_buf->stm.users_queue_delete.execute((long int)time(NULL), web_user_id);

    return 0;
}

/**
 * Отправляет всё что находится в логе канала.
 * @param local_buf
 * @param pipe_name
 * @param MarkerName
 * @return
 *
 * Некоторые каналы могут иметь лог сообщений, если такой лог есть то при подписке на канал отправляются все сообщения из лга.
 * Если сообщений больше чем MAX_LOG_LENGTH то новые сообщения затирают наиболее старые
 */
int Client_connection::send_pipe_log(thread_data* local_buf, char* pipe_name, const char* MarkerName)
{
    // Дополнительные данные в json отправляемые сервером.
    char addData[EVENT_NAME_LEN + 120] = "\"marker\":\"MarkerName\",\"history\":true";
    int addData_lenStart = strlen(addData);

    // Цикл записывает MarkerName в поле marker строки json для отправки клиенту
    for(int i =0; i< 10; i++)
    {
        if( !AZ09test(*(MarkerName + i)) )
        {
            break;
        }

        addData[i+10] = *(MarkerName + i);
    }

    int i = 0;
    if(!local_buf->stm.pipe_messages_select.execute(pipe_name, 99))
    {
        return 0;
    }

    char* tmp = new char[appConf::instance()->buf_size*8];
    while(!local_buf->stm.pipe_messages_select.fetch())
    {
        bzero(tmp, appConf::instance()->buf_size*8);
        mysql_real_escape_string(local_buf->db.getLink(), tmp,
                local_buf->stm.pipe_messages_select.result_message, strlen(local_buf->stm.pipe_messages_select.result_message));

        // Добавляем данные в поле event_name и user_id
        snprintf(addData + addData_lenStart, EVENT_NAME_LEN + 120 - addData_lenStart,
                ",\"event_name\":\"%s\",\"user_id\":%ld",
                local_buf->stm.pipe_messages_select.result_event,
                local_buf->stm.pipe_messages_select.result_user_id);

        TagLoger::log(Log_ClientServer, 0, "send_pipe_log[]:%s [pipe_name=%s][addData=%s]\n", tmp, pipe_name, addData);
        message(local_buf, tmp, pipe_name, MESSAGE_TEXT, addData);
        bzero(addData + addData_lenStart, EVENT_NAME_LEN + 120 - addData_lenStart);
        i++;
    }
    local_buf->stm.pipe_messages_select.free();
    delete[] tmp;

    return i;
}

/**
 * Возвращает количество подписчиков в канале (доступно только для каналов web_)
 * @param local_buf
 * @param pipe_name
 * @param MarkerName
 * @return
 */
int Client_connection::send_pipe_count(thread_data* local_buf, char* pipe_name, const char* MarkerName)
{
    if(memcmp(pipe_name, "web_", 4) != 0)
    {
        // @todo добавить ссылку на описание ошибки
        message(local_buf, base64_encode((const unsigned char*) "{\"data\":{\"number_messages\":-1,\"error\":\"[pipe_count] Недопустимое название канала. Канал должен начинатся с web_\"},\"event_name\":\"answer\"}",
                       strlen("{\"data\":{\"number_messages\":-1,\"error\":\"[pipe_count] Недопустимое название канала. Канал должен начинатся с web_\"},\"event_name\":\"answer\"}") ).data(), "_answer");
        return 0;
    }

    int num_user = 0;
    CP<Pipe> pipe = devManager::instance()->getDevInfo()->findPipe(std::string(pipe_name));
    if(!pipe.isNULL())
    {
        num_user = pipe->size();
    }

    char addData[EVENT_NAME_LEN + 60] = "\"marker\":\"MarkerName\",\"event_name\":\"user_in_pipe\"";
    for(int i =0; i< 10; i++)
    {
        if( !AZ09test(*(MarkerName + i)) )
        {
            break;
        }

        addData[i+10] = *(MarkerName + i);
    }

    char answer[300];
    snprintf(answer, 300, "{\\\"user_in_pipe\\\":\\\"%d\\\"}", num_user);
    message(local_buf, answer, "_answer_pipe_count", MESSAGE_TEXT, addData);
    return 0;
}

/**
 * Обрабатывает сообщения полученые из JS API по websockets
 * Выполняется при подключении и рукопожатии websockets
 * Производит авторизацию и разбор прочего пришедшего из заголовка.
 * Если что то было в очереди сообщений msg_queue_send для пользователя то отправит.
 * @param client
 * @param len
 * @param local_buf
 * @tudo Перепроверить и поправить механизм для работы с строкой сессии разной длины
 */
int Client_connection::web_socket_request(int client, int len, thread_data* local_buf)
{
    TagLoger::log(Log_ClientServer, 0, "[Client=%d, len=%d]\n[%s]\n",client, len, (char*)local_buf->buf);

    char * web_session = parse_url(client, len, local_buf);
    if(web_session == 0)
    {
        return -1;
    }

    web_socket_receive(local_buf);

    /**
     *  Инкремент количиства людей удачных конектов
     */
    devManager::instance()->getDevInfo()->incrFrontendConnections();

    char newHash[USER_HASH_LEN+1];
    bzero(newHash, USER_HASH_LEN+1);
    strncpy(newHash, web_session, USER_HASH_LEN);

    if(devManager::instance()->getDevInfo()->index->get_link(local_buf, web_user_id, newHash))
    {
        TagLoger::log(Log_ClientServer, 0, " >Client Авторизован %ld\n",web_user_id);


        if(!devManager::instance()->getDevInfo()->index->get_hash(local_buf, web_user_id, newHash))
        {
            TagLoger::error(Log_ClientServer, 0, " >Client get_hash return false %ld\n",web_user_id);
            message(local_buf, "", "undefined", MESSAGE_TEXT, "\"authorized\":\"true\"");
        }
        else
        {
            message(local_buf, newHash, "undefined", MESSAGE_TEXT, "\"authorized\":\"true\"");
        }
        msg_queue_send(client, len, local_buf);

        devManager::instance()->getDevInfo()->index->set_link(local_buf, web_user_id,client);
    }
    else
    {
        message(local_buf, "", "undefined", MESSAGE_TEXT, "\"authorized\":false");
        TagLoger::log(Log_ClientServer, 0, " >Client Не вторизован user_id=%ld\n", web_user_id);
        web_user_id = 0;
    }


    online_incr(local_buf);
    return 0;
}


int Client_connection::web_socket_request_message(int client, int len, thread_data* local_buf)
{
    return web_socket_request_message(client, len, local_buf, 0);
}

/**
 * Обрабатывает сообщения полученые из JS API по websockets после установления соединения
 * Фактически занимается декодированием входящих сообщений и выполняет роль роутера для дальнйшей обработке данных
 *
 * Структура обрабатываемых сообщений имеет вид {команда}\n{Данные}
 * В данный момент обрабатываются следующие команды: subscription, web_pipe, pipe_log, user_status
 *
 * @param client
 * @param len
 * @param local_buf
 * @param start_position стартовая позиция в буфере входящих данных.
 *
 * @note Иногда так случается что 2 и более сообщений приходят вместе как бы
 *       слипшимися для обработки этого случая придусмотрен рекурсивный вызов
 *       функции с заданием start_position != 0
 *
 * @note В сервере не реализована работа с фреймами длинной болше 65535 за ненадобностью
 * @note В сервере не реализована работа с фрагментироваными фреймами
 *
 *
 * @note Важно чтоб код обработки сообщения не писал данные в local_buf->buf дальше чем расположено текущее сообщение, так как там может распологатся следующие сообщение.
 *
 * @todo Учитывать инкремент сообщений при рекурсивном вызове.
 * @todo Фрагментированные пакеты не поддерживаютя, а надо бы. (Я имею в виду тот случай когда пришла половина пакета а не когда флаг FIN = 0 )
 */
int Client_connection::web_socket_request_message(int client, int len, thread_data* local_buf, int start_position)
{
    TagLoger::log(Log_ClientServer, 0, " >Client %d[fd=%d, len=%d, start_position=%d, hasFragmentInBuffer=%d] web_socket_request_message\n",client, fd, len, start_position, (int)hasFragmentInBuffer);

    if(hasFragmentInBuffer)
    {
        // Ожидается продолжение фрагментированного пакета.
        // Продолжение фрагметированого пакета не может быть в центре сообщения а значет в нутри этого условия всегда start_position = 0

        TagLoger::warn(Log_ClientServer, LogColorGreen, " >Собираем фрагменты. Client %d[fd=%d, len=%d, start_position=%d, fragment_buf_size=%d]\n",client, fd, len, start_position, fragment_buf.getSize());

        //printHexMin(fragment_buf.getData(), fragment_buf.getSize(), Log_ClientServer);
        //printHexMin(local_buf->buf, len, Log_ClientServer);

        if(fragment_buf.getSize() + len >= appConf::instance()->fragment_buf_max_size )
        {
            TagLoger::error(Log_ClientServer, LogColorRed, " >fragment_buf=%d переполнен. Client %d[fd=%d, len=%d, start_position=%d, fragment_buf_size=%d]\n"
                                                            ,fragment_buf.getSize() + len,client, fd, len, start_position, fragment_buf.getSize());
            return -1;
        }

        unsigned char* tmpMsgData = new unsigned char[len];
        memcpy(tmpMsgData, local_buf->buf.getData(), len);

        local_buf->buf.setSize(fragment_buf.getSize() + len + 100);

        memcpy(local_buf->buf.getData(), fragment_buf.getData(), fragment_buf.getSize());
        memcpy(local_buf->buf.getData() + fragment_buf.getSize(), tmpMsgData, len);

        len += fragment_buf.getSize();
        delete[] tmpMsgData;
        fragment_buf.unlock();
        hasFragmentInBuffer = false;

        TagLoger::log(Log_ClientServer, 0, "Результат сборки пакета [client=%d, fd=%d, len=%d, start_position=%d, fragment_buf_size=%d]\n",client, fd, len, start_position, fragment_buf.getSize());
    }

    // printHexMin(local_buf->buf.getData() + start_position, len, Log_ClientServer);


    unsigned char* p = (unsigned char*)( (char*)local_buf->buf + start_position);
    if(p[0] == 0x88)
    {
        /**
         * 0x88 - первый байт правильного пакета означающего намеренье браузера закрыть соединение
         *
         */
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;32mПолучен фрейм закрытия = %2x %2x\x1b[0m\n", p[2], p[3]);
        return -1;
    }

    if(p[0] == 0x8a)
    {
        /**
         * 0x8a - первый байт правильного PONG пакета
         *
         */
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;32mПолучен PONG фрейм = %2x %2x\x1b[0m\n", p[2], p[3]);
        return 0;
    }

    if( p[0] != 0x82 && p[0] != 0x81 && !(p[0] == 0 && p[1] != 0) && !(p[0] == 0 && len == 1) )
    {
        /**
         * 0x82 - первый байт правильного пакета ( RSV1 = 0, RSV2 = 0, RSV3 = 0, FIN = 1, опкод = 0x1)
         * Или равно 0 в том случаии если это рекурсивный вызов произошедший из за слипания 2 пакетов. ( об этом говорит start_position != 0 )
         */
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mПолучен ошибочный фрейм = %2x \x1b[0m\n", p[0]);
        return -1;
    }


    if(p[1] == 0 )
    {
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mЗаголовок пакета пришёл не полностью[1] [start_position=%d len=%d]\x1b[0m\n", start_position, len);
        // Заголовок пакета пришёл не полностью
        // скопируем сообщение в fragment_buf для хранение его до тех пор пока не придёт ещё часть сообщения.
        hasFragmentInBuffer = true;
        fragment_buf.setSize(len);
        memcpy(fragment_buf.lock(), local_buf->buf.getData() + start_position, len);
        return 0;
    }

    unsigned char* mask = NULL;
    unsigned int msg_data_len = p[1]& 127;
    TagLoger::log(Log_ClientServer, 0, "B2-LEN:%u\n", msg_data_len  );

    /**
     * Отступ для заголовка, используется для расчёта изменения длины если следом идёт ещё одно сообщение.
     */
    int delta_len = 0;


    /**
     * Указатель на начало данных в пакете
     */
    unsigned char* str_data;
    if(msg_data_len == 126)
    {
        if(len < 4)
        {
            TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mЗаголовок пакета пришёл не полностью[2] [start_position=%d len=%d, msg_data_len-%d]\x1b[0m\n", start_position, len, msg_data_len);
            // Заголовок пакета пришёл не полностью
            // скопируем сообщение в fragment_buf для хранение его до тех пор пока не придёт ещё часть сообщения.
            hasFragmentInBuffer = true;
            fragment_buf.setSize(len);
            memcpy(fragment_buf.lock(), local_buf->buf.getData() + start_position, len);
            return 0;
        }

        // Длина пакета до 65 536 байт
        mask = p + 4;
        str_data = p + 8;
        delta_len = 8;

        char t[2];
        t[0] = p[3];
        t[1] = p[2];

        memcpy(&msg_data_len, t, 2);
    }
    else if( msg_data_len == 127)
    {
        // Нет поддержки сообщений длинее 65 536 байт
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mНет поддержки сообщений длинее 65 536 байт\x1b[0m\n");
        return -1;
    }
    else
    {
        // Длина пакета менее 127 байт
        mask = p + 2;
        str_data = p + 6;
        delta_len = 6;
    }

    if( (int)msg_data_len > len - delta_len)
    {
        // Проверка для того чтоб длина указанная не была больше длины всего сообщения
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mОшибка протокола, указанная длина больше длины всего сообщения. [msg_data_len=%u delta_len=%d len=%d]\x1b[0m\n", msg_data_len, delta_len, len);
        //return -1; /** @FixME отключена поддержка фрагментированных пакетов */

        // Если так то возможно пришол фрагментированный пакет
        // Скопируем сообщение в fragment_buf для хранение его до тех пор пока не придёт ещё часть сообщения.
        hasFragmentInBuffer = true;
        fragment_buf.setSize(len);
        memcpy(fragment_buf.lock(), local_buf->buf.getData() + start_position, len);
        return 0;
    }

    //if(TagLoger::isLog(Log_ClientServer, TAGLOG_LOG))
    //{
        TagLoger::log(Log_ClientServer, 0, "data_max_len:%u\n", msg_data_len );
        TagLoger::log(Log_ClientServer, 0, "DATA:" );
        for(unsigned int i=0; i< msg_data_len; i++)
        {
            str_data[i] =  str_data[i] ^ mask[i % 4];
            if(str_data[i] == 0)
            {
                TagLoger::log(Log_ClientServer, 0, "![ERROR:%c]", str_data[i]);
            }
            else
            {
                TagLoger::log(Log_ClientServer, 0, "!%c", str_data[i]);
            }
        }
    //}

    /**
     * Если true то следующий фреим либо не валидный либо содержит код закрытия соединения ( 0x88 )
     */
    bool nextCommandIsEof = false;
    if(str_data[msg_data_len] != 0x82 && p[0] != 0x81 && p[0] != 0)
    {
        /**
         * 0x82 - первый байт правильного пакета ( RSV1 = 0, RSV2 = 0, RSV3 = 0, FIN = 1, опкод = 0x1)
         */
        nextCommandIsEof = true;
    }
    // Установим 0 для удобства обозначения границы сообщения
    str_data[msg_data_len] = 0;

    int res = 0;
    if( memcmp( str_data, "subscription", 12) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-subscription:\n" );
        un_subscription(local_buf);
        res = ws_subscription(local_buf, (char*) (str_data +13) , client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "user_status", 11) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-user_status:\n" );
        res = get_user_last_online_time(local_buf, (char*)(str_data +12), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "pipe_count", 10) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-pipe_count:\n" );
        res = get_pipe_count(local_buf, (char*)(str_data +11), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "statistics", 10) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-statistics:\n" );
        res = log_statistics(local_buf, (char*)(str_data +11), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "web_pipe2", 9) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-web_pipe:web_pipe_msg_v2\n" );
        res = web_pipe_msg_v2(local_buf, (char*)(str_data +10), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "web_pipe", 8) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-web_pipe:web_pipe_msg_v1\n" );
        res = web_pipe_msg_v1(local_buf, (char*)(str_data +9), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "pipe_log", 8) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-pipe_log:\n" );
        res = get_pipe_log(local_buf, (char*)(str_data +9), client, msg_data_len);
        if(res == -1) return -1;
    }
    else
    {
        TagLoger::error(Log_ClientServer, 0, "comand-undefined: len=%d [%s]\n", msg_data_len, str_data );
        return -1;
    }

    if(nextCommandIsEof)
    {
        /**
         * Следующий фреим либо не валидный либо содержит код закрытия соединения ( 0x88 )
         */
        return -1;
    }

    if((unsigned)len > start_position + msg_data_len + delta_len )
    {
        // unlock буферова так как он может использоватся несколько раз в рекурсии
        local_buf->answer_buf.unlock();

        // Рекурсивно вызываем обработку слипшихся пакетов
        TagLoger::log(Log_ClientServer, 0, "\x1b[1;31msub web_socket_request_message\x1b[0m\n" );
        return web_socket_request_message( client,  len - delta_len - msg_data_len, local_buf, start_position + msg_data_len + delta_len);

    }

    return 0;
}

int Client_connection::log_statistics(thread_data* local_buf, const char* event_data,int client, int len)
{
    // @todo Переделать на отправку данных в бд
    //local_buf->answer_buf.lock();
    //mysql_real_escape_string(local_buf->db.getLink(), local_buf->answer_buf.getData(), event_data, strlen(event_data));
    //local_buf->clusterRC.lpush_printf("log_statistics \"%s\"", local_buf->answer_buf.getData());
    //local_buf->clusterRC.ltrim("log_statistics", 0, 1000);

    return 0;
}

/**
 * Отвечает на запрос вида
 * pipe_log\nadminChat\nMarkerName
 */
int Client_connection::get_pipe_log(thread_data* local_buf, char* event_data,int client, int len)
{
    char *pipe_name = event_data;
    char* pMarker = checking_channel_name( local_buf, pipe_name);
    if(pMarker == NULL)
    {
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31msget_pipe_log недопустимое имя канала\x1b[0m\n" );
        message(local_buf, base64_encode((const unsigned char*) "{\"data\":{\"number_messages\":-1,\"error\":\"Недопустимое название канала.\"},\"event_name\":\"answer\"}",
                       strlen("{\"data\":{\"number_messages\":-1,\"error\":\"Недопустимое название канала.\"},\"event_name\":\"answer\"}") ).data(), "_answer");
        return -1;
    }

    *pMarker = 0;
    pMarker++;

    char *end_pMarker = checking_channel_name( local_buf, pMarker);
    if(end_pMarker == NULL)
    {
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31msget_pipe_log недопустимое значение маркера\x1b[0m\n" );
        message(local_buf, base64_encode((const unsigned char*) "{\"data\":{\"number_messages\":-1,\"error\":\"Недопустимое название канала.\"},\"event_name\":\"answer\"}",
                       strlen("{\"data\":{\"number_messages\":-1,\"error\":\"Недопустимое название канала.\"},\"event_name\":\"answer\"}") ).data(), "_answer");
        return -1;
    }

    if( pMarker == 0 || end_pMarker == 0)
    {
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mget_pipe_log argument error\x1b[0m\n");
        message(local_buf, base64_encode((const unsigned char*) "{\"data\":{\"number_messages\":-1,\"error\":\"get_pipe_log argument error\"},\"event_name\":\"answer\"}",
                       strlen("{\"data\":{\"number_messages\":-1,\"error\":\"get_pipe_log argument error\"},\"event_name\":\"answer\"}") ).data(), "_answer");
        return -1;
    }

    //printf("pipe_name:%s\n", pipe_name);
    //printf("pMarker:%s\n", pMarker);
    *end_pMarker = 0;
    send_pipe_log(local_buf, pipe_name, pMarker);

    return 0;
}

/**
 * Возвращает время последнего прибывания online для пользователя
 * @param local_buf
 * @param event_data
 * @param client
 * @param len
 * @return
 * @todo Проверить работоспособность, добавить маркеры как это есть в get_pipe_log
 */
int Client_connection::get_user_last_online_time(thread_data* local_buf, char* event_data,int client, int len)
{
    char *pipe_name = event_data;

    int delta = 0;
    long get_user_id = read_long(pipe_name, &delta);
    long user_time = devManager::instance()->getDevInfo()->index->get_last_online_time(local_buf, get_user_id);

    char answer_data[300];
    snprintf(answer_data, 300,"{\"data\":\"%ld\",\"event_name\":\"user_status_%ld\"}", time(0) - user_time, get_user_id ); // @todo Проверить работоспособность

    message(local_buf, base64_encode((const unsigned char*) answer_data,
                       strlen(answer_data) ).data(), "_answer_user_status");
    return 0;
}

/**
 * Возвращает количество подписчиков в канале (доступно только для каналов web_)
 * @param local_buf
 * @param event_data
 * @param client
 * @param len
 * @return
 */
int Client_connection::get_pipe_count(thread_data* local_buf, char* event_data,int client, int len)
{
    char *pipe_name = event_data;
    char* pMarker = checking_channel_name( local_buf, pipe_name);
    if(pMarker == NULL)
    {
        message(local_buf, base64_encode((const unsigned char*) "{\"data\":{\"number_messages\":-1,\"error\":\"Недопустимое название канала.\"},\"event_name\":\"answer\"}",
                       strlen("{\"data\":{\"number_messages\":-1,\"error\":\"Недопустимое название канала.\"},\"event_name\":\"answer\"}") ).data(), "_answer");
        return -1;
    }

    *pMarker = 0;
    pMarker++;

    char *end_pMarker = checking_channel_name( local_buf, pMarker);
    if(end_pMarker == NULL)
    {
        message(local_buf, base64_encode((const unsigned char*) "{\"data\":{\"number_messages\":-1,\"error\":\"Недопустимое название канала.\"},\"event_name\":\"answer\"}",
                       strlen("{\"data\":{\"number_messages\":-1,\"error\":\"Недопустимое название канала.\"},\"event_name\":\"answer\"}") ).data(), "_answer");
        return -1;
    }

    if( pMarker > 0 && end_pMarker > 0)
    {
        *end_pMarker = 0;
        send_pipe_count(local_buf, pipe_name, pMarker);
    }
    else
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[1;31mget_pipe_count argument error\x1b[0m\n");
        message(local_buf, base64_encode((const unsigned char*) "{\"data\":{\"number_messages\":-1,\"error\":\"get_pipe_count argument error\"},\"event_name\":\"answer\"}",
                       strlen("{\"data\":{\"number_messages\":-1,\"error\":\"get_pipe_count argument error\"},\"event_name\":\"answer\"}") ).data(), "_answer");
        return -1;
    }
    return 0;
}

/**
 * Проверка на валидность имени канала
 * @Note Не забывать проверять код возврата, может вернуть NULL
 * @param pipe_start
 * @return Указатель на первый символ после имени канала, тоесть указатель будет вести на символ разделителя
 * Если имя канала содержит не верные символы то вернёт 0
 */
char* Client_connection::checking_channel_name(thread_data* local_buf, const char* pipe_start)
{
    char *p = (char*)pipe_start;
    int delta = 0;
    while( *p != '\n' && *p != 0) // Проверка на валидность канала
    {
        if( !AZ09test(*p) )
        {
            if(*p != '\n' )
            {
                TagLoger::warn(Log_ClientServer, 0, "\x1b[31mchecking_channel_name Недопустимое название канала[name=%s]\x1b[0m\n", pipe_start);
                message(local_buf, base64_encode((const unsigned char*)"{\"data\":{\"error\":\"Имя канала может содержать только символы A-Za-z0-9_-\"},\"event_name\":\"answer\"}",
                               strlen("{\"data\":{\"error\":\"Имя канала может содержать только символы A-Za-z0-9_-\"},\"event_name\":\"answer\"}") ).data(), "_answer");
                return 0;
            }
        }

        if(delta > PIPE_NAME_LEN)
        {
            TagLoger::warn(Log_ClientServer, 0, "\x1b[31mchecking_channel_name Недопустимое длина названия канала[name=%s]\x1b[0m\n", pipe_start);
            message(local_buf, base64_encode((const unsigned char*)"{\"data\":{\"error\":\"Недопустимая длина названия канала\"},\"event_name\":\"answer\"}",
                           strlen("{\"data\":{\"error\":\"Недопустимая длина названия канала\"},\"event_name\":\"answer\"}") ).data(), "_answer");
            return 0;
        }

        p++;
        delta++;

    }

    return p;
}

char* Client_connection::checking_event_name(thread_data* local_buf, const char* name_start)
{
    char *p = (char*)name_start;
    int delta = 0;
    while( *p != '\n' && *p != 0) // Проверка на валидность канала
    {
        if( !AZ09test(*p) )
        {
            if(*p != '\n' )
            {
                TagLoger::warn(Log_ClientServer, 0, "\x1b[31mchecking_channel_name Недопустимое имя события[name=%s]\x1b[0m\n", name_start);
                message(local_buf, base64_encode((const unsigned char*)"{\"data\":{\"error\":\"Имя события может содержать только символы A-Za-z0-9_-\"},\"event_name\":\"answer\"}",
                               strlen("{\"data\":{\"error\":\"Имя события может содержать только символы A-Za-z0-9_-\"},\"event_name\":\"answer\"}") ).data(), "_answer");
                return 0;
            }
        }

        if(delta > EVENT_NAME_LEN)
        {
            TagLoger::warn(Log_ClientServer, 0, "\x1b[31mchecking_channel_name Недопустимое длина события[name=%s]\x1b[0m\n", name_start);
            message(local_buf, base64_encode((const unsigned char*)"{\"data\":{\"error\":\"Недопустимая длина названия события\"},\"event_name\":\"answer\"}",
                           strlen("{\"data\":{\"error\":\"Недопустимая длина названия события\"},\"event_name\":\"answer\"}") ).data(), "_answer");
            return 0;
        }

        p++;
        delta++;
    }

    return p;
}

/**
 * Обрабатывает событие пришедшие от js для отправки в web_pipe
 * @param event_data
 * @param client
 * @param len
 * @return
 * @note Старая версия, будет удалена когда выйдут из оборота все версии js api моложе 2.53
 * @depricate
 */
int Client_connection::web_pipe_msg_v1(thread_data* local_buf, char* event_data,int client, int len)
{
    bool send_user_id = true;
    char* name = event_data;
    if(memcmp(name, "@web_", 5) == 0)
    {
        // Не добавлять к сообщению id отправителя
        send_user_id = false;
        name++;
    }
    else if(memcmp(name, "web_", 4) != 0)
    {
        // @todo добавить ссылку на описание ошибки
        message(local_buf, base64_encode((const unsigned char*) "{\"data\":{\"number_messages\":-1,\"error\":\"[pipe_msg] Недопустимое название канала. Канал должен начинатся с web_\"},\"event_name\":\"answer\"}",
                       strlen("{\"data\":{\"number_messages\":-1,\"error\":\"[pipe_msg] Недопустимое название канала. Канал должен начинатся с web_\"},\"event_name\":\"answer\"}") ).data(), "_answer");
        return -1;
    }

    char* p = checking_channel_name( local_buf, name);
    if( p == 0)
    {
        message(local_buf, base64_encode((const unsigned char*) "{\"data\":{\"number_messages\":-1,\"error\":\"[pipe_msg] Недопустимое название канала.\"},\"event_name\":\"answer\"}",
                       strlen("{\"data\":{\"number_messages\":-1,\"error\":\"[pipe_msg] Недопустимое название канала.\"},\"event_name\":\"answer\"}") ).data(), "_answer");
        return -1;
    }

    *p = 0;
    p++;

    TagLoger::log(Log_ClientServer, 0, "e:%s\n", name);
    TagLoger::log(Log_ClientServer, 0, "p:%s\n", p);
    char *msg = NULL;
    if( !send_user_id)
    {
        msg = p;
    }
    else
    {
        msg = new char[strlen(p)+40]; // !!!
        snprintf(msg, strlen(p)+40, "A::%d;%s", web_user_id, p);
    }

    TagLoger::log(Log_ClientServer, 0, "msg:%s\n", msg);
    PipeLog::addToLog(local_buf, name, "undefined", "*" , web_user_id , p, strlen(p));

    CP<Pipe> pipe = devManager::instance()->getDevInfo()->findPipe(std::string(name));

    int num_msg = 0;
    if(!pipe.isNULL())
    {
        int num_fail = 0;
        auto it = pipe->subscribers.begin();
        while(it)
        {
            int conection_id = it->data;
            TagLoger::log(Log_ClientServer, 0, "Answer[]->%d\n", conection_id );

            CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(conection_id);
            if(r)
            {
                if( r->getfd() != fd ) // Не отправлять самому себе события.
                {
                    int send_result = r->message(local_buf,  msg, name);
                    TagLoger::log(Log_ClientServer, 0, "R->message = %d\n" , send_result);
                    if(send_result == 0)
                    {
                        num_msg++;
                    }
                    else
                    {
                        TagLoger::log(Log_ClientServer, 0, "send_result[%d]->%d\n", conection_id, send_result );
                        pipe->erase(conection_id);
                        num_fail++;
                    }
                }
            }
            else
            {
                pipe->erase(conection_id);
                num_fail++;
            }
            it = it->Next();
        }
    }

    if( send_user_id)
    {
        delete[] msg;
    }

    char rdname[PIPE_NAME_LEN+64];
    bzero(rdname, PIPE_NAME_LEN+64);
    snprintf(rdname, PIPE_NAME_LEN+64, "_answer_to_%s", name);
    TagLoger::log(Log_ClientServer, 0, "answer:%s\n", rdname);

    local_buf->answer_buf.lock();
    snprintf(local_buf->answer_buf, local_buf->answer_buf.getSize(), "{\"data\":{\"number_messages\":%d,\"error\":\"\"},\"event_name\":\"answer\"}",  num_msg);

    int answer_len = strlen(local_buf->answer_buf);
    TagLoger::log(Log_ClientServer, 0, "rdname:%s\n", rdname);
    TagLoger::log(Log_ClientServer, 0, "answer:%s\n", (char*)local_buf->answer_buf);

    if(message(local_buf, base64_encode( (const unsigned char*)local_buf->answer_buf.getAndUnlock(), answer_len ).data() , rdname) < 0)
    {
        TagLoger::log(Log_ClientServer, 0, " >Client_connection Не удалось отправить ответ\n");
        return -1;
    }
    return 0;
}

/**
 * Обрабатывает событие пришедшие от js для отправки в web_pipe
 * @param event_data
 * @param client
 * @param len
 * @return
 */
int Client_connection::web_pipe_msg_v2(thread_data* local_buf, char* event_data,int client, int len)
{
    int set_user_id = web_user_id;
    char* name = event_data;
    if(memcmp(name, "@web_", 5) == 0)
    {
        // Не добавлять к сообщению id отправителя
        set_user_id = 0;
        name++;
    }
    else if(memcmp(name, "web_", 4) != 0)
    {
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mweb_pipe_msg_v2 Недопустимое название канала[name=%s]\x1b[0m\n", name);
        // @todo добавить ссылку на описание ошибки
        message(local_buf, base64_encode((const unsigned char*) "{\"data\":{\"number_messages\":-1,\"error\":\"[pipe_msg2] Недопустимое название канала. Канал должен начинатся с web_\"},\"event_name\":\"answer\"}",
                       strlen("{\"data\":{\"number_messages\":-1,\"error\":\"[pipe_msg2] Недопустимое название канала. Канал должен начинатся с web_\"},\"event_name\":\"answer\"}") ).data(), "_answer");
        return -1;
    }

    char* p = checking_channel_name( local_buf, name);
    if( p == 0)
    {
        return -1;
    }
    *p = 0;
    p++;

    char* event_name = p;

    p = checking_event_name( local_buf, event_name);
    if( p == 0)
    {
        return -1;
    }

    *p = 0;
    p++;
    // auth_type длина всегда 1 символ
    // По умолчанию как * тоесть сообщение для всех
    // Если authType == 1 то не отправляем сообщение не авторизованным пользователям.
    // Если authType == 0 то не отправляем сообщение авторизованным пользователям.
    // @todo Добавить валидацию auth_type здесь и в других местах
    // @todo Добавить использование значения auth_type здесь а не просто его отправка в лог.
    char* auth_type = p;
    p++;
    if(auth_type[1] != '\n')
    {
        // @todo добавить ссылку на описание ошибки
        message(local_buf, base64_encode((const unsigned char*) "{\"data\":{\"number_messages\":-1,\"error\":\"Недопустимое значение auth_type.\"},\"event_name\":\"answer\"}",
                       strlen("{\"data\":{\"number_messages\":-1,\"error\":\"Недопустимое значение auth_type.\"},\"event_name\":\"answer\"}") ).data(), "_answer");
        return -1;
    }

    *p = 0;
    p++;
    char *msg = p;

    TagLoger::log(Log_ClientServer, 0, "pipe:%s\n", name);
    TagLoger::log(Log_ClientServer, 0, "event:%s\n", event_name);
    TagLoger::log(Log_ClientServer, 0, "auth_type:%s\n", auth_type);
    TagLoger::log(Log_ClientServer, 0, "msg:%s\n", msg);

    local_buf->answer_buf.lock();
    //json_escape_string(msg, strlen(msg), local_buf->answer_buf.getData());
    mysql_real_escape_string(local_buf->db.getLink(), local_buf->answer_buf.getData(), msg, strlen(msg));


    TagLoger::log(Log_ClientServer, 0, "json_msg:%s\n", local_buf->answer_buf.getData());
    PipeLog::addToLog(local_buf, name, event_name, auth_type , set_user_id , msg, strlen(msg));

    // Дополнительные данные в json отправляемые сервером.
    char addData[EVENT_NAME_LEN + 64];
    snprintf(addData, EVENT_NAME_LEN + 64, "\"event_name\":\"%s\",\"user_id\":%d", event_name, set_user_id);


    CP<Pipe> pipe = devManager::instance()->getDevInfo()->findPipe(std::string(name));
    int num_msg = 0;
    if(!pipe.isNULL())
    {
        int num_fail = 0;

        auto it = pipe->subscribers.begin();
        while(it)
        {
            int conection_id = it->data;
            TagLoger::log(Log_ClientServer, 0, "Answer[]->%d\n", conection_id );

            CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(conection_id);
            if(r)
            {
                if( r->getfd() != fd ) // Не отправлять самому себе события.
                {
                    int send_result = r->message(local_buf,  local_buf->answer_buf.getData(), name, MESSAGE_TEXT, addData);
                    TagLoger::log(Log_ClientServer, 0, "R->message = %d\n" , send_result);
                    if(send_result == 0)
                    {
                        num_msg++;
                    }
                    else
                    {
                        TagLoger::log(Log_ClientServer, 0, "send_result[%d]->%d\n", conection_id, send_result );
                        pipe->erase(conection_id);
                        num_fail++;
                    }
                }
            }
            else
            {
                pipe->erase(conection_id);
                num_fail++;
            }
            it = it->Next();
        }
    }


    char* msgStr = new char[local_buf->answer_buf.getSize()+1];
    memcpy(msgStr, local_buf->answer_buf.getData(), local_buf->answer_buf.getSize());

    local_buf->answer_buf.unlock();

    char rdname[PIPE_NAME_LEN+64];
    bzero(rdname, PIPE_NAME_LEN+64);
    snprintf(rdname, PIPE_NAME_LEN+64, "_answer_to_%s", name);
    TagLoger::log(Log_ClientServer, 0, "answer:%s\n", rdname);

    local_buf->answer_buf.lock();
    snprintf(local_buf->answer_buf, local_buf->answer_buf.getSize(), "{\"data\":{\"number_messages\":%d,\"message\":\"%s\",\"error\":\"\"},\"event_name\":\"answer\"}",  num_msg, msgStr);

    delete[] msgStr;

    int answer_len = strlen(local_buf->answer_buf);
    TagLoger::log(Log_ClientServer, 0, "rdname:%s\n", rdname);
    TagLoger::log(Log_ClientServer, 0, "answer:%s\n", (char*)local_buf->answer_buf);

    if(message(local_buf, base64_encode( (const unsigned char*)local_buf->answer_buf.getAndUnlock(), answer_len ).data() , rdname) < 0)
    {
        TagLoger::log(Log_ClientServer, 0, " >Client_connection Не удалось отправить ответ\n");
        return -1;
    }
    return 0;
}


/**
 * Отвечает  на запрос options
 * @param client
 * @param len
 * @param local_buf
 */
int Client_connection::options(int client, int len, thread_data* local_buf)
{
    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:Star.Comet Server\r\nComet-Server:Star.Comet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\n\r\n+OK\r\n";
    if(web_write( resp ) < 0)
    {
      TagLoger::log(Log_ClientServer, 0, " >Client Не удалось отправить данные %d\n",fd);
    }

    return -1;
}

/**
 * Отвечает  на запрос get
 * @param client
 * @param len
 * @param local_buf
 */
int Client_connection::get_request(int client, int len, thread_data* local_buf)
{
    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:Star.Comet Server\r\nComet-Server:Star.Comet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\n\r\n";
    if(web_write( resp ) < 0 || web_write( logoPage ) < 0)
    {
      TagLoger::log(Log_ClientServer, 0, " >Client Не удалось отправить данные %d\n",fd);
    }
    return -1;
}

int Client_connection::http404_answer(int client, int len, thread_data* local_buf)
{
    char resp[]="HTTP/1.1 404 Not Found\r\nContent-Type:text/html; charset=UTF-8\r\nServer:Star.Comet Server\r\nComet-Server:Star.Comet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\n\r\n";
    if(web_write( resp ) < 0 || web_write( logoPage ) < 0)
    {
        TagLoger::log(Log_ClientServer, 0, " >Client Не удалось отправить данные %d\n",fd);
    }
    return -1;
}

/**
 * Отвечает  на запрос get с урлом /info
 * Ответ всегда выглядит {"status":"ok"}
 * Полезно если надо проверить доступность комет сервера и скорость его реакции
 * @param client
 * @param len
 * @param local_buf
 */
int Client_connection::get_info_request(int client, int len, thread_data* local_buf)
{
#ifdef monoUser
    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:Star.Comet Server\r\nComet-Server:Star.Comet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\n\r\n{\"status\":\"ok\",\"node\":\"__________\"}";
#else
    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:Star.Comet Server\r\nComet-Server:Star.Comet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\n\r\n{\"status\":\"ok\",\"node\":\"__________\"}";
#endif
    int respLen = strlen(resp);
    int nameLen = strlen(appConf::instance()->node_name);
    if(nameLen > 10)
    {
        nameLen = 10;
    }

    for(int i =0; i< nameLen; i++)
    {
        resp[respLen-2-10+i] = appConf::instance()->node_name[i];
    }

    if(web_write( resp ) < 0)
    {
      TagLoger::log(Log_ClientServer, 0, " >Client Не удалось отправить данные %d\n",fd);
    }

    return -1;
}

int Client_connection::get_favicon_request(int client, int len, thread_data* local_buf)
{
#ifdef monoUser
    char resp[]="HTTP/1.1 301 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:Star.Comet Server\r\nComet-Server:Star.Comet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\nLocation: http://comet-server.ru/favicon.ico\r\n\r\n";
#else
    char resp[]="HTTP/1.1 301 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:Star.Comet Server\r\nComet-Server:Star.Comet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\nLocation: http://comet-server.ru/favicon.ico\r\n\r\n";
#endif
    if(web_write( resp ) < 0)
    {
      TagLoger::log(Log_ClientServer, 0, " >Client Не удалось отправить данные %d\n",fd);
    }

    return -1;
}

int Client_connection::get_custom_request(int client, int len, thread_data* local_buf)
{
    char *p = local_buf->buf.getData();
    int urlStart = strlen("GET ");
    p = p + urlStart;

    int urlEnd = str_find(p, ' ', 0, 0);
    if(urlEnd <= 0)
    {
        // 404
        return http404_answer(client, len, local_buf);
    }

    p[urlEnd - 1] = 0;
    char *uri = p; 
    std::string name(appConf::instance()->base_dir);
    name.append(uri); 
    
    TagLoger::log(Log_ClientServer, 0, " >Client GET [%s]\n", name.data());
    if(name.find("..", 0) != std::string::npos)
    {
        // Проверка на две точки в урле.
        TagLoger::log(Log_ClientServer, 0, " >Client GET error403 [.. in url] [%s]\n", name.data());
        return http404_answer(client, len, local_buf);
    }
      
    int fp = open(name.data(), O_RDONLY);
    if(fp < 0)
    {
        // 404
        TagLoger::log(Log_ClientServer, 0, " >Client GET error404 [%s]\n", name.data(), errno);
        perror("error404:");
        return http404_answer(client, len, local_buf);
    }
    
    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:Star.Comet Server\r\nComet-Server:Star.Comet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods: GET\r\nAllow: GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\n\r\n";
    web_write(resp);
    
    int size = 0;
    while(size = read(fp, local_buf->answer_buf.getData(),  local_buf->answer_buf.getSize()))
    {
        if(size <= 0)
        {
            break;
        }
        
        web_write( local_buf->answer_buf.getData(), size);
        TagLoger::debug(Log_ClientServer, 0, " >send size=%d [%s]\n", size, local_buf->answer_buf.getData());
    }

    return -1;
}

/**
 * Обрабатывает сообщения от клиентов
 * @param client идентификатор клиента
 * @param text Сообщение
 * @param len Длина сообщения
 * @return 0 в случае успеха
 */
int Client_connection::request(int client, int len, thread_data* local_buf)
{
    // если len = 0 то соединение закрыто.
    if(len == 0)
    {
        return -1;
    }

    //pthread_mutex_lock(&request_mutex);
    if( log_level>1  )
    {
        if(  connection_type != REQUEST_WS)
        {
            TagLoger::log(Log_ClientServer, 0, "Обработка запроса - [Client_connection] -----------------------%d\n[%s]\n------------------------\n",len, (char*)local_buf->buf);
//            for(int i =0; i< len; i++)
//            {
//                TagLoger::log(Log_ClientServer, 0, "[%c|%d]",local_buf->buf[i],local_buf->buf[i]);
//            }
        }
        else
        {
            TagLoger::log(Log_ClientServer, 0, "Обработка WS запроса - [Client_connection] -----------------------%d\n",len);
        }
    }
    int r = -1;
    if(connection_type == REQUEST_NULL)
    {
        if(fragment_buf.isLock())
        {
            unsigned char* tmpMsgData = new unsigned char[len];
            memcpy(tmpMsgData, local_buf->buf.getData(), len);

            local_buf->buf.setSize(fragment_buf.getSize() + len + 100);

            memcpy(local_buf->buf.getData(), fragment_buf.getData(), fragment_buf.getSize());
            memcpy(local_buf->buf.getData() + fragment_buf.getSize(), tmpMsgData, len);

            len += fragment_buf.getSize();
            delete[] tmpMsgData;
            fragment_buf.unlock();
            TagLoger::log(Log_ClientServer, 0, "\x1b[1;32mСобран HTTP WS GET заголовок len=%d\x1b[0m\n[%s]\n",len, (char*)local_buf->buf);
        }

        if(local_buf->buf[len-2] != '\r' && local_buf->buf[len-1] != '\n' && memcmp((char*)local_buf->buf, "POST", strlen("POST"))!=0 )
        {
            // Все запросы кроме POST заканчиваются на \r\n по этому для них выполняем сборку из нескольких пакетов
            // Для POST запроса не выполняем так как функционал long polling не развивается и планируется к удалению

            fragment_buf.lock();
            fragment_buf.setSize(len);
            TagLoger::log(Log_ClientServer, 0, "\x1b[1;31mНе полностью пришёл HTTP WS GET заголовок len=%d\x1b[0m\n[%s]\n",len, (char*)local_buf->buf);
            memcpy(fragment_buf.getData(), local_buf->buf.getData(), len);
            return 0;
        }

        if(  memcmp((char*)local_buf->buf, "OPTIONS", strlen("OPTIONS")) == 0)
        {
            TagLoger::log(Log_ClientServer, 0, " >HTTP OPTIONS\n");

            connection_type = REQUEST_OPTIONS;
            r = options(client,len, local_buf);
        }
        else if( memcmp((char*)local_buf->buf, "GET //ws/", strlen("GET //ws/")) == 0
              || memcmp((char*)local_buf->buf, "GET /ws/", strlen("GET /ws/")) == 0
              || memcmp((char*)local_buf->buf, "GET //comet-server-com/ws/", strlen("GET //comet-server-com/ws/")) == 0
              || memcmp((char*)local_buf->buf, "GET /comet-server-com/ws/", strlen("GET /comet-server-com/ws/")) == 0)
        {
            // GET Вебсокет
            TagLoger::log(Log_ClientServer, 0, " >WS GET\n");

            connection_type = REQUEST_WS;
            r = web_socket_request(client,len, local_buf);
        }
        else if( memcmp((char*)local_buf->buf, "GET /info ", strlen("GET /info ")) == 0
              || memcmp((char*)local_buf->buf, "GET /comet-server-com/info ", strlen("GET /comet-server-com/info ")) == 0 )
        {
            // GET Из браузера
            TagLoger::log(Log_ClientServer, 0, " >HTTP GET\n");

            connection_type = REQUEST_GET;
            r = get_info_request(client,len, local_buf);
        }
        else if( memcmp((char*)local_buf->buf, "GET /favicon.ico ", strlen("GET /favicon.ico ")) == 0
              || memcmp((char*)local_buf->buf, "GET /comet-server-com/favicon.ico ", strlen("GET /comet-server-com/favicon.ico ")) == 0 )
        {
            // GET Из браузера
            TagLoger::log(Log_ClientServer, 0, " >HTTP GET favicon.ico\n");

            connection_type = REQUEST_GET;
            r = get_favicon_request(client,len, local_buf);
        }
        else if( memcmp((char*)local_buf->buf, "GET / ", strlen("GET / ")) == 0 )
        {
            // GET Из браузера
            TagLoger::log(Log_ClientServer, 0, " >HTTP GET\n");

            connection_type = REQUEST_GET;
            r = get_request(client,len, local_buf);
        }
        else if( memcmp((char*)local_buf->buf, "POST", strlen("POST")) == 0 )
        {
            connection_type = REQUEST_POST;
            // Long-Polling
            TagLoger::log(Log_ClientServer, 0, " >HTTP Long-Polling протокол (Отключён!)\n");
            //
            //connection_type = REQUEST_LONG_POLLING;
            //r = long_polling_request(client,len, local_buf);
            http404_answer(client,len, local_buf);
            r = -1;
        }
        else
        {
            connection_type = REQUEST_GET;
            r = get_custom_request(client,len, local_buf);
        }
    }
    else if(  connection_type == REQUEST_WS)
    {
        TagLoger::log(Log_ClientServer, 0, " >HTTP REQUEST_WS\n");
        r = web_socket_request_message(client,len, local_buf);
    }

    devManager::instance()->getDevInfo()->incrMessages();

    //pthread_mutex_unlock(&request_mutex);
    return r;
}

/**
 *  Инкремент количиства людей онлайн
 */
bool Client_connection::online_incr(thread_data* local_buf)
{
    devManager::instance()->getDevInfo()->incrFrontendOnline();
    return true;
}

/**
 * @return
 */
bool Client_connection::online_decr(thread_data* local_buf)
{
    devManager::instance()->getDevInfo()->decrFrontendOnline();
    return true;
}

int Client_connection::message(thread_data* local_buf, const char* msg, const char* name)
{
    return message(local_buf, msg, name, MESSAGE_TEXT, NULL);
}

/**
 * Для отправки сообщений от имени канала человеку
 * @param msg сообщение
 * @param name имя канала
 * @param message_type тип ( MESSAGE_TEXT | MESSAGE_CLOSE)
 * @param server_data Данные от сервера
 */
int Client_connection::message(thread_data* local_buf, const char* msg, const char* name, const char message_type, const char* server_data)
{
    int ret = 0;
    // потокобезопасна при условии потокобезопасности функции web_write
    if(name == NULL)
    {
        return message(local_buf, msg, "undefined", message_type, server_data);
    }

    if(server_data == NULL)
    {
        return message(local_buf, msg, name, message_type, "\"nodata\":true");
    }

    int server_data_len = strlen(server_data) + 1;
    TagLoger::log(Log_ClientServer, 0, "server_data_len->%d\n",server_data_len);

    int msg_len = strlen(msg) + server_data_len;

    TagLoger::log(Log_ClientServer, 0, "| Отправка данных message[web_user_id=%ld, msg_len=%d, connection_type=%d, fd=%d]\n", web_user_id, msg_len, connection_type, fd);

    char* messge = local_buf->messge_buf.lock();
    int answer_messge_len = 0;

    int data_start = 2;
    messge[0] = message_type;

    int constTextLen = 0;

    int pipe_len = strlen(name);

    constTextLen = 21;
    if(msg_len + pipe_len + constTextLen > 125)
    {
        data_start = 4;
        unsigned short* p = (unsigned short*)messge;
        messge[1] = (unsigned char)126;

        p[1] = msg_len + pipe_len + constTextLen;

        char t = messge[2];
        messge[2] = messge[3];
        messge[3] = t;
    }
    else
    {
        messge[1] = (msg_len + pipe_len + constTextLen) & 127;
    }

    snprintf(messge + data_start, local_buf->messge_buf.getSize() - data_start,"{\"data\":\"%s\",\"pipe\":\"%s\",%s}",msg, name, server_data);
    answer_messge_len = msg_len + pipe_len + constTextLen + data_start;


    TagLoger::log(Log_ClientServer, 0, "\x1b[33mmessage[len:%d|", answer_messge_len);
    // printHexMin(messge, answer_messge_len, Log_ClientServer);

    TagLoger::log(Log_ClientServer, 0, "]->write\x1b[0m");

    if(web_write(messge, answer_messge_len)  < 0)
    {
        TagLoger::log(Log_ClientServer, 0, " >Client Не удалось отправить данные %d\n",this->fd);
        ret = -1;
    }
    else
    {
        TagLoger::log(Log_ClientServer, 0, " >Client Удалось отправить данные %d\n",this->fd);
    }
    local_buf->messge_buf.unlock();

    //delete messge;
    devManager::instance()->getDevInfo()->incrMessages();

    // pthread_mutex_unlock(&message_mutex);
    return ret;
}

/**
 * Отправляет сообщение клиенту
 * @param msg
 * @return 0 - успех, -1 - ошибка
 */
int Client_connection::message(thread_data* local_buf, const char* msg)
{
    return message(local_buf, msg, NULL);
}

/**
 * Устанавливает соединению статус
 * Вызывается при создании соединения с аргументом true и при удалении соединения с аргументом false
 * @todo Было бы круто разделить на 2 функции set_online и set_offline
 *
 * @param local_buf
 * @param IsOnLine статус который надо постивить online или offline
 * @return
 */
int Client_connection::set_online(thread_data* local_buf)
{
    if(isOnLine)
    {
        TagLoger::warn(Log_ClientServer, 0, "\x1b[31mset_online: Повторный вызов fd=%d\x1b[0m\n", fd);
        return 0;
    }
    start_online_time = time(0);

    TagLoger::log(Log_ClientServer, 0, "set_online = fd=%d web_user_id=%ld\n", fd, web_user_id);

    isOnLine = true;
    return 0;
}

int Client_connection::set_offline(thread_data* local_buf)
{
    if(!isOnLine)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mset_offline: Повторный вызов fd=%d\x1b[0m\n", fd);
        return 0;
    }

    online_decr(local_buf);
    //pthread_mutex_lock(&request_mutex);
    isOnLine = false;
    devManager::instance()->getDevInfo()->index->un_link(local_buf, web_user_id, fd);

    un_subscription(local_buf);
    int close = web_close();
    connection_type = REQUEST_NULL;

    client_major_version = 0;
    client_minor_version = 0;
    start_online_time = 0;

    hasFragmentInBuffer = false;
    //pthread_mutex_unlock(&request_mutex);

    if(close < 0)
    {
        TagLoger::error(Log_ClientServer, 0, "\x1b[31mset_online:web_close() = -1, fd=%d\x1b[0m\n", fd);
    }

    return close;
}
