// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef USER_INDEX_CPP
#define	USER_INDEX_CPP

#include "user_index.h"

time_t user_index::start_time=time(0);


#include "devManager.h"


#define SALT_LEN 10

useritem::useritem()
{
    bzero(conection_ids, sizeof(int) * MAX_CONECTION_ON_USER_ID);
}

useritem::useritem(thread_data* local_buf, unsigned int User_id,int Conection_id)
{
    user_id = User_id;
    bzero(conection_ids, sizeof(int) * MAX_CONECTION_ON_USER_ID);
    setConection_id(local_buf, Conection_id);
}

useritem::useritem(thread_data* local_buf, unsigned int User_id,const char* Hash)
{
    user_id = User_id;

    int copy_len = strlen(Hash);
    if( copy_len > USER_HASH_LEN)
    {
        copy_len = USER_HASH_LEN;
    }

    hash = new char[USER_HASH_LEN+1];
    bzero(hash, USER_HASH_LEN+1);
    memcpy(hash, Hash, copy_len);

    TagLoger::log(Log_UserItem, 0, "auth_hash_test:%s\n", hash);

    local_buf->stm.users_auth_replace->execute(user_id, hash);

    bzero(conection_ids, sizeof(int) * MAX_CONECTION_ON_USER_ID);
}

/**
 * Добавляет id нового соединения к пользователю.
 * Если этот пользователь до этого не имел не одного соединения то вызовет ему setOnline_time()
 * @param local_buf
 * @param Conection_id
 * @return
 */
int useritem::setConection_id(thread_data* local_buf, int Conection_id)
{
    int isOnline = false;
    for(int i=0; i<MAX_CONECTION_ON_USER_ID; i++)
    {
        if(conection_ids[i] > 0)
        {
            isOnline = true;
            break;
        }
    }

    if(!isOnline)
    {
        setOnline_time(local_buf);
    }

    for(int i=0; i<MAX_CONECTION_ON_USER_ID; i++)
    {
        if(conection_ids[i] <= 0)
        {
            conection_ids[i] = Conection_id;
            TagLoger::log(Log_UserItem, 0, "\x1b[34mAdd Conection_id=%d to user_id=%d i=%d\x1b[0m\n", Conection_id,  user_id, i);
            return 1;
        }
    }

    TagLoger::log(Log_UserItem, 0, "\x1b[34mAdd Conection_id=%d to user_id=%d i=%d\x1b[0m\n", Conection_id,  user_id, -1);
    return 0;
}

/**
 * Массив идентификаторов соединений пользователя длиной MAX_CONECTION_ON_USER_ID
 * @return
 */
int* useritem::getConection_ids()
{
    return conection_ids;
}

/**
 * Отключает конкретный Conection_id для user_id, если это последниее соединение пользователя вызовет setOffline_time() для него
 * @param Conection_id
 * @return
 */
int useritem::unsetConection_id(thread_data* local_buf, int Conection_id)
{
    for(int i=0; i<MAX_CONECTION_ON_USER_ID; i++)
    {
        if(conection_ids[i] == Conection_id)
        {
            TagLoger::log(Log_UserItem, 0, "\x1b[34munsetConection_id=%d from user_id=%d, i=%d\x1b[0m\n", Conection_id,  user_id, i);
            conection_ids[i] = USER_INDEX_NO_CONNECT;
            break;
        }
    }

    for(int i=0; i<MAX_CONECTION_ON_USER_ID; i++)
    {
        if(conection_ids[i] > 0)
        {
            return 1;
        }
    }

    setOffline_time(local_buf);
    return 0;
}

/**
 * Возвращает имеющиеся значение, но если данных нет то обратится в редис.
 */
long useritem::getLast_online_time(thread_data* local_buf)
{
    if(last_online_time == -1)
    {
        local_buf->stm.users_time_select->execute(user_id);
        if(local_buf->stm.users_time_select->fetch())
        {
            local_buf->stm.users_time_select->free();
            return -1;
        }

        last_online_time = local_buf->stm.users_time_select->result_time;
        local_buf->stm.users_time_select->free();
    }

    return last_online_time;
}

/**
 * Возвращает имеющиеся значение
 */
int useritem::getLast_online_time()
{
    return last_online_time;
}

/**
 * Определяет время последней авторизации, берёт значение из редиса.
 * @param local_buf
 * @param user_id
 * @return
 */
long useritem::getLast_online_time(thread_data* local_buf, unsigned int user_id)
{
    if(!local_buf->stm.users_time_select->execute(user_id))
    {
        return -1;
    }

    if(local_buf->stm.users_time_select->fetch())
    {
        local_buf->stm.users_time_select->free();
        return -1;
    }

    long last_online_time = local_buf->stm.users_time_select->result_time;
    local_buf->stm.users_time_select->free();

    return last_online_time;
}

