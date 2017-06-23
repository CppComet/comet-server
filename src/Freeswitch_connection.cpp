// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   Freeswitch_connection.h
 * Author: levha
 *
 * Created on 31 Октября 2016
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
#include "Freeswitch_connection.h" 
#include "MySql_connection.h"
#include "devManager.h"

#include "internalApi.h"

#define	REQUEST_NULL            0
#define	REQUEST_OPTIONS         1
#define	REQUEST_GET             2
#define	REQUEST_LONG_POLLING    3
#define	REQUEST_POST            3 // Все LONG_POLLING отправляются как POST запросы
#define	REQUEST_WS              4

Freeswitch_connection::Freeswitch_connection():connection(),fragment_buf(appConf::instance()->get_int("main", "fragment_buf_size"))
{
}

Freeswitch_connection::~Freeswitch_connection(){}

/**
 * Отвечает  на запрос options
 * @param client
 * @param len
 * @param local_buf
 */
int Freeswitch_connection::options(int client, int len, thread_data* local_buf)
{
    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:Star.Comet Server\r\nComet-Server:Star.Comet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\n\r\n+OK\r\n";

    if(web_write( resp ) < 0)
    {
      TagLoger::log(Log_FreeswitchServer, 0, " >Client Не удалось отправить данные %d\n", fd);
    }

    return -1;
}

/**
 * Отвечает  на запрос get
 * @param client
 * @param len
 * @param local_buf
 */
int Freeswitch_connection::get_request(int client, int len, thread_data* local_buf)
{
    char resp[]="HTTP/1.1 200 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:Star.Comet Server\r\nComet-Server:Star.Comet Server\r\nAccess-Control-Allow-Origin: *\
    \r\nAccess-Control-Allow-Methods:POST, GET\r\nAllow: POST, GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\n\r\n";

    if(web_write( resp ) < 0 || web_write( logoPage ) < 0)
    {
      TagLoger::log(Log_FreeswitchServer, 0, " >Client Не удалось отправить данные %d\n", fd);
    }
    return -1;
}

/**
 * Отвечает  на запрос get /directory
 * @param client
 * @param len
 * @param local_buf
 *
 * @see https://freeswitch.org/confluence/display/FREESWITCH/mod_xml_curl
 *
 * GET /directory?hostname=n6-app.comet.su
 * &section=directory
 * &tag_name=domain
 * &key_name=name
 * &key_value=194.67.209.39
 * &Event-Name=REQUEST_PARAMS
 * &Core-UUID=2d238269-084a-4243-9b4b-b3bdd54fa4bc
 * &FreeSWITCH-Hostname=n6-app.comet.su
 * &FreeSWITCH-Switchname=n6-app.comet.su
 * &FreeSWITCH-IPv4=194.67.209.39
 * &FreeSWITCH-IPv6=2a02%3Af680%3A1%3A1102%3A%3A943
 * &Event-Date-Local=2016-11-04%2007%3A21%3A31
 * &Event-Date-GMT=Fri,%2004%20Nov%202016%2004%3A21%3A31%20GMT
 * &Event-Date-Timestamp=1478233291260338
 * &Event-Calling-File=sofia_reg.c
 * &Event-Calling-Function=sofia_reg_parse_auth
 * &Event-Calling-Line-Number=2819
 * &Event-Sequence=2024
 * &action=sip_auth
 * &sip_profile=internal
 * &sip_user_agent=unknown
 * &sip_auth_username=103
 * &sip_auth_realm=194.67.209.39
 * &sip_auth_nonce=fe29fe2e-176f-47eb-b85e-324ad0c35151
 * &sip_auth_uri=sip%3A194.67.209.39
 * &sip_contact_user=103
 * &sip_contact_host=185.40.4.83
 * &sip_to_user=103
 * &sip_to_host=194.67.209.39
 * &sip_via_protocol=udp
 * &sip_from_user=103
 * &sip_from_host=194.67.209.39
 * &sip_call_id=d762cc971d1647c729940574cd6a28d3
 * &sip_request_host=194.67.209.39
 * &sip_auth_qop=auth
 * &sip_auth_cnonce=cb863d74
 * &sip_auth_nc=00000001
 * &sip_auth_response=bd3aa782ef7be0478986885476345f16
 * &sip_auth_method=REGISTER
 * &client_port=47400
 * &key=id
 * &user=103
 * &domain=194.67.209.39
 * &ip=185.40.4.83
 * HTTP/1.1
 * Authorization: Basic dXNlcjpwYXNz
 * User-Agent: freeswitch-xml/1.0
 * Host: n6-app.comet.su:84
 * Accept: * / *
 *
 */
