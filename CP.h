// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* 
 * File:   common.h
 * Author: levha
 *
 * Created on 2 Октябрь 2010 г., 20:38
 * Добавлено к этому проекту on 12 Май 2011 г., 1:08
 */

#ifndef CP_H
#define	CP_H
 
#include <stdio.h>
 

/*
 * CP
 * Шаблон умного указателя ведущего подсчёт ссылок
 * Применим ко всем классам имеющим функции Grab() и Release()  и конструктор без параметров
 * Ведёт правильный подсчёт ссылок на объект только если к этому объекту обращаются через указатели этого класса
 */
template <class type>
class CP
{  
  private:
    type* pointer = NULL;

  public:
    CP() : pointer(0)
    {
            pointer = new type();
    }

    CP(type* p) :pointer(p)
    {
        if (pointer!= NULL)
        {
            pointer->Grab();
        }
    }

    CP(const CP<type>& cp) : pointer(cp.pointer)
    {
        /**
         * [CP.h]: (style) Value of pointer 'pointer', which points to allocated memory, is copied in copy constructor instead of allocating new memory.
         * Так и задумано, это паттерн умного указателя.
         */
        if (pointer!= NULL)
        {
            pointer->Grab();
        }
    }
    
    virtual ~CP()
    { 
        if (pointer!= NULL)
        {
            pointer->Release();
        }
    }

    CP<type>& operator=(const CP<type>& cp)
    {
        if (this == &cp){ return *this;}

        if (pointer!= NULL)
        {
            pointer->Release();
        }

        pointer = cp.pointer;
        if (pointer!= NULL)
        {
            pointer->Grab();
        }

        return *this;
    }
 
    
    
    bool isNULL() const {return pointer == NULL; }
    operator bool() {return pointer != NULL; }
    
    operator type*() {return pointer; }
    type* operator->() { return pointer; }

    type* get() {return pointer; } 
};

/**
 * Для подсчёта ссылок
 */
class CpClass
{
    
protected:

  /**
    * Для подсчёта ссылок
    */
   int _cp_type_count;
   
public:
    static const int memdebug=0;
    
   CpClass():_cp_type_count(0)
   {
       //int static ID;
       Grab();
       //if(memdebug) printf("> Create %d\n",_cp_type_id);/**/
   }
   
   virtual ~CpClass()
   {
     //if(memdebug)  printf("{Delete %d}",_cp_type_id); /**/
   }

/**
 * Учёт ссылок, вызывается вместо копирования
 */
   inline void Grab(){_cp_type_count++;}

/**
 * Учёт ссылок, вызывается вместо удаления
 * Если ссылок не осталось объект удаляется.
 */
   virtual void Release()
   {
       _cp_type_count--;
       if(_cp_type_count<1)
       {
           //if(memdebug) printf("> Delete %d\n",_cp_type_id);
           delete this;
       }
       else
       {
           //if(memdebug) printf("> NO delete %d\n",_cp_type_id);
       }
   }

    /**
     * Вернёт количество ссылок
     */
   inline int CountLink(){return _cp_type_count;}

};

#endif	/* CP_H */