/**
 * Устанавливает хеш авторизации
 */
bool useritem::setHash(thread_data* local_buf, const char* Hash)
{
    if(hash == 0)
    {
        hash = new char[USER_HASH_LEN+1];
    }

    int copy_len = strlen(Hash);
    if( copy_len > USER_HASH_LEN)
    {
        copy_len = USER_HASH_LEN;
    }

    bzero(hash, USER_HASH_LEN+1);

    memcpy(hash, Hash, copy_len);
    TagLoger::log(Log_UserItem, 0, "auth_hash_test:%s\n", hash);
    local_buf->stm.users_auth_replace->execute(user_id, hash);
    return true;
}


/**
 * Удаляет хеш авторизации
 */
bool useritem::deleteHash(thread_data* local_buf)
{
    local_buf->stm.users_auth_delete->execute(user_id);
    if(hash == NULL)
    {
        return true;
    }

    delete[] hash;
    hash = NULL;
    return true;
}

/**
 * Вернёт ссылку на строку с хешем авторизации или NULL если данных нет.
 *
 * Данные берёт из редиса
 * @param local_buf
 * @return
 *
 * @deprecated В замен таблицы хешей пользователей решено переходить на алгоритм авторизации по токену
 */
bool useritem::getHash(thread_data* local_buf, unsigned int user_id, char* out_hash)
{
    local_buf->stm.users_auth_select->execute(user_id);

    if(local_buf->stm.users_auth_select->fetch())
    {
        TagLoger::log(Log_UserItem, 0, "[4]Hash, user_id=%d not found\n", user_id);
        local_buf->stm.users_auth_select->free();
        return false;
    }

    if(out_hash != NULL)
    {
        bzero(out_hash, USER_HASH_LEN);
        memcpy(out_hash, local_buf->stm.users_auth_select->result_hash, USER_HASH_LEN);
    }
    TagLoger::log(Log_UserItem, 0, "[5]Hash user_id=%d is %s\n", user_id, local_buf->stm.users_auth_select->result_hash);
    local_buf->stm.users_auth_select->free();
    return true;
}

bool useritem::deleteHash(thread_data* local_buf, unsigned int user_id)
{
    local_buf->stm.users_auth_delete->execute(user_id);
    return true;
}

/**
 * Вернёт ссылку на строку с хешем авторизации или NULL если данных нет.
 *
 * Данные берёт из редиса или если есть из памяти переменной hash
 * @param local_buf
 * @return
 */
bool useritem::getHash(thread_data* local_buf, char* out_hash)
{
    if(hash == 0)
    {
        local_buf->stm.users_auth_select->execute(user_id);
        if(local_buf->stm.users_auth_select->fetch())
        {
            local_buf->stm.users_auth_select->free();
            TagLoger::log(Log_UserItem, 0, "[3]Hash user_id=%d not found\n", user_id);
            return false;
        }

        hash = new char[USER_HASH_LEN+1];
        bzero(hash, USER_HASH_LEN+1);

        memcpy(hash, local_buf->stm.users_auth_select->result_hash, USER_HASH_LEN );
        local_buf->stm.users_auth_select->free();
        TagLoger::log(Log_UserItem, 0, "[1]Hash user_id=%d is %s\n", user_id, hash);
    }
    else
    {
        TagLoger::log(Log_UserItem, 0, "[2]Hash user_id=%d is %s\n", user_id, hash);
    }

    if(out_hash != NULL)
    {
        bzero(out_hash, USER_HASH_LEN);
        memcpy(out_hash, hash, USER_HASH_LEN);
    }
    return true;
}

/**
 * Проверяет хеш авторизации
 * @param local_buf
 * @param Hash
 * @return
 */
