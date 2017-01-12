// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


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

#ifndef INTERNAL_API_H
#define	INTERNAL_API_H

class internalApi{ 
public:
    /**
     * @todo все вызовы отправки в канал как из cometQL так и из других мест наладить через эту функцию.
     */
    static int send_event_to_pipe(thread_data* local_buf, const char* pipe_name, const char* msg_data, const char* server_data); 
    
    
    static int send_to_user(thread_data* local_buf, int user_id, const char* pipe_event, const char* msg_data);
    static int send_to_user(thread_data* local_buf, int user_id, const char* pipe_event, const char* msg_data, const char* server_data);
};

#endif	/* INTERNAL_API_H */