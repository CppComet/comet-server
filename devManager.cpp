// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "sha1.h"
#include "base64.h"
#include "devManager.h"

void devInfo::setDevKey(const char* devKey)
{
    if(key == NULL)
    {
        key = new char[DEV_KEY_LEN+1];
    }
    bzero(key, DEV_KEY_LEN+1);
    strncpy(key,devKey,DEV_KEY_LEN);
}
 
const char* devInfo::getDevUrl(int i) const
{
    if(i < 0 || i > urls_count)
    {
        return NULL;
    }
    
    return urls[i];
}

int devInfo::countDevUrl() const
{
    return urls_count;
}

void devInfo::setDevUrl(const char* devUrl)
{
    if(devUrl[0] == '*')
    {
        return;
    }
    
    if(url == NULL)
    {
        url = new char[DEV_URL_LEN+1];
    }
    bzero(url, DEV_URL_LEN+1);
    strncpy(url,devUrl,DEV_URL_LEN);
    
    char* start = url;
    
    urls_count =0;
    for(int i = 0; i<DEV_URL_LEN; i++)
    {
        if(url[i] == 0)
        {
            break;
        }
        
        if(url[i] == ',')
        {
            url[i] = 0;
            if(urls == NULL)
            {
                urls = new char*[DEV_MAX_URLS];
            }
            
            urls[urls_count] = start;
            start = url + i + 1;
            urls_count++;
        }
    }
}
  
devInfo::devInfo(const char* dev_key)
{ 
    setDevKey(dev_key);
    index = new user_index(); 
    active = true;
    pthread_mutex_init(&pipe_index_mutex,NULL);
}

devInfo::~devInfo()
{
    delete index;
    delete[] key;
}

bool devInfo::testDevKey(const char* devKey) const
{
    if(key == NULL)
    {
        return false;
    }
    
    return memcmp(key,devKey,DEV_KEY_LEN) == 0;
}

const char* devInfo::getDevKey() const
{
    return key;
}

/**
 * Проверяет DevKey в соответсвии с механизмом авторизации Mysql
 * @param dev_id
 * @param random20bytes
 * @param DevKeyHashStart
 * @return
 */
bool devInfo::testDevKey(const char* random20bytes, const char* DevKeyHashStart, thread_data* local_buf)
{ 
    char* devKeyPassword = appConf::instance()->rootPassword; 
    
    unsigned char sha1_password[20];
    bzero(sha1_password, 20);
    sha1::calc(devKeyPassword, DEV_KEY_LEN ,sha1_password);

    unsigned char sha1_password2[20];
    bzero(sha1_password2, 20);
    sha1::calc(sha1_password, 20 ,sha1_password2);

    unsigned char part2[40];
    memcpy(part2, random20bytes, 20);
    memcpy(part2+20, sha1_password2, 20);

    unsigned char sha1_part2[20];
    sha1::calc(part2, 40 ,sha1_part2);

    unsigned char xorRes[20];
    for(int i=0; i<20; i++)
    {
        xorRes[i] = sha1_password[i] xor sha1_part2[i];
    }

    if(memcmp(xorRes,DevKeyHashStart,20) == 0 )
    {
        return true;
    }

    return false;
}

void devInfo::incrMessages()
{
    messages++; 
    devManager::instance()->addNetworkEvents(); 
}


//**************************
//* devManager
//*************************/

devManager* devManager::inst = NULL;

devManager* devManager::instance()
{
   if (devManager::inst == 0)
   {
        devManager::inst = new devManager();
   }
   return devManager::inst;
}

devManager::~devManager()
{ 
    delete[] index;
}
 
devInfo* devManager::getDevInfo()
{
    return index;
} 