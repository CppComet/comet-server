// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef DEVMANAGER_H
#define	DEVMANAGER_H

#include <list>     // подключаем заголовок списка
#include <iterator> // заголовок итераторов
#include <algorithm>

#include "user_index.h"


class devManager;

template< class dataType >
class fastList;

template<class dataType >
class fastListItem: public CpClass
{
    friend class fastList<dataType>;
protected:
    CP<fastListItem<dataType> > next;
public:
    dataType data;

    fastListItem():next(NULL){
    }

    fastListItem(dataType d):data(d), next(NULL){

    }

    bool hasNext() const {
        return !next.isNULL();
    }

    CP<fastListItem<dataType> > Next()  {
        return next;
    }
};

template< class dataType >
class fastList
{
    pthread_mutex_t mutex;
    int list_size = 0;
    CP<fastListItem<dataType> > first;
public:

    fastList():first(NULL)
    {
        pthread_mutex_init(&mutex,NULL);
    }
    
    CP<fastListItem<dataType> > begin()
    {
        return first;
    } 
     
    bool empty() const
    {
        return first.isNULL();
    }

    int size() const
    {
       return list_size;
    }

    void append(dataType data)
    {
        CP<fastListItem<dataType> > item;
        item->data = data;
         
        pthread_mutex_lock(&mutex);
        if(!first.isNULL())
        {
            item->next = first;
        }
        first = item;
        pthread_mutex_unlock(&mutex);
        
        list_size++;
    }
 
    bool remove(dataType data)
    {
        pthread_mutex_lock(&mutex);
        auto item = first;
        CP<fastListItem<dataType> > lastItem;
        bool useLastItem = false;

        while(item)
        {
            if(item->data == data)
            { 
                if(useLastItem)
                {
                    lastItem->next = item->next;
                }
                else
                {
                    first = first->next;
                }
                pthread_mutex_unlock(&mutex);
                list_size--;
                return true;
            }

            lastItem = item;
            useLastItem = true;
            item = item->next;
        }

        pthread_mutex_unlock(&mutex);
        return false;
    }
};



/**
template< class keyType, class dataType >
class fastListMap;

template<class keyType, class dataType >
class fastListMapItem: public CpClass
{
    friend class fastListMap<keyType, dataType>;
protected:
    CP<fastListMapItem<keyType, dataType> > next = NULL;
public:
    dataType data;
    keyType key;

    fastListMapItem(keyType k, dataType d):key(k), data(d){

    }

    bool hasNext() const {
        return next != NULL;
    }

    CP<fastListMapItem<keyType, dataType> > getNext() const {
        return next;
    }
};

template< class keyType, class dataType >
class fastListMap
{
    //pthread_mutex_t mutex;
    CP<fastListMapItem<keyType, dataType> > invalid;
public:
    CP<fastListMapItem<keyType, dataType> > first;

    fastListMap()
    {
        //pthread_mutex_init(&mutex,NULL);
    }

    void append(keyType key, dataType data)
    {
        auto item = CP<fastListMapItem<keyType, dataType> >(new fastListMapItem<keyType, dataType>(key, data));
        if(first != NULL)
        {
            item->next = first->next;
        }
        first = item;
    }

    void remove(keyType key)
    {
        //pthread_mutex_lock(&mutex);
        auto item = first;
        CP<fastListMapItem<keyType, dataType> > lastItem;

        while(item != NULL)
        {
            if(item->key == key)
            {
                if(lastItem != NULL)
                {
                    lastItem->next = item->next;
                }

                //delete item;
                break;
            }

            lastItem = item;
            item = item->next;
        }
        //pthread_mutex_unlock(&mutex);
    }

    CP<fastListMapItem<keyType, dataType> > find(keyType key)
    {
        auto item = first;
        while(item != NULL)
        {
            if(item->key == key)
            {
                return item;
            }

            item = item->next;
        }

        return NULL; // invalid
    }

    bool isValid(CP<fastListMapItem<keyType, dataType> > item)
    {
        return invalid == item;
    }
};*/
/**
 * Класс канала со списком подписчиков
 */
class Pipe: public CpClass
{
public:
    fastList<int> subscribers;

    Pipe():CpClass()
    {

    }

    /**
     * Добавляет подписчика в канал
     */
    void insert(int id)
    {
        subscribers.append(id);
    }

    int size() const
    {
        return subscribers.size();
    }

    bool empty() const
    {
        return subscribers.empty();
    }

    /**
     * Вырежет из списка подписчиков один элемент
     */
    void erase(int id)
    {
        subscribers.remove(id);
    }
};

/**
 * Информация об одеом dev_id
 */
class devInfo
{
    friend class devManager;

        bool active = false;
        char* key = NULL;

        char* url = NULL;

        char** urls = NULL;
        int urls_count = 0;

        int frontend_online = 0;
        int backend_online = 0;
        int messages = 0;

        int frontend_connections = 0;
        int backend_connections = 0;


