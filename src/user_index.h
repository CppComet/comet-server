// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * File:   user_index.h
 * Author: victor
 *
 * Created on 18 Ноябрь 2012 г., 11:59
 */

#ifndef USER_INDEX_H
#define	USER_INDEX_H


#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <time.h>
#include <pthread.h>


#include "hashlib2plus/src/hashlibpp.h"
#include <map>

#include "backtrace.h" 
#include "thread_data.h"
#include "internalApi.h"
#include "base64.h"

#define  useritem_debug_info 0


/**
 * Клас пользователя
 * Содержит информацию о одном пользователе.
 */
class useritem{

    /**
     * @todo сделать через умный указатель или возвращать копию в функции get так как оригинальный объект может быть удалён в другом потоке
     */
    char* hash = NULL; // пометить как voliate или mutable

    int conection_ids[MAX_CONECTION_ON_USER_ID+1];
    time_t last_online_time = -1;

private:

    /**
     * Запрещаем копирование для объектов данного класса
     */
    useritem(const useritem& cpy) = delete;

    /**
     * Запрещаем копирование для объектов данного класса
     */
    void operator=( const useritem& cpy) = delete;

public:

    int user_id = 0;


    useritem()
    {
        bzero(conection_ids, sizeof(int) * MAX_CONECTION_ON_USER_ID);
    }

    useritem(thread_data* local_buf, unsigned int User_id,int Conection_id)
    {
        user_id = User_id;
        bzero(conection_ids, sizeof(int) * MAX_CONECTION_ON_USER_ID);
        setConection_id(local_buf, Conection_id);
    }

    useritem(thread_data* local_buf, unsigned int User_id,const char* Hash)
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

        local_buf->stm.users_auth_replace.execute(user_id, hash);

