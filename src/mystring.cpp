// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Содержит вспомагательные функции работы со строками
 */
#include "mystring.h"
#include "TagLoger.h"

#include <uuid/uuid.h> // apt-get install uuid-dev

/**
 * Читает целое число из строки, если ошибка то вернёт -1
 * @param buffer Строка
 * @param delimiter Конец для числа
 * @param delta Количество символов занятое числом и разделителем
 * @return
 */
int read_int(const char* buffer,char delimiter,int* delta)
{
    const char* p = buffer;
    int len = 0;
    int d = 0;

    while(*p == '0' )
    {
        (*delta)++;
        p++;
        //return 0;
    }

    while(*p != delimiter)
    {
        if(*p > '9' || *p < '0')
        {
            return -1;
        }

        len = (len*10)+(*p - '0');
        p++;
        (*delta)++;
        d++;

        if(d > 9)
        {
            return -1;
        }
    }
    return len;
}

int read_int(const char* buffer,char delimiter)
{
    const char* p = buffer;
    int len = 0;
    int delta = 0;

    while(*p == '0' )
    {
        p++;
    }

    while(*p != delimiter)
    {
        if(*p > '9' || *p < '0')
        {
            return -1;
        }

        len = (len*10)+(*p - '0');
        p++;
        delta++;
        if(delta > 9)
        {
            return -1;
        }
    }

    return len;
}

int read_int(const char* buffer, int* delta)
{
    const char* p = buffer;
    int len = 0;
    int d = 0;

    while(*p == '0' )
    {
        (*delta)++;
        p++;
        //return 0;
    }

    while(1)
    {
        if(*p > '9' || *p < '0')
        {
            return len;
        }

        len = (len*10)+(*p - '0');
        p++;
        (*delta)++;
        d++;

        if(d > 9)
        {
            return -1;
        }
    }
    return len;
}
/**
 * Читает целое число из строки, если ошибка то вернёт -1
 * @param buffer Строка
 * @param delimiter Конец для числа
 * @param delta Количество символов занятое числом и разделителем
 * @return
 */
long read_long(const char* buffer,char delimiter,int* delta)
{
    const char* p = buffer;
    int len = 0;
    int d = 0;

    while(*p == '0' )
    {
        (*delta)++;
        p++;
        //return 0;
    }

    while(*p != delimiter)
    {
        if(*p > '9' || *p < '0')
        {
            return -1;
        }

        len = (len*10)+(*p - '0');
        p++;
        (*delta)++;
        d++;

        if(d > 18)
        {
            return -1;
        }
    }
    return len;
}

long read_long(const char* buffer,char delimiter)
{
    const char* p = buffer;
    int len = 0;
    int delta = 0;

    while(*p == '0' )
    {
        p++;
    }

    while(*p != delimiter)
    {
        if(*p > '9' || *p < '0')
        {
            return -1;
        }

        len = (len*10)+(*p - '0');
        p++;
        delta++;
        if(delta > 18)
        {
            return -1;
        }
    }

    return len;
}

long read_long(const char* buffer, int* delta)
{
    const char* p = buffer;
    int len = 0;
    int d = 0;

    while(*p == '0' )
    {
        (*delta)++;
        p++;
        //return 0;
    }

    while(1)
    {
        if(*p > '9' || *p < '0')
        {
            return len;
        }

        len = (len*10)+(*p - '0');
        p++;
        (*delta)++;
        d++;

        if(d > 18)
        {
            return -1;
        }
    }
    return len;
}
  
void uuid37(char* src)
{
    uuid_t uuid;
    uuid_generate_time ( uuid );
    uuid_unparse ( uuid, src );
}
  
/**
 *
 * @param s исходня строка
 * @param c символ искомый
 * @param n номер вхождения
 * @param start начальный символ
 * @return позиция в строке
 */
int str_find(const char* s,const char c,int n,int start)
{ 
    return str_find(s, c, n, start, strlen(s));
}
   
/**
 *
 * @param s исходня строка
 * @param c символ искомый
 * @param n номер вхождения
 * @param start начальный символ
 * @return
 */
