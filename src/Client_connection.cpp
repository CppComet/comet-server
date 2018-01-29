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

Client_connection::Client_connection():connection(),fragment_buf(appConf::instance()->get_int("main", "fragment_buf_size"))
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
    web_close();
}

/**
 * Отписка клиента от событий
 * @todo Реализовать удаление каналов на которых нет подписчиков
 * @fixMe исправить для недопущения редис инекций
 */
int Client_connection::un_subscription(thread_data* local_buf)
{
    local_buf->setThreadStatus('U');
    for(int i=0; i< MAX_SUBSCRIPTION_PIPE; i++)
    {
        if(subscriptions[i] !=0 )
        {
            TagLoger::log(Log_ClientServer, 0, "un_subscription[%d]%s\n",i, subscriptions[i]);

            // В замен srem_printf
            devInfo* dev = devManager::instance()->getDevInfo(web_user_dev_id);
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
                internalApi::send_event_to_pipe(local_buf, subscriptions[i], "{\\\"data\\\":\\\"\\\"}", web_user_dev_id, addData);
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
    local_buf->setThreadStatus('S');
    int event_data_len = strlen(event_data);
    if(event_data_len > SUBSCRIPTION_DATA_LEN)
    {
            TagLoger::error(Log_ClientServer, 0, "\x1b[1;31mToo many letters[%d]\x1b[0m\n", event_data_len);
            message(local_buf, base64_encode( (const char*) "{\"error\":\"Too many letters\"}").data(), "_answer");
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
                TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mToo many letters [%d]\x1b[0m\n", i);
                message(local_buf, base64_encode( (const char*) "{\"error\":\"Too many letters\"}").data(), "_answer");
                return 0;
            }

            TagLoger::log(Log_ClientServer, 0, "start_key::[%zu][%s]\n",strlen(start_subscription_name), start_subscription_name);

            // В замен sadd_printf
            devManager::instance()->getDevInfo(web_user_dev_id)->getPipe(std::string(start_subscription_name))->insert(fd);

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
                char data[500];
                bzero(data, 500);
                snprintf(data, 500, "{\\\"data\\\":{\\\"user_id\\\":\\\"%d\\\",\\\"uuid\\\":\\\"%s\\\"}}", web_user_id, web_user_uuid);
                internalApi::send_event_to_pipe(local_buf, subscriptions[i], data, web_user_dev_id, "\"event_name\":\"subscription\"");

                // @todo Вставить правку в js апи для совместимости (перенести данные из data в addData)
                if(local_buf->isWSClusterActive())
                {
                    auto it = local_buf->wsCluster.begin();
                    while(it != local_buf->wsCluster.end())
                    {
                        auto link = *it;
                        link->query_format(
                        "cometqlcluster_v1 set dev_id=%d; INSERT INTO pipes_messages (name, event, message)VALUES('%s', 'subscription', '{\"user_id\":\"%d\",\"uuid\":\"%s\"');",
                                web_user_dev_id,
                                subscriptions[i],
                                web_user_id,
                                web_user_uuid);
                        it++;
                    }
                }
            }


            if(memcmp(subscriptions[i], "trust_", strlen("trust_")) == 0)
            {
                // Отправка последних 20 сообщений из канала trust_
                local_buf->stm.pipe_messages_select->execute(web_user_dev_id, subscriptions[i], 20);
                while(!local_buf->stm.pipe_messages_select->fetch())
                {
                    char serverData[EVENT_NAME_LEN+256];
                    snprintf(serverData,
                            EVENT_NAME_LEN+256,
                            "\"fromQueue\":true,\"event_name\":\"%s\",\"uuid\":\"%s\"",
                            local_buf->stm.pipe_messages_select->result_event,
                            local_buf->stm.pipe_messages_select->result_id);

                    message(local_buf, local_buf->stm.pipe_messages_select->result_message, NULL, MESSAGE_TEXT, serverData);
                }
                local_buf->stm.pipe_messages_select->free();
            }

            p++;
            i++;
            start_subscription_name = p;
        }

        if(p - start_subscription_name >= PIPE_NAME_LEN)
        {
            TagLoger::error(Log_ClientServer, 0, "\x1b[1;31mChannel name is too long [%s]\x1b[0m\n", start_subscription_name);
            message(local_buf, base64_encode( (const char*) "{\"error\":\"Channel name is too long\"}").data(), "_answer");
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
    if(web_socket_receive(local_buf) < 0)
    {
        return -1;
    }
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
 * Извлекает данный web_user_id, web_user_dev_id, client_major_version, client_minor_version
 * @param client
 * @param len
 * @return указатель на начало строки web_hash или NULL в случаии ошибки
 */
char* Client_connection::parse_url(int client, int len, thread_data* local_buf)
{
    isAuthUser = false;
    char * mytext = local_buf->buf;
    int ses_index = str_find(mytext,'=',300);
    if(ses_index == -1)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mInvalid request [Session ID not found] \x1b[0m\n");
        web_write_error( "Error code 401(Invalid request, session ID not found)", 401, local_buf);
        return NULL;
    }

    char * web_session = &mytext[ses_index+1];
     
    int uid_index = str_find(mytext,'=',2,0,300);
    if(uid_index == -1)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mWrong WS request [No user ID found] \x1b[0m\n");
        web_write_error( "Error code 401(Invalid request, the user ID can not be found)", 401, local_buf);
        return NULL;
    }


    web_user_id = read_long(mytext + uid_index,'&');
    if(web_user_id < 0 )
    {
        web_user_id = 0;
    }

    int udev_id_index = str_find(mytext,'=',3,0,300);
    if(udev_id_index == -1)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mInvalid request [No identifier dev_id found] \x1b[0m\n");
        web_write_error( "Error code 406(Invalid request, no public key found [1])" , 406, local_buf);
        return NULL;
    }


    web_user_dev_id = read_long(mytext + udev_id_index,'&');
    if(web_user_dev_id < 0)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mInvalid request [No identifier dev_id found] \x1b[0m\n");
        web_write_error( "Error code 406(Invalid request, no public key found [2])" , 406, local_buf);
        return NULL;
    }

    if(web_user_dev_id > devManager::instance()->getDevIndexSize())
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mInvalid request [No identifier dev_id found] \x1b[0m\n");
        web_write_error( "Error code 406(Invalid request, no public key found [3])" , 406, local_buf);
        return NULL;
    }

    int version_index = str_find(mytext,'=',4,0,300);
    if(version_index == -1)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mInvalid request [No version of api client transferred] \x1b[0m\n");
        web_write_error( "Error code 418(Invalid request Not assigned client api version)\n", 418, local_buf);
        return NULL;
    }

    int delta = 0;
    client_major_version = read_int(mytext+version_index,'.', &delta);

    if(client_major_version < 0)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mInvalid request [No version of api client transferred] \x1b[0m\n");
        web_write_error( "Error code 418(Invalid request Not assigned client api version)\n", 418, local_buf);
        return NULL;
    }

    client_minor_version = read_int(mytext+version_index + delta+1,'&');

    if(client_minor_version == -1)
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mInvalid request [No version of api client transferred] \x1b[0m\n");
        web_write_error( "Error code 418(Invalid request Not assigned client api version)\n", 418, local_buf);
        return NULL;
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

            int urls = devManager::instance()->getDevInfo(web_user_dev_id)->countDevUrl();
            if(urls > 0)
            {
                for(int k = 0; k< urls; k++ )
                {
                    const char* urlK = devManager::instance()->getDevInfo(web_user_dev_id)->getDevUrl(k);

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

    if(host_error && devManager::instance()->getDevInfo(web_user_dev_id)->countDevUrl())
    {
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mInvalid request [Request from a denied domain] \x1b[0m\n");
        web_write_error( "Error code 423(Access denied, Request from unauthorized domain.)" , 423, local_buf);
        return NULL;
    }

    TagLoger::log(Log_ClientServer, 0, "web_user_id[API=%d.%d] web_user_id->%d web_user_dev_id->%d\n",client_major_version,client_minor_version,  web_user_id,web_user_dev_id);

    isAuthUser = true;
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
    //TagLoger::log(Log_ClientServer, 0, "answer_key:%s\n", answer_key);

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
    }

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
    local_buf->stm.users_queue_select->execute(web_user_dev_id, web_user_id, 10);
    while(!local_buf->stm.users_queue_select->fetch())
    {
        char serverData[EVENT_NAME_LEN+64];
        snprintf(serverData, EVENT_NAME_LEN+64, "\"fromQueue\":true,\"event_name\":\"%s\"", local_buf->stm.users_queue_select->result_event);
        message(local_buf, local_buf->stm.users_queue_select->result_message, NULL, MESSAGE_TEXT, serverData);
    }
    local_buf->stm.users_queue_select->free();
    local_buf->stm.users_queue_delete->execute((long int)time(NULL), web_user_dev_id, web_user_id);

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
    if(!local_buf->stm.pipe_messages_select->execute(web_user_dev_id, pipe_name, 99))
    {
        return 0;
    }

    char* tmp = new char[appConf::instance()->get_int("main", "buf_size")*8];
    while(!local_buf->stm.pipe_messages_select->fetch())
    {
        bzero(tmp, appConf::instance()->get_int("main", "buf_size")*8);
        mysql_real_escape_string(local_buf->db.getLink(), tmp,
                local_buf->stm.pipe_messages_select->result_message, strlen(local_buf->stm.pipe_messages_select->result_message));

        // Добавляем данные в поле event_name и user_id
        snprintf(addData + addData_lenStart, EVENT_NAME_LEN + 120 - addData_lenStart,
                ",\"event_name\":\"%s\",\"user_id\":%ld",
                local_buf->stm.pipe_messages_select->result_event,
                local_buf->stm.pipe_messages_select->result_user_id);

        TagLoger::log(Log_ClientServer, 0, "send_pipe_log[]:%s [pipe_name=%s][addData=%s]\n", tmp, pipe_name, addData);
        message(local_buf, tmp, pipe_name, MESSAGE_TEXT, addData);
        bzero(addData + addData_lenStart, EVENT_NAME_LEN + 120 - addData_lenStart);
        i++;
    }
    local_buf->stm.pipe_messages_select->free();
    delete[] tmp;

    return i;
}

/**
 * Возвращает количество подписчиков в канале (доступно только для каналов web_ и track_)
 * @param local_buf
 * @param pipe_name
 * @param MarkerName
 * @return
 */
int Client_connection::send_pipe_count(thread_data* local_buf, char* pipe_name, const char* MarkerName)
{
    if(memcmp(pipe_name, "web_", 4) != 0 && memcmp(pipe_name, "track_", 6) != 0)
    {
        // @todo добавить ссылку на описание ошибки
        message(local_buf, base64_encode((const char*) "{\"data\":{\"number_messages\":-1,\"error\":\"[pipe_count] Invalid channel name. The channel should start with web_\"},\"event_name\":\"answer\"}").data(), "_answer");
        return 0;
    }

    int num_user = 0;
    CP<Pipe> pipe = devManager::instance()->getDevInfo(web_user_dev_id)->findPipe(std::string(pipe_name));
    if(!pipe.isNULL())
    {
        num_user = pipe->size();
    }

    if(local_buf->isWSClusterActive())
    {
        MYSQL_ROW row;
        auto it = local_buf->wsCluster.begin();
        while(it != local_buf->wsCluster.end())
        {
            auto link = *it;

            if(!link->query_format("cometqlcluster_v1 set dev_id=%d; SELECT users FROM pipes WHERE name = '%s'", web_user_dev_id, pipe_name))
            {
                break;
            }

            auto result = mysql_store_result(link->getLink());

            while((row = mysql_fetch_row(result)))
            {
                try{
                    num_user += std::stoi(row[0]);
                }catch(...)
                {
                    printf("\x1b[1;31mexeption in parsing num_user value = `%s` in query `SELECT users FROM pipes WHERE name = '%s'` \x1b[0m\n", row[0], pipe_name);
                    return false;
                }
            }
            mysql_free_result(result);
            it++;
        }
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

    char* web_hash = parse_url(client, len, local_buf);
    if(web_hash == NULL)
    {
        return -1;
    }

    std::string web_session(web_hash, str_find(web_hash, '&', 600));

    if(web_socket_receive(local_buf) < 0)
    {
        return -1;
    }

    /**
     *  Инкремент количиства людей удачных конектов
     */
    devManager::instance()->getDevInfo(web_user_dev_id)->incrFrontendConnections();
 
    char newHash[USER_HASH_LEN+1];
    bzero(newHash, USER_HASH_LEN+1);
    
    std::string addInfo("\"server\":\"");
    addInfo.append(MYSQL_SERVERNAME).append("\",");

    if(devManager::instance()->getDevInfo(web_user_dev_id)->index->get_link(local_buf, web_user_id, web_session))
    {
        TagLoger::log(Log_ClientServer, 0, " >Client Authorized %ld\n",web_user_id);
        addInfo.append("\"authorized\":\"true\"");


        if(!devManager::instance()->getDevInfo(web_user_dev_id)->index->get_hash(local_buf, web_user_id, newHash))
        {
            TagLoger::error(Log_ClientServer, 0, " >Client get_hash return false %ld\n",web_user_id);
            message(local_buf, "", "undefined", MESSAGE_TEXT, addInfo.data());
        }
        else
        {
            message(local_buf, newHash, "undefined", MESSAGE_TEXT, addInfo.data());
        }

        msg_queue_send(client, len, local_buf);
        devManager::instance()->getDevInfo(web_user_dev_id)->index->set_link(local_buf, web_user_id,client);
    }
    else
    {
        addInfo.append("\"authorized\":\"false\"");
        message(local_buf, "", "undefined", MESSAGE_TEXT, addInfo.data());
        TagLoger::log(Log_ClientServer, 0, " >Client not Authorized dev_id=%d, user_id=%ld\n", web_user_dev_id, web_user_id);
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
 * @param client идентификатор клиента
 * @param len длина пришедшего сообщения
 * @param local_buf ссылка на local_buf потока
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

        TagLoger::warn(Log_ClientServer, LogColorGreen, " >Collecting fragments. Client %d[fd=%d, len=%d, start_position=%d, fragment_buf_size=%d]\n",client, fd, len, start_position, fragment_buf.getSize());

        //printHexMin(fragment_buf.getData(), fragment_buf.getSize(), Log_ClientServer);
        //printHexMin(local_buf->buf, len, Log_ClientServer);

        if(fragment_buf.getSize() + len >= appConf::instance()->get_int("main", "fragment_buf_max_size") )
        {
            TagLoger::error(Log_ClientServer, LogColorRed, " >fragment_buf=%d overcrowded. Client %d[fd=%d, len=%d, start_position=%d, fragment_buf_size=%d]\n"
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

        TagLoger::debug(Log_ClientServer, 0, "The result of the package assembly [client=%d, fd=%d, len=%d, start_position=%d, fragment_buf_size=%d]\n",client, fd, len, start_position, fragment_buf.getSize());
    }

    // printHexMin(local_buf->buf.getData() + start_position, len, Log_ClientServer);


    unsigned char* p = (unsigned char*)( (char*)local_buf->buf + start_position);
    if(p[0] == 0x88)
    {
        /**
         * 0x88 - первый байт правильного пакета означающего намеренье браузера закрыть соединение
         *
         */
        TagLoger::log(Log_ClientServer, 0, "\x1b[1;32mClosed frame received = %2x %2x\x1b[0m\n", p[2], p[3]);
        return -1;
    }

    if(p[0] == 0x8a)
    {
        /**
         * 0x8a - первый байт правильного PONG пакета
         *
         */
        TagLoger::log(Log_ClientServer, 0, "\x1b[1;32mReceived PONG frame = %2x %2x\x1b[0m\n", p[2], p[3]);
        return 0;
    }

    if( p[0] != 0x82 && p[0] != 0x81 && !(p[0] == 0 && p[1] != 0) && !(p[0] == 0 && len == 1) )
    {
        /**
         * 0x82 - первый байт правильного пакета ( RSV1 = 0, RSV2 = 0, RSV3 = 0, FIN = 1, опкод = 0x1)
         * Или равно 0 в том случаии если это рекурсивный вызов произошедший из за слипания 2 пакетов. ( об этом говорит start_position != 0 )
         */
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mReceived error frame = %2x \x1b[0m\n", p[0]);
        return -1;
    }


    if(p[1] == 0 )
    {
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mThe package header did not come fully[1] [start_position=%d len=%d]\x1b[0m\n", start_position, len);
        // The package header did not come fully
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
            TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mThe package header did not come fully[2] [start_position=%d len=%d, msg_data_len-%d]\x1b[0m\n", start_position, len, msg_data_len);
            // The package header did not come fully
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
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mNo message support longer 65 536 byte\x1b[0m\n");
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
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mProtocol error, the specified length is longer than the length of the entire message. [msg_data_len=%u delta_len=%d len=%d]\x1b[0m\n", msg_data_len, delta_len, len);
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
    if( memcmp( str_data, "subscription", strlen("subscription")) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-subscription:\n" );
        un_subscription(local_buf);
        res = ws_subscription(local_buf, (char*) (str_data + strlen("subscription") + 1) , client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "user_status", strlen("user_status")) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-user_status:\n" );
        res = get_user_last_online_time(local_buf, (char*)(str_data + strlen("user_status") + 1), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "pipe_count", strlen("pipe_count")) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-pipe_count:\n" );
        res = get_pipe_count(local_buf, (char*)(str_data + strlen("pipe_count") + 1), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "statistics", strlen("statistics")) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-statistics:\n" );
        res = log_statistics(local_buf, (char*)(str_data + strlen("statistics") + 1), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "web_pipe2", strlen("web_pipe2")) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-web_pipe:web_pipe_msg_v2\n" );
        res = web_pipe_msg_v2(local_buf, (char*)(str_data + strlen("web_pipe2") + 1), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "pipe_log", strlen("pipe_log")) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-pipe_log:\n" );
        res = get_pipe_log(local_buf, (char*)(str_data + strlen("pipe_log") + 1), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "track_pipe_users", strlen("track_pipe_users")) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-web_pipe:track_pipe_users\n" );
        res = track_pipe_users(local_buf, (char*)(str_data + strlen("track_pipe_users") + 1), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "user_data", strlen("user_data")) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "user_data:\n" );
        res = web_user_data(local_buf, (char*)(str_data + strlen("user_data") + 1), client, msg_data_len);
        if(res == -1) return -1;
    }
    else if(memcmp( str_data, "cgi_call", strlen("cgi_call")) == 0)
    {
        TagLoger::log(Log_ClientServer, 0, "comand-cgi_call:\n" );
        res = cgi_call(local_buf, (char*)(str_data + strlen("cgi_call") + 1), client, msg_data_len);
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
    // @todo simpleTask Переделать на отправку данных в бд
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
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31msget_pipe_log Invalid channel name\x1b[0m\n" );
        message(local_buf, base64_encode((const char*) "{\"data\":{\"number_messages\":-1,\"error\":\"Invalid channel name.\"},\"event_name\":\"answer\"}").data(), "_answer");
        return -1;
    }

    *pMarker = 0;
    pMarker++;

    char *end_pMarker = checking_channel_name( local_buf, pMarker);
    if(end_pMarker == NULL)
    {
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31msget_pipe_log Invalid marker value\x1b[0m\n" );
        message(local_buf, base64_encode((const char*) "{\"data\":{\"number_messages\":-1,\"error\":\"Invalid marker value.\"},\"event_name\":\"answer\"}").data(), "_answer");
        return -1;
    }

    if( pMarker == 0 || end_pMarker == 0)
    {
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mget_pipe_log argument error\x1b[0m\n");
        message(local_buf, base64_encode((const char*) "{\"data\":{\"number_messages\":-1,\"error\":\"get_pipe_log argument error\"},\"event_name\":\"answer\"}").data(), "_answer");
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
    long user_time = devManager::instance()->getDevInfo(web_user_dev_id)->index->get_last_online_time(local_buf, get_user_id);

    char answer_data[300];
    snprintf(answer_data, 300,"{\"data\":\"%ld\",\"event_name\":\"user_status_%ld\"}", time(0) - user_time, get_user_id ); // @todo Проверить работоспособность

    message(local_buf, base64_encode((const char*) answer_data).data(), "_answer_user_status");
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
        message(local_buf, base64_encode((const char*) "{\"data\":{\"number_messages\":-1,\"error\":\"Invalid pipe name.\"},\"event_name\":\"answer\"}").data(), "_answer");
        return -1;
    }

    *pMarker = 0;
    pMarker++;

    char *end_pMarker = checking_channel_name( local_buf, pMarker);
    if(end_pMarker == NULL)
    {
        message(local_buf, base64_encode((const char*) "{\"data\":{\"number_messages\":-1,\"error\":\"Invalid pipe name.\"},\"event_name\":\"answer\"}").data(), "_answer");
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
        message(local_buf, base64_encode((const char*) "{\"data\":{\"number_messages\":-1,\"error\":\"get_pipe_count argument error\"},\"event_name\":\"answer\"}").data(), "_answer");
        return -1;
    }
    return 0;
}

/**
 * Проверка на валидность имени канала
 * @note Не забывать проверять код возврата, может вернуть NULL
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
                TagLoger::warn(Log_ClientServer, 0, "\x1b[31mchecking_channel_name Invalid pipe name [name=%s]\x1b[0m\n", pipe_start);
                message(local_buf, base64_encode((const char*)"{\"data\":{\"error\":\"The channel name can only contain characters A-Za-z0-9_-\"},\"event_name\":\"answer\"}").data(), "_answer");
                return 0;
            }
        }

        if(delta > PIPE_NAME_LEN)
        {
            TagLoger::warn(Log_ClientServer, 0, "\x1b[31mchecking_channel_name Invalid pipe name length[name=%s]\x1b[0m\n", pipe_start);
            message(local_buf, base64_encode((const char*)"{\"data\":{\"error\":\"Invalid pipe name length\"},\"event_name\":\"answer\"}").data(), "_answer");
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
                TagLoger::warn(Log_ClientServer, 0, "\x1b[31mchecking_channel_name Invalid event name [name=%s]\x1b[0m\n", name_start);
                message(local_buf, base64_encode((const char*)"{\"data\":{\"error\":\"The event name can only contain characters A-Za-z0-9_-\"},\"event_name\":\"answer\"}").data(), "_answer");
                return 0;
            }
        }

        if(delta > EVENT_NAME_LEN)
        {
            TagLoger::warn(Log_ClientServer, 0, "\x1b[31mchecking_channel_name Invalid event name length [name=%s]\x1b[0m\n", name_start);
            message(local_buf, base64_encode((const char*)"{\"data\":{\"error\":\"Invalid event name length\"},\"event_name\":\"answer\"}").data(), "_answer");
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
 * @todo учесть работу в кластере
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
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mweb_pipe_msg_v2 Invalid channel name [name=%s]\x1b[0m\n", name);
        // @todo добавить ссылку на описание ошибки
        message(local_buf, base64_encode((const char*) "{\"data\":{\"number_messages\":-1,\"error\":\"[pipe_msg2] Invalid channel name. The channel should begin with web_\"},\"event_name\":\"answer\"}").data(), "_answer");
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
    // @note auth_type будет удалён.
    char* auth_type = p;
    p++;
    if(auth_type[1] != '\n')
    {
        // @todo добавить ссылку на описание ошибки
        message(local_buf, base64_encode((const char*) "{\"data\":{\"number_messages\":-1,\"error\":\"Invalid value auth_type.\"},\"event_name\":\"answer\"}").data(), "_answer");
        return -1;
    }

    *p = 0;
    p++;
    char *msg = p;

    TagLoger::log(Log_ClientServer, 0, "pipe:%s\n", name);
    TagLoger::log(Log_ClientServer, 0, "event:%s\n", event_name);
    TagLoger::log(Log_ClientServer, 0, "msg:%s\n", msg);

    local_buf->answer_buf.lock();
    //json_escape_string(msg, strlen(msg), local_buf->answer_buf.getData());
    mysql_real_escape_string(local_buf->db.getLink(), local_buf->answer_buf.getData(), msg, strlen(msg));


    TagLoger::log(Log_ClientServer, 0, "json_msg:%s\n", local_buf->answer_buf.getData());
    PipeLog::addToLog(local_buf, web_user_dev_id, name, event_name, set_user_id , msg, strlen(msg));

    // Дополнительные данные в json отправляемые сервером.
    char addData[EVENT_NAME_LEN + 64];
    snprintf(addData, EVENT_NAME_LEN + 64, "\"event_name\":\"%s\",\"user_id\":%d", event_name, set_user_id);


    CP<Pipe> pipe = devManager::instance()->getDevInfo(web_user_dev_id)->findPipe(std::string(name));
    int num_msg = 0;
    if(!pipe.isNULL())
    {
        int num_fail = 0;

        auto it = pipe->subscribers->begin();
        while(it)
        {
            int conection_id = it->data;
            TagLoger::log(Log_ClientServer, 0, "Answer[]->%d\n", conection_id );

            CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(conection_id);
            if(r && r->web_user_dev_id == web_user_dev_id)
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

    // @todo Отметить в документации что number_messages относится только к инфе
    // этого сервера и сколько было сообщений доставлено по нодам кластера не известно.
    // @todo добавить как баг тот фапкт что будет терятся user_id отправителя при работе в кластере.
    if(local_buf->isWSClusterActive())
    {
        auto it = local_buf->wsCluster.begin();
        while(it != local_buf->wsCluster.end())
        {
            auto link = *it;

            // @todo Проверять что если ошибка сетевая или что то ещё то повторять попытку.
            link->query_format("cometqlcluster_v1 set dev_id=%d; INSERT INTO pipes_messages (name, event, message)VALUES('%s', '%s', '%s');",
                    web_user_dev_id,
                    name,
                    event_name,
                    local_buf->answer_buf.getData());
            it++;
        }

        local_buf->answer_buf.unlock();
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
        return -1;
    }
    return 0;
}

/**
 * Для вызова внешних скриптов
 * @param local_buf ссылка на local_buf потока
 * @param event_data данные вызова
 * @param client идентификатор клиента
 * @param len длина пришедшего сообщения
 * @return
 *
 *
 * Вызов внешнего скрипта:
 * exec синхронный, но можно спроектировать так чтоб он был как будто асинхронный и тогда надо будет просто
 * передавать с ним колбек функцию которя могла бы отправлять ответ от сервера.
 *
 */
int Client_connection::cgi_call(thread_data* local_buf, char* event_data,int client, int len)
{ 
    return 0;
}

/**
 * Формат сообщения:
 * user_data\n
 * pMarker\n
 * user_id
 * 
 * @param local_buf
 * @param event_data
 * @param client
 * @param len
 * @return 
 */
int Client_connection::web_user_data(thread_data* local_buf, char* event_data,int client, int len)
{ 
    char* pMarker = event_data; 
    char* end_pMarker = checking_channel_name( local_buf, pMarker);
    if(end_pMarker == NULL)
    {
        message(local_buf, base64_encode((const char*) "{\"data\":{\"user_id\":-1,\"user_data\":\"\",\"error\":\"Invalid marker name.\"},\"event_name\":\"answer\"}").data(), "_answer");
        return -1;
    }
    
    *(end_pMarker-1) = 0;
    
    int userId = 0;
    try{
        userId = std::stoi(end_pMarker);
    }catch(...)
    {
        message(local_buf, base64_encode((const char*) "{\"data\":{\"user_id\":-1,\"user_data\":\"\",\"error\":\"Invalid marker name.\"},\"event_name\":\"answer\"}").data(), "_answer");
        return -1;
    }

    std::string res;
    std::string server_info;
    res.append("{\"data\":{\"user_id\":").append(std::to_string(userId)).append(",\"user_data\":\"");
    server_info.append("\"marker\":\"").append(pMarker).append("\"");
    
    
    local_buf->stm.users_data_select->execute(web_user_dev_id, userId);
    if(!local_buf->stm.users_data_select->fetch())
    {
        local_buf->answer_buf.lock();
        
        mysql_real_escape_string(local_buf->db.getLink(), local_buf->answer_buf.getData(), local_buf->stm.users_data_select->result_data, strlen(local_buf->stm.users_data_select->result_data));
        res.append(local_buf->answer_buf.getAndUnlock()).append("\"},\"event_name\":\"answer\"}"); 
        message(local_buf, base64_encode(res.data()).data(), "_answer", MESSAGE_TEXT, server_info.data());
    }
    else
    { 
        res.append("\"},\"event_name\":\"answer\"}"); 
        message(local_buf, base64_encode(res.data()).data(), "_answer", MESSAGE_TEXT, server_info.data());
    }
    
    local_buf->stm.users_data_select->free();

    return 0;
}

/**
 * Обрабатывает событие пришедшие от js для получения списка подписчиков на канале track_*
 * @param event_data
 * @param client
 * @param len
 * @return
 */
int Client_connection::track_pipe_users(thread_data* local_buf, char* event_data,int client, int len)
{
    char pipe_name[PIPE_NAME_LEN+1];
    bzero(pipe_name, PIPE_NAME_LEN+1);

    char marker[PIPE_NAME_LEN+1];
    bzero(marker, PIPE_NAME_LEN+1);
    sscanf(event_data, "%64s\n%64s\n", pipe_name, marker);

    if(memcmp(pipe_name, "track_", strlen("track_")) != 0)
    {
        TagLoger::warn(Log_ClientServer, 0, "\x1b[1;31mtrack_pipe_users Invalid channel name[name=%s]\x1b[0m\n", pipe_name);

        // @todo добавить ссылку на описание ошибки
        message(local_buf, base64_encode((const char*) "{\"data\":{\"number_users\":-1,\"error\":\"[track_pipe_users] Invalid channel name. The channel should start with track_\"},\"event_name\":\"answer\"}").data(), "_answer");
        return -1;
    }


    std::string usersstr("{\"event_name\":\"answer\",\"data\":{\"users\":[");

    bool hasData = false;
    char strtmp[200];
    TagLoger::log(Log_ClientServer, 0, "track_pipe_users pipe:%s\n", pipe_name);
    CP<Pipe> pipe = devManager::instance()->getDevInfo(web_user_dev_id)->findPipe(std::string(pipe_name));
    if(!pipe.isNULL())
    {
        auto it = pipe->subscribers->begin();
        while(it)
        {
            int conection_id = it->data;
            TagLoger::log(Log_ClientServer, 0, "Answer[]->%d\n", conection_id );

            CP<Client_connection> r = tcpServer <Client_connection>::instance()->get(conection_id);
            if(r && r->web_user_dev_id == web_user_dev_id)
            {
                // @todo simpleTask отдавать всем не uuid а его солёный хеш.
                bzero(strtmp, 200);
                snprintf(strtmp, 200, "{\"user_id\":%d,\"uuid\":\"%s\"}", r->web_user_id, r->web_user_uuid);
                usersstr.append(strtmp);
                hasData = true;
            }

            it = it->Next();
            if(it)
            {
                usersstr.append(",");
            }
        }
    }

    if(local_buf->isWSClusterActive())
    {
        MYSQL_ROW row;
        auto it = local_buf->wsCluster.begin();
        while(it != local_buf->wsCluster.end())
        {
            auto link = *it;

            if(!link->query_format("cometqlcluster_v1 set dev_id=%d; SELECT user_id, uuid FROM users_in_pipes WHERE name = '%s'", web_user_dev_id, pipe_name))
            {
                break;
            }

            auto result = mysql_store_result(link->getLink());

            while((row = mysql_fetch_row(result)))
            {
                if(hasData)
                {
                    usersstr.append(",");
                }

                bzero(strtmp, 200);
                snprintf(strtmp, 200, "{\"user_id\":%s,\"uuid\":\"%s\"}", row[0], row[1]);
                usersstr.append(strtmp);
                hasData = true;
            }
            mysql_free_result(result);
            it++;
        }
    }

    usersstr.append("]}}");

    std::string rdname("_answer_to_");
    rdname.append(pipe_name);
    std::string addData("\"marker\":\"");
    addData.append(marker).append("\"");

    TagLoger::log(Log_ClientServer, 0, "answer:%s\n", usersstr.data());
    if(message(local_buf, base64_encode( (const char*)usersstr.data()).data() , rdname.data(), MESSAGE_TEXT, addData.data()) < 0)
    {
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
    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:CppComet Server\r\nComet-Server:CppComet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nCache-Control: max-age=3600\r\nConnection: close\r\n\r\n+OK\r\n";
    if(web_write( resp ) < 0)
    {
      TagLoger::log(Log_ClientServer, 0, " >Client Failed to send data %d\n",fd);
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
    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:CppComet Server\r\nComet-Server:CppComet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nCache-Control: max-age=3600\r\nConnection: close\r\n\r\n";
    if(web_write( resp ) < 0 || web_write( logoPage ) < 0)
    {
      TagLoger::log(Log_ClientServer, 0, " >Client Failed to send data %d\n",fd);
    }
    return -1;
}

int Client_connection::http404_answer(int client, int len, thread_data* local_buf)
{
    char resp[]="HTTP/1.1 404 Not Found\r\nContent-Type:text/html; charset=UTF-8\r\nServer:CppComet Server\r\nComet-Server:CppComet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nCache-Control: max-age=3600\r\nConnection: close\r\n\r\n";
    if(web_write( resp ) < 0 || web_write( logoPage ) < 0)
    {
        TagLoger::log(Log_ClientServer, 0, " >Client Failed to send data %d\n",fd);
    }
    return -1;
}

int Client_connection::http403_answer(int client, int len, thread_data* local_buf)
{
    char resp[]="HTTP/1.1 403 Forbidden\r\nContent-Type:text/html; charset=UTF-8\r\nServer:CppComet Server\r\nComet-Server:CppComet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nCache-Control: max-age=3600\r\nConnection: close\r\n\r\n";
    if(web_write( resp ) < 0 || web_write( logoPage ) < 0)
    {
        TagLoger::log(Log_ClientServer, 0, " >Client Failed to send data %d\n",fd);
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
    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:CppComet Server\r\nComet-Server:CppComet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\n\r\n{\"status\":\"ok\",\"node\":\"__________\"}";

    const char* nodeName = appConf::instance()->get_chars("main", "node_name");
    int respLen = strlen(resp);
    int nameLen = strlen(nodeName);
    if(nameLen > 10)
    {
        nameLen = 10;
    }

    for(int i =0; i< nameLen; i++)
    {
        resp[respLen-2-10+i] = nodeName[i];
    }

    if(web_write( resp ) < 0)
    {
      TagLoger::log(Log_ClientServer, 0, " >Client Failed to send data %d\n",fd);
    }

    return -1;
}

int Client_connection::get_favicon_request(int client, int len, thread_data* local_buf)
{
    char resp[]="HTTP/1.1 301 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:CppComet Server\r\nComet-Server:CppComet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nCache-Control: max-age=3600\r\nConnection: close\r\nLocation: http://comet-server.com/favicon.ico\r\n\r\n";

    if(web_write( resp ) < 0)
    {
      TagLoger::log(Log_ClientServer, 0, " >Client Failed to send data %d\n",fd);
    }

    return -1;
}

/**
 * Из строки GET запроса вынет имя файла и вернёт путь к нему на диске или вернёт
 * пустую строку если операция не удалась или запрос пришёл не валидный.
 *
 * @note Если потом буду делать аналог файлов .htaccess то правки буду вносить гдето здесь.
 */
std::string get_file_name_from_HTTP_query(const char* message, int len)
{
    TagLoger::log(Log_ClientServer, 0, ">Client GET get_custom_request\n");
    const char *p = message;
    p = p + strlen("GET ");

    int urlEnd = str_find(p, ' ', 0, 0, len);
    if(urlEnd <= 0)
    {
        // 404
        return std::string();
    }

    const char *uri = p;

    if(strncmp(uri, "/comet-server", strlen("/comet-server")) == 0)
    {
        uri += strlen("/comet-server");
    }

    std::string name(appConf::instance()->get_string("main", "base_dir"));
    if(name.empty())
    {
        // 403
        return std::string();
    }

    name.append(uri, urlEnd - 1);

    auto pos = name.rfind('.');
    if(pos <= 0)
    {
        // 404
        return std::string();
    }

    TagLoger::log(Log_ClientServer, 0, " >Client GET [%s]\n", name.data());
    if(name.find("..") != std::string::npos)
    {
        // Проверка на две точки в урле.
        TagLoger::log(Log_ClientServer, 0, " >Client GET error403 [.. in url] [%s]\n", name.data());
        return std::string();
    }

    return name;
}

// https://app.comet-server.ru/api/CometServerApi.js
/**
 * Обработка произвольного GET запроса
 * @param client
 * @param len
 * @param local_buf
 * @return
 */
int Client_connection::get_custom_request(int client, int len, thread_data* local_buf)
{
    TagLoger::log(Log_ClientServer, 0, ">Client GET get_custom_request\n");

    std::string name = get_file_name_from_HTTP_query(local_buf->buf.getData(), len);
    if(name.empty())
    {
        // 404
        return http404_answer(client, len, local_buf);
    }

    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:%s; charset=UTF-8\r\nServer:CppComet Server\r\nComet-Server:CppComet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods: GET\r\nAllow: GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nCache-Control: max-age=3600\r\nConnection: close\r\n\r\n";

    auto pos = name.rfind('.');
    std::string ext = name.substr(pos+1, 10);
    std::string headers(appConf::instance()->get_string("content-type", ext));
    if(headers.empty())
    {
        TagLoger::log(Log_ClientServer, 0, " >Client GET [%s][ext=%s] not found\n", name.data(), ext.data());
        // 404
        return http404_answer(client, len, local_buf);
    }

    char headers_resp[1024];
    snprintf(headers_resp, 1024, resp, headers.data());

    int fp = open(name.data(), O_RDONLY);
    if(fp < 0)
    {
        // 404
        TagLoger::log(Log_ClientServer, 0, " >Client GET error404 [%s]\n", name.data(), errno);
        perror("error404:");
        return http404_answer(client, len, local_buf);
    }

    web_write(headers_resp);
    //std::string response(headers_resp);

    int size = 0;
    while(size = read(fp, local_buf->answer_buf.getData(),  local_buf->answer_buf.getSize()))
    {
        if(size <= 0)
        {
            break;
        }

        //response.append(local_buf->answer_buf.getData());
        web_write( local_buf->answer_buf.getData(), size);
        TagLoger::debug(Log_ClientServer, 0, " >send name=%s from disk [size=%d]\n", name.data(), size);
    }

    //ram_file_cache.insert(std::pair<std::string,const char*>(name, response.data()));
    return -1;
}


// @todo уяснить как вебсервер отдаёт большие объёмы данных по http
// @todo уяснить как фрагментировать ответ и как это делает php
int Client_connection::cultivate_custom_request(int client, const char* message, int len, thread_data* local_buf)
{
    TagLoger::log(Log_ClientServer, 0, ">Client GET get_custom_request\n");

    std::string name = get_file_name_from_HTTP_query(local_buf->buf.getData(), len);
    if(name.empty())
    {
        // 404
        return http404_answer(client, len, local_buf);
    }

    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:%s; charset=UTF-8\r\nServer:CppComet Server\r\nComet-Server:CppComet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods: GET\r\nAllow: GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nCache-Control: max-age=3600\r\nConnection: close\r\n\r\n";

    auto pos = name.rfind('.');
    std::string ext = name.substr(pos+1, 10);
    std::string headers(appConf::instance()->get_string("content-type", ext));
    if(headers.empty())
    {
        TagLoger::log(Log_ClientServer, 0, " >Client GET [%s][ext=%s] not found\n", name.data(), ext.data());
        // 404
        return http404_answer(client, len, local_buf);
    }

    char headers_resp[1024];
    snprintf(headers_resp, 1024, resp, headers.data());

    int fp = open(name.data(), O_RDONLY);
    if(fp < 0)
    {
        // 404
        TagLoger::log(Log_ClientServer, 0, " >Client GET error404 [%s]\n", name.data(), errno);
        perror("error404:");
        return http404_answer(client, len, local_buf);
    }

    web_write(headers_resp);
    //std::string response(headers_resp);

    int size = 0;
    while(size = read(fp, local_buf->answer_buf.getData(),  local_buf->answer_buf.getSize()))
    {
        if(size <= 0)
        {
            break;
        }

        //response.append(local_buf->answer_buf.getData());
        web_write( local_buf->answer_buf.getData(), size);
        TagLoger::debug(Log_ClientServer, 0, " >send name=%s from disk [size=%d]\n", name.data(), size);
    }

    //ram_file_cache.insert(std::pair<std::string,const char*>(name, response.data()));
    return -1;
}

/**
 * Обрабатывает сообщения от клиентов
 * @param client идентификатор клиента
 * @param text Сообщение
 * @param len Длина сообщения
 * @return 0 в случае успеха
 *
 *
 *
 * @note для того чтоб стать вебсервером надо включить поддержку
 *  POST запросов, Причём больших запросов и работу с бинарными данными в запросах.
 *  В целом не мало работы, но решаемо на самом деле.
 *
 * @note Чтоб отдавать данные эффективно надо бы ещё и прикрутить поддержку сжатия gzip и плюшки http версии болше чем 1.0
 *
 *
 *
 *
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
            //for(int i =0; i< len; i++)
            //{
            //    TagLoger::log(Log_ClientServer, 0, "![%c|%d]",local_buf->buf[i], (unsigned char)local_buf->buf[i]);
            //}
        }
        else
        {
            TagLoger::log(Log_ClientServer, 0, "Processing WS request - [Client_connection] -----------------------%d\n",len);
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
            TagLoger::log(Log_ClientServer, 0, "\x1b[1;32mAssembled HTTP WS GET header len=%d\x1b[0m\n[%s]\n",len, (char*)local_buf->buf);
        }

        if(local_buf->buf[len-2] != '\r' && local_buf->buf[len-1] != '\n' && memcmp((char*)local_buf->buf, "POST", strlen("POST"))!=0 )
        {
            // Все запросы кроме POST заканчиваются на \r\n по этому для них выполняем сборку из нескольких пакетов
            // Для POST запроса не выполняем так как функционал long polling не развивается и планируется к удалению

            fragment_buf.lock();
            fragment_buf.setSize(len);
            TagLoger::debug(Log_ClientServer, 0, "\x1b[1;31mThe HTTP WS GET header did not fully come len=%d\x1b[0m\n[%s]\n",len, (char*)local_buf->buf);
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
              || memcmp((char*)local_buf->buf, "GET //comet-server/ws/", strlen("GET //comet-server/ws/")) == 0
              || memcmp((char*)local_buf->buf, "GET /comet-server/ws/", strlen("GET /comet-server/ws/")) == 0)
        {
            // GET Вебсокет
            TagLoger::log(Log_ClientServer, 0, " >WS GET\n");
            local_buf->setThreadStatus('W');
            connection_type = REQUEST_WS;
            r = web_socket_request(client,len, local_buf);
        }
        else if( memcmp((char*)local_buf->buf, "GET /info ", strlen("GET /info ")) == 0
              || memcmp((char*)local_buf->buf, "GET /comet-server/info ", strlen("GET /comet-server/info ")) == 0 )
        {
            // GET Из браузера
            TagLoger::log(Log_ClientServer, 0, " >HTTP GET\n");

            connection_type = REQUEST_GET;
            r = get_info_request(client,len, local_buf);
        }
        else if( memcmp((char*)local_buf->buf, "GET /favicon.ico ", strlen("GET /favicon.ico ")) == 0
              || memcmp((char*)local_buf->buf, "GET /comet-server/favicon.ico ", strlen("GET /comet-server/favicon.ico ")) == 0 )
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
            TagLoger::log(Log_ClientServer, 0, " >HTTP Long-Polling Protocol (Disconnected!)\n");
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
        local_buf->setThreadStatus('Q');
        TagLoger::log(Log_ClientServer, 0, " >HTTP REQUEST_WS\n");
        r = web_socket_request_message(client,len, local_buf);
    }

    if(isAuthUser)
    {
        devManager::instance()->getDevInfo(web_user_dev_id)->incrMessages();
    }
    //pthread_mutex_unlock(&request_mutex);
    return r;
}

/**
 *  Инкремент количиства людей онлайн
 */
bool Client_connection::online_incr(thread_data* local_buf)
{
    devManager::instance()->getDevInfo(web_user_dev_id)->incrFrontendOnline();
    return true;
}

/**
 * @return
 */
bool Client_connection::online_decr(thread_data* local_buf)
{
    devManager::instance()->getDevInfo(web_user_dev_id)->decrFrontendOnline();
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

    TagLoger::log(Log_ClientServer, 0, "| Sending data message[web_user_id=%ld, msg_len=%d, connection_type=%d, fd=%d]\n", web_user_id, msg_len, connection_type, fd);

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
    //printHexMin(messge, answer_messge_len, Log_ClientServer);

    TagLoger::log(Log_ClientServer, 0, "]->write\x1b[0m");

    if(web_write(messge, answer_messge_len)  < 0)
    {
        TagLoger::log(Log_ClientServer, 0, " >Client Failed to send data %d\n",this->fd);
        ret = -1;
    }

    local_buf->messge_buf.unlock();

    //delete messge;
    devManager::instance()->getDevInfo(web_user_dev_id)->incrMessages();

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
 * @param local_buf
 * @param IsOnLine статус который надо постивить online или offline
 * @return
 */
int Client_connection::set_online(thread_data* local_buf)
{
    if(isOnLine)
    {
        TagLoger::warn(Log_ClientServer, 0, "\x1b[31mset_online: Recalling fd=%d\x1b[0m\n", fd);
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
        TagLoger::log(Log_ClientServer, 0, "\x1b[31mset_offline: Recalling fd=%d\x1b[0m\n", fd);
        return web_close();
    }

    online_decr(local_buf);
    //pthread_mutex_lock(&request_mutex);
    isOnLine = false;
    isAuthUser = false;
    devManager::instance()->getDevInfo(web_user_dev_id)->index->un_link(local_buf, web_user_id, fd);

    un_subscription(local_buf);
    int close = web_close();
    connection_type = REQUEST_NULL;


    web_user_dev_id = 0;
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