        bzero(conection_ids, sizeof(int) * MAX_CONECTION_ON_USER_ID);
    }

    /**
     * Добавляет id нового соединения к пользователю.
     * Если этот пользователь до этого не имел не одного соединения то вызовет ему setOnline_time()
     * @param local_buf
     * @param Conection_id
     * @return
     */
    int setConection_id(thread_data* local_buf, int Conection_id)
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
    int* getConection_ids()
    {
        return conection_ids;
    }

    /**
     * Отключает конкретный Conection_id для user_id, если это последниее соединение пользователя вызовет setOffline_time() для него
     * @param Conection_id
     * @return
     */
    int unsetConection_id(thread_data* local_buf, int Conection_id)
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
    long getLast_online_time(thread_data* local_buf)
    {
        if(last_online_time == -1)
        {
            local_buf->stm.users_time_select.execute(user_id); 
            if(local_buf->stm.users_time_select.fetch())
            {
                local_buf->stm.users_time_select.free();
                return -1;
            }

            last_online_time = local_buf->stm.users_time_select.result_time;
            local_buf->stm.users_time_select.free(); 
        }

        return last_online_time;
    }

    /**
     * Возвращает имеющиеся значение
     */
    int getLast_online_time()
    {
        return last_online_time;
    }

    /**
     * Определяет время последней авторизации, берёт значение из редиса.
     * @param local_buf
     * @param user_id
     * @return
     */
    long static getLast_online_time(thread_data* local_buf, unsigned int user_id)
    {
        if(!local_buf->stm.users_time_select.execute(user_id))
        {
            return -1;
        }
        
        if(local_buf->stm.users_time_select.fetch())
        {
            local_buf->stm.users_time_select.free();
            return -1;
        }

        long last_online_time = local_buf->stm.users_time_select.result_time;
        local_buf->stm.users_time_select.free(); 
         
        return last_online_time;
    }

    /**
     * Устанавливает хеш авторизации
     */
    bool inline setHash(thread_data* local_buf, const char* Hash)
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
        local_buf->stm.users_auth_replace.execute(user_id, hash);
        return true;
    }


    /**
     * Удаляет хеш авторизации
     */
    bool inline deleteHash(thread_data* local_buf)
    {
        local_buf->stm.users_auth_delete.execute(user_id);
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
    static bool getHash(thread_data* local_buf, unsigned int user_id, char* out_hash)
    {
        local_buf->stm.users_auth_select.execute(user_id);
        
        if(local_buf->stm.users_auth_select.fetch())
        { 
            TagLoger::log(Log_UserItem, 0, "[4]Hash, user_id=%d not found\n", user_id);
            local_buf->stm.users_auth_select.free();
            return false;
        }

        if(out_hash != NULL)
        {
            bzero(out_hash, USER_HASH_LEN);
            memcpy(out_hash, local_buf->stm.users_auth_select.result_hash, USER_HASH_LEN);
        }
        TagLoger::log(Log_UserItem, 0, "[5]Hash user_id=%d is %s\n", user_id, local_buf->stm.users_auth_select.result_hash);
        local_buf->stm.users_auth_select.free();
        return true;
    }

    static bool deleteHash(thread_data* local_buf, unsigned int user_id)
    {
        local_buf->stm.users_auth_delete.execute(user_id);
        return true;
    }

    /**
     * Вернёт ссылку на строку с хешем авторизации или NULL если данных нет.
     *
     * Данные берёт из редиса или если есть из памяти переменной hash
     * @param local_buf
     * @return 
     */
    bool getHash(thread_data* local_buf, char* out_hash)
    {
        if(hash == 0)
        {
            local_buf->stm.users_auth_select.execute(user_id); 
            if(local_buf->stm.users_auth_select.fetch())
            { 
                local_buf->stm.users_auth_select.free();
                TagLoger::log(Log_UserItem, 0, "[3]Hash user_id=%d not found\n", user_id);
                return false;
            }

            hash = new char[USER_HASH_LEN+1];
            bzero(hash, USER_HASH_LEN+1);

            memcpy(hash, local_buf->stm.users_auth_select.result_hash, USER_HASH_LEN );
            local_buf->stm.users_auth_select.free();
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
     *
     * @param local_buf
     * @param token строка из 32 символов
     * @return
     * http://redmine.comet-server.com/issues/65
     * http://hashlib2plus.sourceforge.net/example.html
     *
     * Hash вида
     * token = salt + MD5(salt + dev_key + user_id )
     * salt - всегда 6 символов
     */
    bool static testToken(thread_data* local_buf, const char* token, int user_id);


    /**
     * Проверяет хеш авторизации
     * @param local_buf
     * @param Hash
     * @return
     */
    bool inline testHash(thread_data* local_buf, const char* Hash)
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
    bool static testHash(thread_data* local_buf, const char* Hash, unsigned int user_id)
    {
        // Если в друг нам пришёл не хеш авторизации а токен то проверим его как токен
        if(testToken(local_buf, Hash, user_id))
        {
            // Если токен верен но хеша авторизации нет то установим значение токена в качестве хеша авторизации
            if(!getHash(local_buf, user_id, NULL))
            {
                local_buf->stm.users_auth_replace.execute(user_id, Hash);
            }
            return true;
        }
 
        local_buf->stm.users_auth_select.execute(user_id);
        if(local_buf->stm.users_auth_select.fetch())
        { 
            local_buf->stm.users_auth_select.free();
            return false;
        }

        if(memcmp(Hash, local_buf->stm.users_auth_select.result_hash, USER_HASH_LEN) == 0)
        {
            local_buf->stm.users_auth_select.free();
            return true;
        }

        local_buf->stm.users_auth_select.free();
        return false;
    }

    ~useritem()
    {
        if(hash == 0)
        {
            delete[] hash;
        }
    }

protected:

    /**
     * Запоминает время ухода пользователя и сохраняет эту информацию в редис
     * Отправляет сообщение в канал user_status_{user_id} о том что человек offline
     */
    void setOffline_time(thread_data* local_buf)
    {
        last_online_time = time(0); 
        if(user_id > 0)
        {
            local_buf->db.query_format("replace into `users_time` (`user_id`, `time`) VALUES ('%d', '%d')", user_id, last_online_time); 
            char pipe_name[100];
            snprintf(pipe_name, 100,"user_status_%d", user_id);
            internalApi::send_event_to_pipe(local_buf, pipe_name, "{\\\"data\\\":\\\"offline\\\",\\\"event_name\\\":\\\"offline\\\"}", NULL);
        }
    }

    /**
     * Устанавливает пользователю статус online и сохраняет эту информацию в редис
     * Отправляет сообщение в канал user_status_{user_id} о том что человек online
     */
    void setOnline_time(thread_data* local_buf)
    {
        last_online_time = 0;
        if(user_id > 0)
        {
            local_buf->db.query_format("replace into `users_time` (`user_id`, `time`) VALUES ('%d', 0)", user_id);  // Подусать может этот запрос не нужен 
            char pipe_name[100];
            snprintf(pipe_name, 100, "user_status_%d", user_id);
            internalApi::send_event_to_pipe(local_buf, pipe_name, "{\\\"data\\\":\\\"online\\\",\\\"event_name\\\":\\\"online\\\"}", NULL);
        }
    }


};


/**
 * Класс для хранения информации о пользователях и работы с пользователями
 */
class user_index
{
    std::map<int,useritem*>* map_index;

    static time_t start_time;

    /**
     * Чем больше Pool_index_size тем меньше вероятность того что один поток будет ожидать завершения выполнения другого потока в этой секции.
     */
    int map_index_size = 51;
    pthread_mutex_t* request_mutex;

    /**
     *  @todo Заменить std::map на std::unordered_map так как он быстрее http://forum.vingrad.ru/topic-341520.html
     */
    inline std::map<int,useritem*>& getMapToUserId(unsigned int user_id) const
    {
        TagLoger::log(Log_UserIndex, 0, "[getMap:%d=%d]",user_id, user_id%map_index_size);
        return map_index[user_id%map_index_size];
    }

    /**
     * Гдето здесь можно хранить информацию о потреблённом пользователем количестве ресурсов.
     *
     * Хешей авторизации до 50 000 в руки, и больше если по договорённости.
     * Общее количество сообщений в логах каналов и общеее количество сообщений в очереди пользователя: от 6300 до 63000 для тарифа EVO-8-SSD
     *
     * Сообщения в секунду, количество человек online
     */

private:

    /**
     * Запрещаем копирование для объектов донного класса
     */
    user_index(const user_index& ) = delete;

    /**
     * Запрещаем копирование для объектов донного класса
     */
    void operator=( const user_index& ) = delete;

public:

    user_index()
    {
        try
        {
            map_index = new std::map<int,useritem*>[map_index_size+1];
        }
        catch (std::bad_alloc& ba)
        {
          TagLoger::log(Log_UserIndex, 0, "\x1b[31mbad_alloc caught\x1b[0m\n" );
          print_backtrace(ba.what());
        }

        request_mutex = new pthread_mutex_t[map_index_size+1];

        for(int i=0; i<map_index_size+1; i++ )
        {
            pthread_mutex_init(&request_mutex[i],NULL);
        }

    }

    ~user_index()
    {
        for(int i=0; i<map_index_size+1; i++ )
        {
            pthread_mutex_destroy(&request_mutex[i]);
        }

        delete[] request_mutex;

        for(int i=0; i<map_index_size+1; i++ )
        {
            std::map<int,useritem*>::const_iterator it = map_index[i].begin();
            while(it != map_index[i].end())
            {
                delete it->second;
            }

            map_index[i].clear();
        }

        delete[] map_index;
    }

    /**
     * Возвращает указатель на массив соединений по user_id или 0 если этот user_id выходит за рамки индекса или соединение не активно
     * @param id
     * @return
     */
    int* get_conection_id(thread_data* local_buf, unsigned int user_id)
    {
        pthread_mutex_lock(&request_mutex[user_id%map_index_size]);
        std::map<int,useritem*>::const_iterator it = getMapToUserId(user_id).find(user_id);

        int* cid = NULL;
        if( it != getMapToUserId(user_id).end() )
        {
            cid = it->second->getConection_ids();
        }

        pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);
        return cid;
    }

    /**
     * Вернёт время нахождения пользователя офлайн или uptime сервера если данных нет.
     * @param local_buf
     * @param user_id
     * @return Вернёт время нахождения пользователя офлайн или -1 если данных нет.
     */
    long get_last_online_time(thread_data* local_buf, unsigned int user_id)
    {
        long l = -1;
        pthread_mutex_lock(&request_mutex[user_id%map_index_size]);

        std::map<int,useritem*>::const_iterator it = getMapToUserId(user_id).find(user_id);
        if( it != getMapToUserId(user_id).end() )
        {
            l = it->second->getLast_online_time(local_buf);
        }
        else
        {
            l = useritem::getLast_online_time(local_buf, user_id);
            if( l == 0 )
            {
                l = -1;
            }
        }

        pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);
        return l;/**/
    }

    /**
     * Добавляет хеш авторизации для пользователя
     * @param local_buf
     * @param user_id пользователь
     * @param hash ключ авторизации
     * @return
     */
    bool add_hash(thread_data* local_buf, unsigned int user_id,const char* hash)
    {
        TagLoger::log(Log_UserIndex, 0, "add_hash %d->%s\n",user_id,hash);
        pthread_mutex_lock(&request_mutex[user_id%map_index_size]);
        std::map<int,useritem*>::const_iterator it = getMapToUserId(user_id).find(user_id);
        if( it != getMapToUserId(user_id).end() )
        {
            it->second->setHash(local_buf, hash);
        }
        else
        {
            /**
             * безопасность параллельного чтения. Несколько потоков могут одновременно читать содержимое контейнера, и это не помешает его правильной работе. Естественно, запись в контейнер при этом не допускается;
             * безопасность записи в разные контейнеры. Несколько потоков могут одновременно производить запись в разные контейнеры.
             */
            if(getMapToUserId(user_id).size() > MAX_MAP_SIZE )// Удалить тех кто офлайн когда в Map их много наберётся @todo Надо cделать более оптимально удаление (в отдельном потоке или както ещё)
            {
                // @todo Заменить std::map на std::unordered_map так как он быстрее http://forum.vingrad.ru/topic-341520.html
                std::map<int,useritem*>::const_iterator it = getMapToUserId(user_id).begin();
                while( it != getMapToUserId(user_id).end() )
                {
                    if(it->second->getLast_online_time() != 0) // Удалить тех кто офлайн
                    {
                        it = getMapToUserId(user_id).erase(it);
                    }
                    else
                    {
                        it++;
                    }
                }
            }
            getMapToUserId(user_id).insert(std::pair<int,useritem*>(user_id,new useritem(local_buf, user_id, hash) ));
        }

        pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);
        return true;
    }


    /**
     * Возвращает хеш авторизации для пользователя
     * @param local_buf
     * @param user_id пользователь
     * @return hash ключ авторизации
     * @Note Возможно этот вызов можно оптимизировать.
     *
     * @deprecated В замен таблицы хешей пользователей решено переходить на алгоритм авторизации по токену
     */
    bool get_hash(thread_data* local_buf, unsigned int user_id, char* out_hash)
    {
        bool isResult = false;
        TagLoger::log(Log_UserIndex, 0, "get_hash %d",user_id);
        pthread_mutex_lock(&request_mutex[user_id%map_index_size]);
        std::map<int,useritem*>::const_iterator it = getMapToUserId(user_id).find(user_id);
        if( it != getMapToUserId(user_id).end() )
        {
            isResult = it->second->getHash(local_buf, out_hash);
            pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);
        }
        else
        {
            pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);
            isResult = useritem::getHash(local_buf, user_id, out_hash);
        }

        return isResult;
    }

    bool delete_hash(thread_data* local_buf, unsigned int user_id)
    {
        bool result = false;
        TagLoger::log(Log_UserIndex, 0, "delete_hash %d", user_id);
        pthread_mutex_lock(&request_mutex[user_id%map_index_size]);
        std::map<int,useritem*>::const_iterator it = getMapToUserId(user_id).find(user_id);
        if( it != getMapToUserId(user_id).end() )
        {
            result = it->second->deleteHash(local_buf);
            pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);
        }
        else
        {
            pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);
            result = useritem::deleteHash(local_buf, user_id);
        }

        return result;
    }

    /**
     * Проверяет соответсвие хеша идентификации у user_id с педеданым
     * @param id Идентификатор пользователя (user_id)
     * @param hash Хеш идентификации
     * @return Если хеши иденитфикации совпадают то вернёт true
     */
    bool get_link(thread_data* local_buf, unsigned int user_id,const char* hash)
    {
        if(user_id == 0)
        {
            return false;
        }

        pthread_mutex_lock(&request_mutex[user_id%map_index_size]);
        std::map<int,useritem*>::const_iterator it = getMapToUserId(user_id).find(user_id);
        if( it != getMapToUserId(user_id).end() )
        {
            pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);
            if( it->second->testHash(local_buf, hash) )
            {
                TagLoger::log(Log_UserIndex, 0, "\x1b[34m[get_link]User's hashes of %d match\x1b[0m\n",user_id);
                return true;
            }

            TagLoger::log(Log_UserIndex, 0, "\x1b[35m[get_link]User's hashes of %d not match\x1b[0m\n",user_id);
            return false;
        }
        else
        {
            pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);

            if(useritem::testHash(local_buf, hash,  user_id))
            {
                TagLoger::log(Log_UserIndex, 0, "\x1b[34m[get_link][static]User's hashes of %d match\x1b[0m\n",user_id);
                return true;
            }

            TagLoger::log(Log_UserIndex, 0, "\x1b[35m[get_link][static]User's hashes of %d not match\x1b[0m\n",user_id);
            return false;
        }
    }

    /**
     * Устанавливает соответствие между иденитификатором соединения и идентификатором пользователя
     * @param id Идентификатор пользователя (user_id)
     */
    bool set_link(thread_data* local_buf, unsigned int user_id,int conection_id)
    {
        if(user_id == 0)
        {
            return false;
        }

        pthread_mutex_lock(&request_mutex[user_id%map_index_size]);

        std::map<int,useritem*>::const_iterator it = getMapToUserId(user_id).find(user_id);
        if( it != getMapToUserId(user_id).end() )
        {
            it->second->user_id = user_id;
            it->second->setConection_id(local_buf, conection_id);

            TagLoger::log(Log_UserIndex, 0, "\x1b[34m[set_link]User %d is authorized on the connection %d\x1b[0m\n",user_id,conection_id);
        }
        else
        {
            getMapToUserId(user_id).insert(std::pair<int,useritem*>(user_id,new useritem(local_buf, user_id,conection_id)));
            TagLoger::log(Log_UserIndex, 0, "\x1b[34m[set_link]User %d is authorized on the connection %d\x1b[0m\n",user_id,conection_id);
        }

        pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);
        return true;
    }

    /**
     * Удаляет соответствие между иденитификатором соединения и идентификатором пользователя
     */
    int un_link(thread_data* local_buf, unsigned int user_id, int conection_id)
    {
        if(user_id == 0)
        {
            return false;
        }

        pthread_mutex_lock(&request_mutex[user_id%map_index_size]);
        std::map<int,useritem*>::const_iterator it = getMapToUserId(user_id).find(user_id);
        if( it != getMapToUserId(user_id).end() )
        {
            TagLoger::log(Log_UserIndex, 0, "\x1b[34m[un_link]User %d is disconnected from the connection %d\x1b[0m\n",user_id, conection_id);
            TagLoger::log(Log_UserIndex, 0, "\x1b[34m[un_link]User %d has updated the value of last_online_time\x1b[0m\n",user_id);

            it->second->user_id = user_id;
            it->second->unsetConection_id(local_buf, conection_id);

            pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);
            return 0;
        }

        pthread_mutex_unlock(&request_mutex[user_id%map_index_size]);
        return -1;
    }
};
#endif	/* USER_INDEX_H */
