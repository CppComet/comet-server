// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com



#include <iterator> // заголовок итераторов
#include "intervalLoop.h"

#include "thread_data.h"
 
intervalLoop* intervalLoop::inst;

intervalLoop* intervalLoop::instance()
{
   if (intervalLoop::inst == 0)
   {
         intervalLoop::inst = new intervalLoop();
   }
   return intervalLoop::inst;
}


void intervalLoop::start()
{
    // Запуск потока обработки
    pthread_t c_sthreads; 
    pthread_create(&c_sthreads, NULL, loop, (void *)this);
}

void intervalLoop::add(std::function<void (int, thread_data*)> routine)
{
    pthread_mutex_lock(&mutex);
    functions.push_back(routine);
    pthread_mutex_unlock(&mutex);
}

void intervalLoop::add(intervalLoopObject* obj)
{
    pthread_mutex_lock(&mutex);
    objects.push_back(obj);
    pthread_mutex_unlock(&mutex);
}
    

void intervalLoop::step(thread_data* local_buf)
{
    pthread_mutex_lock(&mutex);
    time++;
    auto it = functions.begin();
    while( it != functions.end() )
    {
        auto routine = *it;
        routine(time, local_buf);
        it++;
    }

    auto itObj = objects.begin();
    while( itObj != objects.end() )
    {
        intervalLoopObject* obj = *itObj;
        obj->step(time, local_buf);
        itObj++;
    }
    
    pthread_mutex_unlock(&mutex);
}

void* intervalLoop::loop(void *iLoop)
{
    intervalLoop* iloop = (intervalLoop*)iLoop;
    thread_data* local_buf = new thread_data(appConf::instance());
    while(1)
    {
        iloop->step(local_buf);
        sleep(1);
    }
    return NULL;
}

void intervalLoopObject::step(int time, thread_data* local_buf)
{
    
}