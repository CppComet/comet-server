// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef USER_INDEX_CPP
#define	USER_INDEX_CPP

#include "user_index.h"

time_t user_index::start_time=time(0);


#include "devManager.h"

  
#include "jwt/jwt_all.h"
#include "mystring.h"
using json = nlohmann::json;

useritem::useritem()
{
    bzero(conection_ids, sizeof(int) * MAX_CONECTION_ON_USER_ID);
}

useritem::useritem(thread_data* local_buf, int Dev_Id, unsigned int User_id,int Conection_id)
{
    dev_id = Dev_Id;
    user_id = User_id;
    bzero(conection_ids, sizeof(int) * MAX_CONECTION_ON_USER_ID);
    setConection_id(local_buf, Conection_id);
}

useritem::useritem(thread_data* local_buf, int Dev_Id, unsigned int User_id,std::string Hash)
{
    dev_id = Dev_Id;
    user_id = User_id;
 
    hash = Hash;

    TagLoger::log(Log_UserItem, 0, "auth_hash_test:%s\n", hash.data());

    local_buf->stm.users_auth_replace->execute(dev_id, user_id, hash.data());

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

void useritem::setDevId(int Dev_id)
{
    dev_id = Dev_id;
}

int useritem::getDevId(void)
{
    return dev_id;
}

/**
 * Возвращает имеющиеся значение, но если данных нет то обратится в редис.
 */
long useritem::getLast_online_time(thread_data* local_buf)
{
    if(last_online_time == -1)
    {
        local_buf->stm.users_time_select->execute(dev_id, user_id); 
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
 * @param dev_id
 * @return
 */
long useritem::getLast_online_time(thread_data* local_buf, unsigned int user_id, int dev_id)
{
    if(!local_buf->stm.users_time_select->execute(dev_id, user_id))
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
bool useritem::setHash(thread_data* local_buf, std::string Hash)
{
    hash = Hash;
 
    TagLoger::log(Log_UserItem, 0, "auth_hash_test:%s\n", hash.data());
    local_buf->stm.users_auth_replace->execute(dev_id, user_id, hash.data());
    return true;
}


/**
 * Удаляет хеш авторизации
 */
bool useritem::deleteHash(thread_data* local_buf)
{
    local_buf->stm.users_auth_delete->execute(dev_id, user_id);
    if(hash.empty())
    {
        return true;
    }

    hash.erase();
    return true;
}

/**
 * Вернёт ссылку на строку с хешем авторизации или NULL если данных нет.
 *
 * Данные берёт из редиса
 * @param local_buf
 * @return
 *
 */
bool useritem::getHash(thread_data* local_buf, unsigned int user_id, int dev_id, char* out_hash)
{
    local_buf->stm.users_auth_select->execute(dev_id, user_id);

    if(local_buf->stm.users_auth_select->fetch())
    {
        TagLoger::log(Log_UserItem, 0, "[4]Hash dev_id=%d, user_id=%d not found\n", dev_id, user_id);
        local_buf->stm.users_auth_select->free();
        return false;
    }

    if(out_hash != NULL)
    {
        bzero(out_hash, USER_HASH_LEN);
        memcpy(out_hash, local_buf->stm.users_auth_select->result_hash, USER_HASH_LEN);
    }
    
    TagLoger::log(Log_UserItem, 0, "[5]Hash dev_id=%d, user_id=%d is %s\n", dev_id, user_id, local_buf->stm.users_auth_select->result_hash);
    local_buf->stm.users_auth_select->free();
    return true;
}

bool useritem::deleteHash(thread_data* local_buf, unsigned int user_id, int dev_id)
{
    local_buf->stm.users_auth_delete->execute(dev_id, user_id);
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
    if(hash.empty())
    {
        local_buf->stm.users_auth_select->execute(dev_id, user_id); 
        if(local_buf->stm.users_auth_select->fetch())
        {
            local_buf->stm.users_auth_select->free();
            TagLoger::log(Log_UserItem, 0, "[3]Hash dev_id=%d, user_id=%d not found\n", dev_id, user_id);
            return false;
        }

        hash = local_buf->stm.users_auth_select->result_hash;
 
        local_buf->stm.users_auth_select->free();
        TagLoger::log(Log_UserItem, 0, "[1]Hash dev_id=%d, user_id=%d is %s\n", dev_id, user_id, hash.data());
    }
    else
    {
        TagLoger::log(Log_UserItem, 0, "[2]Hash dev_id=%d, user_id=%d is %s\n", dev_id, user_id, hash.data());
    }

    if(out_hash != NULL)
    {
        bzero(out_hash, USER_HASH_LEN);
        memcpy(out_hash, hash.data(), USER_HASH_LEN);
    }
    return true;
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
 * token = salt + MD5(salt + dev_key + dev_id + user_id )
 * salt - всегда 10 символов 
 * 
 * 
 * 
 * JWT
 * 
 * 
 * 
 */
bool useritem::testToken(thread_data* local_buf, std::string token, int dev_id, int user_id)
{  
    ExpValidator exp;
     
    std::string secret(appConf::instance()->get_chars("main", "password"));

    secret.append(std::to_string(dev_id));
    HS256Validator signer(secret);
    
    // https://jwt.io/
    // Now let's use these validators to parse and verify the token we created
    // in the previous example
  
    try {
        // Decode and validate the token
        ::json header, payload;

        std::tie(header, payload) = JWT::Decode(token, &signer, &exp);
        //std::cout << "Header: " << header << std::endl;
        //std::cout << "Payload: " << payload << std::endl;
        
        if(payload["user_id"] != user_id)
        { 
            TagLoger::debug(Log_UserItem, 0, "Validation failed user_id error:jwt-user_id=%d, user_id=%d\n", (int)payload["user_id"], user_id);
            return false;
        }
         
        local_buf->stm.revoked_tokens_select->execute(dev_id, token.data());
        if(local_buf->stm.revoked_tokens_select->fetch())
        { 
            // Токен не найден среди отозванных
            TagLoger::debug(Log_UserItem, 0, "Validation ok:[secret=%s, token=%s]\n", secret.data(), token.data());
            local_buf->stm.revoked_tokens_select->free();
            return true;
        }

        TagLoger::debug(Log_UserItem, 0, "Validation failed revoked_tokens:[secret=%s, token=%s]\n", secret.data(), token.data());
        local_buf->stm.revoked_tokens_select->free(); 
        return false;
        
    } catch (InvalidTokenError &tfe) {
        // An invalid token 
        TagLoger::debug(Log_UserItem, 0, "Validation failed: %s [secret=%s, token=%s]\n", tfe.what(), secret.data(), token.data());
    }
    catch(...)
    { 
        // handling for exceptions with any type
        TagLoger::debug(Log_UserItem, 0, "Validation failed: secret=%s, token=%s\n", secret.data(), token.data());
    }
  
    return false;
}
  
bool useritem::testHash(thread_data* local_buf, std::string Hash)
{
    // Если в друг нам пришёл не хеш авторизации а токен то проверим его как токен
    if(testToken(local_buf, Hash, dev_id, user_id))
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
 
    TagLoger::log(Log_UserItem, 0, "testHash: %s = %s\n", hash.data(), Hash.data());
   
    return Hash == hash;
}

/**
 * Проверяет хеш авторизации
 * @param local_buf
 * @param Hash
 * @param user_id
 * @param dev_id 
 * @return
 */
bool useritem::testHash(thread_data* local_buf, std::string Hash, unsigned int user_id, int dev_id)
{
    // Если в друг нам пришёл не хеш авторизации а токен то проверим его как токен
    if(testToken(local_buf, Hash, dev_id, user_id))
    {
        // Если токен верен но хеша авторизации нет то установим значение токена в качестве хеша авторизации
        if(!getHash(local_buf, user_id, dev_id, NULL))
        {
            char uuidHash[38];
            bzero(uuidHash, 38);
            uuid37(uuidHash);
            for(int i = 0; i<38; i++)
            {
                if(uuidHash[i] == '-')
                {
                    uuidHash[i] = '0';
                }
            }
            
            local_buf->stm.users_auth_replace->execute(dev_id, user_id, uuidHash);
        }
        return true;
    }

    local_buf->stm.users_auth_select->execute(dev_id, user_id);
    if(local_buf->stm.users_auth_select->fetch())
    {
        local_buf->stm.users_auth_select->free();
        return false;
    }

    if(Hash == local_buf->stm.users_auth_select->result_hash)
    {
        local_buf->stm.users_auth_select->free();
        return true;
    }


    local_buf->stm.users_auth_select->free();
    return false;
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
            local_buf->db.query_format("replace into `users_time` (`dev_id`, `user_id`, `time`) VALUES ('%d', '%d', '%d')", dev_id, user_id, last_online_time); 
        }
        if(appConf::instance()->get_bool("main", "send_user_offline_events"))
        {
            char pipe_name[100];
            snprintf(pipe_name, 100,"user_status_%d", user_id);
            internalApi::send_event_to_pipe(local_buf, pipe_name, "{\\\"data\\\":\\\"offline\\\",\\\"event_name\\\":\\\"offline\\\"}", dev_id, NULL);

            if(local_buf->isWSClusterActive())
            {
                auto it = local_buf->wsCluster.begin();
                while(it != local_buf->wsCluster.end())
                {
                    auto link = *it; 
                    link->query_format("cometqlcluster_v1 set dev_id=%d; INSERT INTO pipes_messages (name, event, message)VALUES('%s', 'offline', 'offline');", dev_id, pipe_name);
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
            local_buf->db.query_format("replace into `users_time` (`dev_id`, `user_id`, `time`) VALUES ('%d', '%d', 0)", dev_id, user_id); 
        }

        if(appConf::instance()->get_bool("main", "send_user_online_events"))
        {
            char pipe_name[100];
            snprintf(pipe_name, 100, "user_status_%d", user_id);
            internalApi::send_event_to_pipe(local_buf, pipe_name, "{\\\"data\\\":\\\"online\\\",\\\"event_name\\\":\\\"online\\\"}", dev_id, NULL);


            if(local_buf->isWSClusterActive())
            {
                auto it = local_buf->wsCluster.begin();
                while(it != local_buf->wsCluster.end())
                {
                    auto link = *it; 
                    link->query_format("cometqlcluster_v1 set dev_id=%d; INSERT INTO pipes_messages (name, event, message)VALUES('%s', 'online', 'online');", dev_id, pipe_name);
                    it++;
                }
            }
        }
    }
}




#endif	/* USER_INDEX_CPP */