int Freeswitch_connection::get_directory_request(int client, int len, thread_data* local_buf)
{
    int userId = 1;
    char user[64];
    bzero(user, 64);

    char sip_to_host[64];
    bzero(sip_to_host, 64);
 
    char *p = local_buf->buf.getData();
    int pos = 0;
    while (pos = str_find(p, '&'))
    {
        if(pos <= 0)
        {
            break;
        }

        p += pos+1;
        if(memcmp(p, "user=", strlen("user=")) == 0)
        {
            p = p +  strlen("user=");
            int len = str_find(p, '&');
            if(len <= 0 || len >= 64)
            {
                TagLoger::log(Log_FreeswitchServer, 0, " >Error len=%d, user:%s\n",len, p);
                return -1;
            }

            memcpy(user, p, len);
            TagLoger::log(Log_FreeswitchServer, 0, " >len=%d, user:%s\n",len, user);
        }
        else if(memcmp(p, "sip_to_host=", strlen("sip_to_host=")) == 0)
        {
            p = p +  strlen("sip_to_host=");
            int len = str_find(p, '&');
            if(len <= 0 || len >= 64)
            {
                TagLoger::log(Log_FreeswitchServer, 0, " >Error len=%d, sip_to_host:%s\n",len, p);
                return -1;
            }

            memcpy(sip_to_host, p, len);
            TagLoger::log(Log_FreeswitchServer, 0, " >len=%d, sip_to_host:%s\n",len, sip_to_host);
        }
    }
    
    if(strlen(user) < 7)
    {
        return -1;
    }
     
    sscanf(user, "%9d", &userId);

    char hash[USER_HASH_LEN+1];  
    bzero(hash, USER_HASH_LEN+1);
    
    if(!devManager::instance()->getDevInfo()->index->get_hash(local_buf, userId, hash))
    {
        TagLoger::log(Log_FreeswitchServer, 0, "\x1b[35mНет данных авторизации user:%d [%s]\x1b[0m\n", userId, user);
        return -1;
    }
    TagLoger::log(Log_FreeswitchServer, 0, "\x1b[34mАвторизация пройдена user:%d [%s], hash=%s\x1b[0m\n", userId, user, hash);

    local_buf->answer_buf.lock();
    snprintf(local_buf->answer_buf, local_buf->answer_buf.getSize(),
            "HTTP/1.1 200 OK\r\nContent-Type:text/html; charset=UTF-8\r\nServer:Star.Comet Server\r\nComet-Server:Star.Comet Server\r\nAccess-Control-Allow-Origin: *\r\n\
Access-Control-Allow-Methods:GET\r\nAllow: GET\r\nAccess-Control-Allow-Headers: origin, content-type, accept\r\nConnection: close\r\n\r\n\
<document type=\"freeswitch/xml\">\n\
    <section name=\"directory\">\n\
        <domain name=\"FSdefaultDomain\">\n\
            <params>\n\
                <param name=\"dial-string\" value=\"{presence_id=${dialed_user}@${dialed_domain}}${sofia_contact(${dialed_user}@${dialed_domain})}\"/>\n\
            </params>\n\
            <groups>\n\
                <group name=\"default\">\n\
                    <users>\n\
                        <user id=\"%s\">\n\
                            <params>\n\
                                <param name=\"password\" value=\"%s\"/>\n\
                            </params>\n\
                            <variables>\n\
                                <variable name=\"toll_allow\" value=\"domestic,international,local\"/>\n\
                                <variable name=\"accountcode\" value=\"%s\"/>\n\
                                <variable name=\"user_context\" value=\"default\"/>\n\
                                <variable name=\"effective_caller_id_name\" value=\"Extension %s\"/>\n\
                                <variable name=\"effective_caller_id_number\" value=\"%s\"/>\n\
                                <variable name=\"outbound_caller_id_name\" value=\"$${outbound_caller_name}\"/>\n\
                                <variable name=\"outbound_caller_id_number\" value=\"$${outbound_caller_id}\"/>\n\
                                <variable name=\"callgroup\" value=\"techsupport\"/>\n\
                            </variables>\n\
                        </user>\n\
                    </users>\n\
                </group>\n\
            </groups>\n\
        </domain>\n\
    </section>\n\
</document>", user, hash, user, user, user);
    if(web_write( local_buf->answer_buf ) < 0 )
    {
      TagLoger::log(Log_FreeswitchServer, 0, " >Client Не удалось отправить данные %d\n", fd);
    }
    local_buf->answer_buf.unlock();
    return -1;
} 
/**
 * Обрабатывает сообщения от клиентов
 * @param client идентификатор клиента
 * @param text Сообщение
 * @param len Длина сообщения
 * @return 0 в случае успеха
 */
