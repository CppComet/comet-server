// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   Client_connection.cpp
 * Author: levha
 *
 * Created on 01 Май 2015 г., 13:20
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <vector>
#include <map>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "mystring.h"
#include "user_index.h"
#include "internalApi.h"
#include "Client_connection.h"
#include "CometQLProxy_connection.h" 

#include "CometQL.h"
#include "y.tab.h"
#include "sha1.h"
#include "CometQLcluster.h"
#include "devManager.h"
#include <ctime>

#include <openssl/sha.h>

#include "sha1.h"

CometQLProxy_connection::CometQLProxy_connection():MySql_connection()
{
    //printf("create CometQLProxy_connection\n");
}

CometQLProxy_connection::~CometQLProxy_connection()
{
    //printf("delete CometQLProxy_connection\n");
}
  