int str_find(const char* s,const char c,int n,int start, int end)
{
    for(int i=start; i<end; i++)
    {
        if(s[i] == c)
        {
            if(i+2<end)
            {
                if(n>1) return str_find( s,c, n-1,i+1,end);
                return i+1;
            }
            else
            {
                return -1;
            }
            break;
        }
    }
    return -1;
}

/**
 *
 * @param s исходня строка
 * @param c символ искомый
 * @return номер сивола от начала строки или -1
 */
int str_find(const char* s,const char c )
{
    int len = strlen(s);
    for(int i=0; i<len; i++)
    {
        if(s[i] == c)
        {
            return i;
        }
    }
    return -1;
}

/**
 * @param s исходня строка
 * @param c символ искомый
 * @param len для для поиска
 * @return номер сивола от начала строки или -1
 */
int str_find(const char* s,const char c,int len )
{
    for(int i=0; i<len; i++)
    {
        if(s[i] == c)
        {
            return i;
        }
    }
    return -1;
}

/**
 * Проверка символа p на соответсвие следующему регулярному выражению [A-Za-z0-9\-_]
 * @param p
 * @return true в случаии соответсвия
 */
bool AZ09test(char p)
{
    return (p >= '0' && p <= '9') || ( p >= 'A' && p <= 'Z' ) || ( p >= 'a' && p <= 'z' ) || ( p == '-' || p == '_' );
}

bool AZ09test(const char* s, int len )
{
    for(int i=0; i<len; i++)
    {
        if( !AZ09test(s[i]) )
        {
            return false;
        }
    }
    return true;
}

bool validation_string(const char* s, int len )
{
    for(int i=0; i<len; i++)
    {
        if( (s[i] >= '0' && s[i] <= '9') || ( s[i] >= 'A' && s[i] <= 'Z' ) || ( s[i] >= 'a' && s[i] <= 'z' ) || ( s[i] == '=' || s[i] == '+'  || s[i] == '/'  || s[i] == '_' ) )
        {

        }
        else
        {
            return false;
        }
    }
    return true;
}

