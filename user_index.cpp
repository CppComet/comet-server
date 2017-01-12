// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef USER_INDEX_CPP
#define	USER_INDEX_CPP

#include "user_index.h"

time_t user_index::start_time=time(0);


#include "devManager.h"


#define SALT_LEN 10

/**
 * 
 * @param local_buf
 * @param token строка из 32 символов
 * @return 
 * 
 * http://redmine.comet-server.com/issues/65
 * http://redmine.comet-server.com/issues/67
 * http://redmine.comet-server.com/issues/68
 * 
 * http://hashlib2plus.sourceforge.net/example.html
 * https://learn.javascript.ru/csrf 
 * 
 * Hash вида
 * token = salt + MD5(salt + dev_key + user_id )
 * salt - всегда 10 символов 
 */
bool useritem::testToken(thread_data* local_buf, const char* token, int user_id)
{ 
    if(strlen(token) < 32)
    {
        return false;
    }
    
    const char* dev_key = devManager::instance()->getDevInfo()->getDevKey();
    if(dev_key == NULL)
    {
        return false;
    }

    char hashStr[256];
    bzero(hashStr, 256);
    
    memcpy(hashStr, token, SALT_LEN);
    memcpy(hashStr+SALT_LEN, dev_key, DEV_KEY_LEN); 
    snprintf(hashStr+SALT_LEN+DEV_KEY_LEN, 32, "%d", user_id);
    
    //printf("token %10s|%s\n", token, token+SALT_LEN);
    //printf("hashStr %s\n", hashStr);

    hashwrapper *myWrapper = new md5wrapper();
    std::string test_hash = myWrapper->getHashFromString(hashStr);

    
    //printf("test_hash %10s|%s\n", test_hash.data(), test_hash.data()+SALT_LEN);
    
    return memcmp(token+SALT_LEN, test_hash.data()+SALT_LEN, 32-SALT_LEN) == 0;
}
  

#endif	/* USER_INDEX_CPP */


