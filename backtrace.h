// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <stdio.h>

//static void full_write(int fd, const char *buf, size_t len);

/**
 * Для записи backtrace в файл в случаи ошибки
 */
void print_backtrace(void);

/**
 * Для вывода backtrace в консоль в случаи ошибки
 */
void print_backtrace(const char* reason);