/**
 * Объединяет масив целых чисел в строку.
 * @param delimetr Разделитель между объединяемыми строками
 * @param answer Буфер для записи ответа, должен содержать только символы \0
 * @param data Данные
 * @param len Количество данных
 * @return
 * /
bool join_string(const char delimetr, char* answer, const int* data, int len )
{
    sprintf(answer,"%d",data[0] );
    answer = answer + strlen(answer);

    if(len > 1)
    {
        for(int i=1; i<len; i++)
        {
            sprintf(answer, "%c%d", delimetr, data[i]);
            answer = answer + strlen(answer);
        }
    }

    return true;
}

/**
 * Экранирует символы для того чтоб строку можно было использовать в json. Работает с кодировкой utf8 и поддерживает многобайтные символы
 * @param string исходная строка
 * @param len длина исходной строки
 * @param buff буфер для результирующей строки
 * @return длина результирующей строки
 * @link https://ru.wikipedia.org/wiki/UTF-8
 * /
int json_escape_string(const char* string, int len, char* buffer)
{
    int buffPos = 0;
    for(int i=0; i<len; i+=0)
    {
        //printf("i=%d, c=%02x, c=%d, c=[%c]\n", i, (unsigned char)string[i], (unsigned char)string[i], string[i]);
        if( string[i] == '\\' || string[i] == '"' )
        {
            buffer[buffPos] = '\\';
            buffPos++;

            buffer[buffPos] = string[i];
            buffPos++;
            i += 1;
        }
        else if( (unsigned char)string[i] >= 255 )
        {
            // Вообще то это не по стандарту ut8 но теоретически возможно
            // Символ 8 байт
            // +7
            if(i+6 >= len)
            {
                continue;
            }
            memcpy(buffer+buffPos, string+i, 8);
            buffPos += 8;
            i += 8;
        }
        else if( (unsigned char)string[i] >= 254 )
        {
            // Вообще то это не по стандарту ut8 но теоретически возможно
            // Символ 7 байт
            // +6
            if(i+5 >= len)
            {
                continue;
            }
            memcpy(buffer+buffPos, string+i, 7);
            buffPos += 7;
            i += 7;
        }
        else if( (unsigned char)string[i] >= 252 )
        {
            // Символ 6 байт
            // +5
            if(i+4 >= len)
            {
                continue;
            }
            memcpy(buffer+buffPos, string+i, 6);
            buffPos += 6;
            i += 6;
        }
        else if( (unsigned char)string[i] >= 248 )
        {
            // Символ 5 байт
            // +4
            if(i+3 >= len)
            {
                continue;
            }
            memcpy(buffer+buffPos, string+i, 5);
            buffPos += 5;
            i += 5;
        }
        else if( (unsigned char)string[i] >= 240 )
        {
            // Символ 4 байта
            // +3
            if(i+2 >= len)
            {
                continue;
            }
            memcpy(buffer+buffPos, string+i, 4);
            buffPos += 4;
            i += 4;
        }
        else if( (unsigned char)string[i] >= 224 )
        {
            // Символ 3 байта
            // +2
            if(i+1 >= len)
            {
                continue;
            }
            memcpy(buffer+buffPos, string+i, 3);
            buffPos += 3;
            i += 3;
        }
        else if( (unsigned char)string[i] >= 192 )
        {
            // Символ 2 байта
            // +1
            if(i >= len)
            {
                continue;
            }
            memcpy(buffer+buffPos, string+i, 2);
            buffPos += 2;
            i += 2;
        }
        else if( (unsigned char)string[i] < 20 )
        {
            buffer[buffPos] = '\\';
            buffPos++;

            buffer[buffPos] = 'u';
            buffPos++;

            buffer[buffPos] = '0';
            buffPos++;

            buffer[buffPos] = '0';
            buffPos++;

            buffer[buffPos] = '0' + ( string[i] > 9 );
            buffPos++;

            buffer[buffPos] = '0' + ( string[i] % 10 );
            buffPos++;
            i += 1;
        }
        else
        {
            buffer[buffPos] = string[i];
            buffPos++;
            i += 1;
        }
    }

    //printf("string: %d[%s]\n", len, string);
    //printHex(string, len);

    //printf("buffer: %d[%s]\n", buffPos, buffer);
    //printHex(buffer, buffPos);
    return buffPos;
} //*/


bool exec(const char* cmd, char* buff, int buff_size)
{
    char buffer[128];
    FILE* fp = popen(cmd, "r");
    if(fp == NULL)
    {
        return false;
    }
    bzero(buff, buff_size);
    char* p = fgets(buff, buff_size, fp);
    pclose(fp);
    return p != NULL;
}

/**
 * Заменяет кавычки экранированые кавычками на кавычки экранированые слешем обратным.
 * Ожидает что строка закончится четерьмя нулевыми символами.
 * @param query
 * @return 
 */
bool strReplaceQuote(char* query)
{
    bool isReplace = false;
    // Замена экранирования кавычки кавычкой на экранирование кавычки слешем.
    bool inStringQQ = false;
    bool inStringQ = false;
    int i = 0;
    do{
        if(inStringQQ == false && inStringQ == false && query[i] == '\'')
        {
            inStringQ = true;
        }
        else if(inStringQQ == false && inStringQ == true && query[i] == '\'' && query[i - 1] != '\\')
        {
            if(query[i + 1] == '\'')
            {
                query[i] = '\\';
                isReplace = true;
                i++;
            }
            else
            {
                inStringQ = false;
            }
        }

        if(inStringQ == false && inStringQQ == false && query[i] == '"')
        {
            inStringQQ = true;
        }
        else if(inStringQ == false && inStringQQ == true && query[i] == '"' && query[i - 1] != '\\')
        {
            if(query[i + 1] == '"')
            {
                query[i] = '\\';
                isReplace = true;
                i++;
            }
            else
            {
                inStringQQ = false;
            }
        }

        i++;
        // Четыре нулевых символа чтоб не напоротся на проблемы с utf8 многобайтовыми символами.
    }while(query[i] != 0 && query[i+1] != 0 && query[i+2] != 0 && query[i+3] != 0);
    
    return isReplace;
}