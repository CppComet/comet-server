 
/**
 * Надо ли вообще проверять с какого урла пишёл запрос.
 */
#define testUrl
   
//#define monoURL "78.47.206.108,inparadise.info,demo.inparadise.info,n2.comet-server.com,chat.inparadise.info,demo-chat.inparadise.info,int.dev,social-chat.comet-server.com,chat.emed24.ru"
//#define monoUrlsCount 8
      
/**
 * Максимальная длина имени канала 
 */
#define	PIPE_NAME_LEN           64 
#define	EVENT_NAME_LEN          64
 
# define ARRAY_BUFFER_SIZE 64

#define DEV_KEY_LEN 64
#define DEV_URL_LEN 255
#define DEV_MAX_URLS 10


#define NULL_IP "0.0.0.0" 
 
#define NODE_NAME "NoName" 
 
#define LOG_ALL     5
#define LOG_DEBUG   4
#define LOG_INFO    3
#define LOG_WARN    2
#define LOG_ERROR   1
#define LOG_NONE    0

/**
 * Максимальное количество нод в кластере с котороыми может быть связан этот экземпляр сервера
 */
#define MAX_HL_CLUSTER_SIZE 16

/**
 * Если идентификатор соединения равен этой констане значит соединение не валидное
 */
#define  USER_INDEX_NO_CONNECT -1

/**
 * Длина хеша авторизации пользователя
 */
#define  USER_HASH_LEN 32

#define  MAX_MAP_SIZE 500

/**
 * Максимальное количество соединений на одного пользователя.
 */
#define  MAX_CONECTION_ON_USER_ID 16


/*
#define printBase printf("\x1b[0m")

#define printBlue printf("\x1b[34m")
#define printBBlue printf("\x1b[1;34m")

#define printBRed printf("\x1b[1;31m")
#define printRed printf("\x1b[31m")

#define printGreen printf("\x1b[32m")
#define printBGreen printf("\x1b[1;32m")
 */
  
    #define logoPage "<!DOCTYPE html>\
    <html>\
    <head>\
     <meta charset='utf-8'>\
     <title>Star.Comet</title>\
     <link rel='shortcut icon' href='https://comet-server.com/favicon.ico' type='image/x-icon'>\
     <style type='text/css'>\
        #center {\
                    top: 50%;\
                    left: 50%;\
                    width: 650px;\
                    height: 100px;\
                    position: absolute;\
                    margin-top: -50px;\
                    margin-left: -325px;\
            }\
     </style>\
    </head>\
    <body> \
                    <table id='center'>\
                            <tr style='vertical-align: top;'>\
                                    <td> <img src='https://comet-server.org/Logo.png'></td>\
                                <td style='  padding-left: 10px;'> \
                                        <div style='font-size: 30px;  font-weight: bold;'>Star.Comet</div>\
                                            Comet сервер Star.Comet<br>\
                                            Больше информации смотрите на comet-server.org и support@comet-server.org\
                                    </td>\
                            </tr>\
            </table> \
    </body>\
    </html>" 
 
#define MYSQL_SERVERNAME "CometServer 2.02"

#define maxValue( a, b ) ( (a) > (b) ? (a) : (b) )
#define minValue( a, b ) ( (a) < (b) ? (a) : (b) )

#include "TagTimer.h"
#include "TagLoger.h"
