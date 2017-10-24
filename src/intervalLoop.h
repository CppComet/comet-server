// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

 
#ifndef INTERVALLOOP_H
#define	INTERVALLOOP_H

#include <list>     // подключаем заголовок списка 
#include <functional>

class thread_data;
class intervalLoop;
class intervalLoopObject;


class intervalLoop
{ 
    static intervalLoop* inst;  
    std::list<std::function<void (int, thread_data*)>> functions; 
    std::list<intervalLoopObject*> objects;
    
    pthread_mutex_t mutex;
    int time = 0;
    
public:
    static intervalLoop* instance();
    
    intervalLoop()
    { 
        pthread_mutex_init(&mutex,NULL);
    }
    
    void start();
    
    /**
     * Добавление лямда функции к списку задачь
     * @param routine
     */
    void add(std::function<void (int, thread_data*)> routine);
    
    /**
     * Добавление класса наследника от intervalLoopObject к списку задачь
     * @param routine
     */
    void add(intervalLoopObject* obj);
     
protected:
    static void*loop(void* iLoop); 
    void step(thread_data* local_buf);
};
 
class intervalLoopObject
{ 
    friend class intervalLoop;
protected:
    /** 
     * @param time количество итераций примерно по одной секунде прошедшее с момента запуска сервера 
     */
    void virtual step(int time, thread_data* local_buf);
};
#endif	/* INTERVALLOOP_H */