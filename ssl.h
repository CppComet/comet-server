// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* 
 * File:   ssl.h
 * Author: victor
 *
 * Created on 17 Январь 2013 г., 10:18
 */

/**
 * Поддержка SSL реализована не полностью и не работает.
 * Но заготовки кода для работы с https на прямую а не через реверс прокси из кода не удалены ещё.
 */
#ifndef SSL_H
#define	SSL_H

static void sigpipe_handle(int x){  TagLoger::log(Log_Any, 0, "\x1b[1;31msigpipe_handle %d\x1b[0m\n", x);}
 //SSL_CTX *initialize_ctx(const char *key_file,const char *pem_file);
 

/*
#define PORT	4333
int tcp_listen()
{
    int sock;
    struct sockaddr_in sin;
    int val=1;

    if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
      TagLoger::log(Log_Any, 0, "Couldn't make socket");

    memset(&sin,0,sizeof(sin));
    sin.sin_addr.s_addr=INADDR_ANY;
    sin.sin_family=AF_INET;
    sin.sin_port=htons(PORT);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));

    if(bind(sock,(struct sockaddr *)&sin,
      sizeof(sin))<0)
      TagLoger::log(Log_Any, 0, "Couldn't bind");
    listen(sock,5);

    return(sock);
}*/

void* apps_ssl_info_callback(SSL *s, int where, int ret);

#endif	/* SSL_H */