int Freeswitch_connection::request(int client, int len, thread_data* local_buf)
{
    // если len = 0 то соединение закрыто.
    if(len == 0)
    {
        return -1;
    }

    TagLoger::debug(Log_FreeswitchServer, 0, " >request [%s]\n",local_buf->buf.getData());

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
            TagLoger::log(Log_FreeswitchServer, 0, "\x1b[1;32mСобран HTTP WS GET заголовок len=%d\x1b[0m\n[%s]\n",len, (char*)local_buf->buf);
        }

        if(local_buf->buf[len-2] != '\r' && local_buf->buf[len-1] != '\n')
        {
            // Все запросы кроме POST заканчиваются на \r\n по этому для них выполняем сборку из нескольких пакетов
            fragment_buf.lock();
            fragment_buf.setSize(len);
            TagLoger::log(Log_FreeswitchServer, 0, "\x1b[1;31mНе полностью пришёл HTTP GET заголовок len=%d\x1b[0m\n[%s]\n",len, (char*)local_buf->buf);
            memcpy(fragment_buf.getData(), local_buf->buf.getData(), len);
            return 0;
        }

        if(  memcmp((char*)local_buf->buf, "OPTIONS", strlen("OPTIONS")) == 0)
        {
            TagLoger::log(Log_FreeswitchServer, 0, " >HTTP OPTIONS\n");

            connection_type = REQUEST_OPTIONS;
            r = options(client,len, local_buf);
        } 
        else if( memcmp((char*)local_buf->buf, "GET /directory", strlen("GET /directory")) == 0 || memcmp((char*)local_buf->buf, "GET /comet-server-com/directory", strlen("GET /comet-server-com/directory")) == 0 )
        {
            // GET /directory от Freeswitch
            TagLoger::log(Log_FreeswitchServer, 0, " >HTTP GET /directory\n");

            connection_type = REQUEST_GET;
            r = get_directory_request(client,len, local_buf);
        }
        else if( memcmp((char*)local_buf->buf, "GET", strlen("GET")) == 0 )
        {
            // GET Из браузера
            TagLoger::log(Log_FreeswitchServer, 0, " >HTTP GET\n");

            connection_type = REQUEST_GET;
            r = get_request(client,len, local_buf);
        }
    }
    return r;
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
int Freeswitch_connection::set_online(thread_data* local_buf)
{
    if(isOnLine)
    {
        TagLoger::warn(Log_FreeswitchServer, 0, "\x1b[31mset_online: Повторный вызов fd=%d\x1b[0m\n", fd);
        return 0;
    }
    start_online_time = time(0);

    TagLoger::log(Log_FreeswitchServer, 0, "set_online = fd:%d\n",fd);

    isOnLine = true;
    return 0;
}

int Freeswitch_connection::set_offline(thread_data* local_buf)
{
    if(!isOnLine)
    {
        TagLoger::log(Log_FreeswitchServer, 0, "\x1b[31mset_offline: Повторный вызов fd=%d\x1b[0m\n", fd);
        return 0;
    }

    //pthread_mutex_lock(&request_mutex);
    isOnLine = false;
    int close = web_close();
    connection_type = REQUEST_NULL;


    start_online_time = 0;

    hasFragmentInBuffer = false;
    //pthread_mutex_unlock(&request_mutex);

    if(close < 0)
    {
        TagLoger::error(Log_FreeswitchServer, 0, "\x1b[31mset_online:web_close() = -1, fd=%d\x1b[0m\n", fd);
    }

    return close;
}