bool useritem::testHash(thread_data* local_buf, const char* Hash)
{
    // Если в друг нам пришёл не хеш авторизации а токен то проверим его как токен
    if(testToken(local_buf, Hash, user_id))
    {
        // Если токен верен но хеша авторизации нет то установим значение токена в качестве хеша авторизации
        if(!getHash(local_buf, NULL))
        {
            setHash(local_buf, Hash);
        }
        return true;
    }

    if(!getHash(local_buf, NULL))
    {
        return false;
    }


    int Hash_len = strlen(Hash);
    if(Hash_len > USER_HASH_LEN)
    {
        // Если хеш длинее USER_HASH_LEN то ограничемся USER_HASH_LEN
        Hash_len = USER_HASH_LEN;
    }


    int local_hash_len = strlen(hash);
    if(local_hash_len > USER_HASH_LEN)
    {
        // По какойто причине какимто неведомым образом hash может указывать на строку больше USER_HASH_LEN
        // Если хеш длинее USER_HASH_LEN то ограничемся USER_HASH_LEN
        local_hash_len = USER_HASH_LEN;
    }

    TagLoger::log(Log_UserItem, 0, "testHash: [%zu]%s = [%d]%s\n", local_hash_len, hash,  Hash_len, Hash);

    if(local_hash_len > Hash_len)
    {
        return false;
    }

    if(memcmp(hash, Hash, local_hash_len ) == 0)
    {
        return true;
    }

    return false;
}

/**
 * Проверяет хеш авторизации
 * @param local_buf
 * @param Hash
 * @param user_id
 * @return
 */
bool useritem::testHash(thread_data* local_buf, const char* Hash, unsigned int user_id)
{
    // Если в друг нам пришёл не хеш авторизации а токен то проверим его как токен
    if(testToken(local_buf, Hash, user_id))
    {
        // Если токен верен но хеша авторизации нет то установим значение токена в качестве хеша авторизации
        if(!getHash(local_buf, user_id, NULL))
        {
            local_buf->stm.users_auth_replace->execute(user_id, Hash);
        }
        return true;
    }

    local_buf->stm.users_auth_select->execute(user_id);
    if(local_buf->stm.users_auth_select->fetch())
    {
        local_buf->stm.users_auth_select->free();
        return false;
    }

    if(memcmp(Hash, local_buf->stm.users_auth_select->result_hash, USER_HASH_LEN) == 0)
    {
        local_buf->stm.users_auth_select->free();
        return true;
    }

    local_buf->stm.users_auth_select->free();
    return false;
}

useritem::~useritem()
{
    if(hash == 0)
    {
        delete[] hash;
    }
}

/**
 * Запоминает время ухода пользователя и сохраняет эту информацию в редис
 * Отправляет сообщение в канал user_status_{user_id} о том что человек offline 
 */
void useritem::setOffline_time(thread_data* local_buf)
{
    last_online_time = time(0);
    if(user_id > 0)
    {
        if(appConf::instance()->get_bool("main", "save_users_last_online_time"))
        {
            local_buf->db.query_format("replace into `users_time` (`user_id`, `time`) VALUES ('%d', '%d')", user_id, last_online_time);
        }
        if(appConf::instance()->get_bool("main", "send_user_offline_events"))
        {
            char pipe_name[100];
            snprintf(pipe_name, 100,"user_status_%d", user_id);
            internalApi::send_event_to_pipe(local_buf, pipe_name, "{\\\"data\\\":\\\"offline\\\",\\\"event_name\\\":\\\"offline\\\"}", NULL);

            if(local_buf->isWSClusterActive())
            {
                auto it = local_buf->wsCluster.begin();
                while(it != local_buf->wsCluster.end())
                {
                    auto link = *it; 
                    link->query_format("cometqlcluster_v1; INSERT INTO pipes_messages (name, event, message)VALUES('%s', 'offline', 'offline');", pipe_name);
                    it++;
                }
            }
        }
    }
}

/**
 * Устанавливает пользователю статус online и сохраняет эту информацию
 * Отправляет сообщение в канал user_status_{user_id} о том что человек online 
 */
void useritem::setOnline_time(thread_data* local_buf)
{
    last_online_time = 0;
    if(user_id > 0)
    {
        if(appConf::instance()->get_bool("main", "save_users_last_online_time") && !local_buf->isWSClusterActive())
        {
            // В не кластерной работы точно нет необходимости в этом запросе
            local_buf->db.query_format("replace into `users_time` (`user_id`, `time`) VALUES ('%d', 0)", user_id);
        }

        if(appConf::instance()->get_bool("main", "send_user_online_events"))
        {
            char pipe_name[100];
            snprintf(pipe_name, 100, "user_status_%d", user_id);
            internalApi::send_event_to_pipe(local_buf, pipe_name, "{\\\"data\\\":\\\"online\\\",\\\"event_name\\\":\\\"online\\\"}", NULL);


            if(local_buf->isWSClusterActive())
            {
                auto it = local_buf->wsCluster.begin();
                while(it != local_buf->wsCluster.end())
                {
                    auto link = *it; 
                    link->query_format("cometqlcluster_v1; INSERT INTO pipes_messages (name, event, message)VALUES('%s', 'online', 'online');", pipe_name);
                    it++;
                }
            }
        }
    }
}


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


