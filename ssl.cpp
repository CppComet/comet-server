// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* 
 * File:   ssl.cpp
 * Author: victor
 * 
 * Created on 17 Январь 2013 г., 10:18
 */

/**
 * Поддержка SSL реализована не полностью и не работает.
 * Но заготовки кода для работы с https на прямую а не через реверс прокси из кода не удалены ещё.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include <openssl/ssl.h>
#include <openssl/err.h>


#include "TagLoger.h"
#include "ssl.h"


void* apps_ssl_info_callback(SSL *s, int where, int ret)
{
    const char *str;
    int w;
    w=where& ~SSL_ST_MASK;
    if (w & SSL_ST_CONNECT) str="SSL_connect";
    else if (w & SSL_ST_ACCEPT) str="SSL_accept";
    else str="undefined";
    if (where & SSL_CB_LOOP)
    {
            TagLoger::log(Log_Any, 0, "%s:%s\n",str,SSL_state_string_long(s));
    }
    else if (where & SSL_CB_ALERT)
    {
            str=(where & SSL_CB_READ)?"read":"write";
            TagLoger::log(Log_Any, 0, "SSL3 alert %s:%s:%s\n",
                    str,
                    SSL_alert_type_string_long(ret),
                    SSL_alert_desc_string_long(ret));
    }
    else if (where & SSL_CB_EXIT)
    {
            if (ret == 0)
                    TagLoger::log(Log_Any, 0, "%s:failed in %s\n",
                            str,SSL_state_string_long(s));
            else if (ret < 0)
            {
                    TagLoger::log(Log_Any, 0, "%s:error in %s\n",
                            str,SSL_state_string_long(s));
            }
    }
    
    return 0;
}
/*
int sslsock_handle_nbio (void *ssl, int ret, struct timeval  *totv)
{
    int     sfd, i;
    fd_set  rset, wset;

    sfd = SSL_get_fd (ssl);
    i = SSL_get_error (ssl, ret);

    if (i == SSL_ERROR_WANT_READ)
    {
        do {
            FD_ZERO (&rset);
            FD_SET (sfd, &rset);
            i = select (sfd + 1, &rset, NULL, NULL, totv);
        } while ((i < 0) && (errno == EINTR));
        if (i == 0) {
            // the select() timed out  
            ret = -2;
            errno = ETIMEDOUT;
        } else {
            // either an error, or it's readable  
            ret = i;
        }
    }
    else if (i == SSL_ERROR_WANT_WRITE)
    {
        do {
            FD_ZERO (&wset);
            FD_SET (sfd, &wset);
            i = select (sfd + 1, NULL, &wset, NULL, totv);
        } while ((i < 0) && (errno == EINTR));
        if (i == 0) {
            // the select() timed out  
            ret = -2;
            errno = ETIMEDOUT;
        } else {
            // either an error, or it's writable  
            ret = i;
        }
    }
    //else, leave "ret" alone, and return it as-is  

    return (ret);
}*/