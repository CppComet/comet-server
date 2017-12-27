// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* 
 * File:   mystring.h
 * Author: victor
 *
 * Created on 19 Ноябрь 2012 г., 13:58
 */

/**
 * Содержит вспомагательные функции работы со строками
 */

#ifndef MYSTRING_H
#define	MYSTRING_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

void uuid37(char* src);

int str_find(const char* s,char c,int n,int start);
int str_find(const char* s,char c,int n,int start, int end);
int str_find(const char* s,const char c );
int str_find(const char* s,const char c,int len );

/**
 * Проверяет строку на соответсвиее выражению [A-z0-9/+=]
 * @param s строка
 * @param len длина строки
 * @return true если строка содержит символы [A-z0-9/+=]
 */
bool validation_string(const char* s, int len );


/**
 * В answer запишет строку со значениями массива data разделёнными символом delimetr
 * @param delimetr разделитель
 * @param answer Буфер для записи результата
 * @param data Масив с данными
 * @param len Длина масива с данными
 * @return 
 */
bool join_string(const char delimetr, char* answer, const int* data, int len );


/**
 * Проверка символа p на соответсвие следующему регулярному выражению [A-Za-z0-9\-_]
 * @param p
 * @return true в случаии соответсвия
 */
bool AZ09test(char p);
bool AZ09test(const char* s, int len );

/**
 * Экранирует символы для того чтоб строку можно было использовать в json. Работает с кодировкой utf8 и поддерживает многобайтные символы
 * @param string исходная строка
 * @param len длина исходной строки
 * @param buff буфер для результирующей строки
 * @return длина результирующей строки
 * @link https://ru.wikipedia.org/wiki/UTF-8  
 */
int json_escape_string(const char* string, int len, char* buffer);

/**
 * Выполнит системный вызов и вернёт результат в виде строки
 * @param cmd
 * @param buff
 * @param buff_size
 * @return false если чтение не удалось
 */
bool exec(const char* cmd, char* buff, int buff_size);

/**
 * Заменяет кавычки экранированые кавычками на кавычки экранированые слешем обратным.
 * Ожидает что строка закончится четерьмя нулевыми символами.
 * @param query
 * @return 
 */
bool strReplaceQuote(char* query);

int read_int(const char* buffer, char delimiter, int* delta);
int read_int(const char* buffer, char delimiter);
int read_int(const char* buffer, int* delta);

long read_long(const char* buffer, char delimiter, int* delta);
long read_long(const char* buffer, char delimiter);
long read_long(const char* buffer, int* delta);


#endif	/* MYSTRING_H */

