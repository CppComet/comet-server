// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

 
#include "sha1.h"
#include "base64.h"
#include "devManager.h"
 

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

devInfo::devInfo(int dev_id)
{
    id = dev_id;
    index = new user_index(dev_id);
    pthread_mutex_init(&pipe_index_mutex,NULL);
}






devInfo::~devInfo()
{
    delete index;
}

bool devInfo::testDevKey(const char* devKey) const
{
                    return memcmp(appConf::instance()->get_chars("main", "password"), devKey, DEV_KEY_LEN) == 0;

  
}


/**
 * Проверяет DevKey в соответсвии с механизмом авторизации Mysql
 * @param dev_id
 * @param random20bytes
 * @param DevKeyHashStart
 * @return
 */
bool devInfo::testDevKey(const char* random20bytes, const char* DevKeyHashStart)
{

    const char* devKeyPassword = appConf::instance()->get_chars("main", "password");

    unsigned char sha1_password[20];
    bzero(sha1_password, 20);
    sha1::calc(devKeyPassword, strlen(devKeyPassword), sha1_password);

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
        if(dev_index_size > 0)
        {
            for(int i =0; i< dev_index_size; i++)
            {
                delete index[i];
            }
            delete[] index;
        }
}

/**
 * Устанавливает максимальное количество разных dev_id и создаёт под них масив user_index**
 * @param size
 */
void devManager::setDevIndexSize(int size)
{
    TagLoger::log(Log_devManager, 0, "devManager::setDevIndexSize dev_index_size=%d\n", dev_index_size);

    if(index == NULL)
    {
        dev_index_size = size;
        index = new devInfo*[size];
        bzero(index, size *sizeof(devInfo*));

        /**
         * Инициализация первого devInfo
         * Он будет возвращатся во всех случаях кода getDevInfo(int dev_id) будет получать не валидный dev_id
         */ 
            index[0] = new devInfo(0);
    }
    else if(dev_index_size > size)
    {
        dev_index_size = size;
    }
}

devInfo* devManager::getDevInfo(int dev_id)
{
    if(dev_id < 0 || dev_id >= dev_index_size)
    {
        if(dev_id > 0 )
        {
            TagLoger::error(Log_devManager, 0, "Запрвшивается не доступный devInfo с dev_id=%d, dev_index_size = %d\n",dev_id, dev_index_size);
        }
        //print_backtrace("Запрвшивается не доступный devInfo");
        return index[0];
    }

    if( index[dev_id] == NULL)
    {
        index[dev_id] = new devInfo(dev_id);
    }

    return index[dev_id];
}