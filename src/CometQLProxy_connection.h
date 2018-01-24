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
 

#define	PROXY_TO_ALL -1
#define	PROXY_TO_RANDOM -2


class CometQLProxy_connection:public MySql_connection
{
    friend class tcpServer<CometQLProxy_connection>;
 
public:
       
    CometQLProxy_connection();
    virtual ~CometQLProxy_connection();
 
protected:
   
    int proxy_union_select(int node, const char* query, thread_data* local_buf, unsigned int PacketNomber); 
    
    int proxy_insert(int node, const char* query, thread_data* local_buf, unsigned int PacketNomber);
    
    int proxy_query(int node, thread_data* local_buf, unsigned int PacketNomber);
    
    virtual int query_router(thread_data* local_buf, int PacketNomber);
    
    int sql_select_from_pipes(thread_data* local_buf, unsigned int PacketNomber);
    int sql_union_select_from_users_time(thread_data* local_buf, unsigned int PacketNomber);
};

#endif	/* COMETQLPROXY_CONNECTION_H */