        pthread_mutex_t pipe_index_mutex;

        std::map<std::string,CP<Pipe>> pipe_index;

        void pipe_index_lock()
        {
            pthread_mutex_lock(&pipe_index_mutex);
        }

        void pipe_index_unloc(){
            pthread_mutex_unlock(&pipe_index_mutex);
        }

    protected:
        devInfo(const char* dev_key);

    public:

        void setDevKey(const char* devKey);
        void setDevUrl(const char* devUrl);

        int countDevUrl() const;
        const char* getDevUrl(int i) const;

        bool isActive() const { return active; }

        /**
         * Работает в соответсвии с алгоритмом MySQL Authentication::Native41
         * http://dev.mysql.com/doc/internals/en/secure-password-authentication.html#packet-Authentication::Native41
         * @param dev_id
         * @param random20bytes
         * @param dev_key
         * @return
         */
        static bool testDevKey(const char* random20bytes, const char* DevKeyHashStart, thread_data* local_buf);

        CP<Pipe> getPipe(const std::string pipe_name)
        {
            CP<Pipe> pipe = NULL;
            pipe_index_lock();
            auto it = pipe_index.find(pipe_name);

            if( it != pipe_index.end() )
            {
                pipe = it->second;
            }
            else
            {
                pipe = new Pipe();
                pipe_index.insert(std::pair<std::string,CP<Pipe>>(pipe_name, pipe));
            }

            pipe_index_unloc();
            return pipe;
        }

        CP<Pipe> findPipe(const std::string pipe_name)
        {
            pipe_index_lock();
            auto it = pipe_index.find(pipe_name);

            if( it != pipe_index.end() )
            {
                CP<Pipe> pipe = it->second;
                pipe_index_unloc();
                return pipe;
            }

            pipe_index_unloc();
            return CP<Pipe>(NULL);
        }

        /**
         * Надо бы вызывать кода в канале не остаётся подписчиков
         * @param pipe_name
         */
        void deletePipe(const std::string pipe_name)
        {
            pipe_index_lock();
            auto it = pipe_index.find(pipe_name);

            if( it != pipe_index.end() )
            {
                pipe_index.erase(it);
            }

            pipe_index_unloc();
        }

        user_index* index;

        ~devInfo();

        const char* getDevKey() const;
        bool testDevKey(const char* devKey) const;

    /**
     * Увеличение колва онлайн (фронтенд)
     */
    void incrFrontendOnline() { frontend_online++;}

    /**
     * Уменьшение колва онлайн (фронтенд)
     */
    void decrFrontendOnline() { frontend_online--;}

    /**
     * Вернёт количество онлайн (фронтенд)
     */
    int getFrontendOnline() const { return frontend_online;}

    /**
     * Увеличение колва онлайн (бекенд)
     */
    void incrBackendOnline() { backend_online++;}

    /**
     * Уменьшение колва онлайн (бекенд)
     */
    void decrBackendOnline() { backend_online--;}

    /**
     * Вернёт количество онлайн (бекенд)
     */
    int getBackendOnline() const { return backend_online;}

    /**
     *  Инкремент количиства отправленных сообщений в Js от dev_id
     */
    void incrMessages();

    /**
     * Вернёт количиство отправленных сообщений в Js от dev_id
     */
    int getMessages() const { return messages;}

    /**
     *  Инкремент количиства удачных конектов (фронтенд)
     */
    void incrFrontendConnections() { frontend_connections++;}
    int getFrontendConnections() const { return frontend_connections;}

    /**
     *  Инкремент количиства удачных конектов (бекенд)
     */
    void incrBackendConnections() { backend_connections++;}
    int getBackendConnections() const { return backend_connections;}
};

/**
 * Класс управления массивом dev_id клиентов (вебмастеров)
 */
class devManager
{ 
    devInfo* index; 
    int tps_network_events = 0;
    float ps_network_events = 0;

    public:
 
    static devManager* inst;
    static devManager* instance();

    ~devManager();
    devManager()
    {
        index = new devInfo(appConf::instance()->root_password); 
        intervalLoop::instance()->add([](int uptime, thread_data* local_buf)
        {
            if( uptime%appConf::instance()->client_benchmark != 0)
            {
                return;
            }
            
            devManager::instance()->ps_network_events = devManager::instance()->tps_network_events / appConf::instance()->client_benchmark;
            devManager::instance()->tps_network_events = 0;
        });

    }  
      
    float getPsNetworkEvents(){ return ps_network_events; }

    /**
     * Для учёта общего количества отправленых и пинятых сообщений на сервере от авторизованных пользователей
     */
    void addNetworkEvents(){ tps_network_events++; }

    /**
     * Вернёт devInfo по dev_id
     * @param dev_id
     * @return
     *
     * Если dev_id не валидный то будет возвращатся index[0]
     */
    devInfo* getDevInfo();

protected:

};


#endif	/* DEVMANAGER_H */