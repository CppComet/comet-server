// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com



#include "backtrace.h"

#include <cstdlib>
#include <iostream>
#include <memory>

#include <pthread.h>

#include <time.h>

#include <sys/stat.h>
#include <sys/wait.h>

#include <execinfo.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static void full_write(int fd, const char *buf, size_t len)
{
        while (len > 0) {
                ssize_t ret = write(fd, buf, len);

                if ((ret == -1) && (errno != EINTR))
                        break;

                buf += (size_t) ret;
                len -= (size_t) ret;
        }
}

void print_backtrace(void)
{
        static const char start[] = "\x1b[1;31m------------ BACKTRACE ------------\x1b[0m\n";
        static const char end[] =   "\x1b[1;31m-----------------------------------\x1b[0m\n";

        void *bt[1024];
        int bt_size;
        char **bt_syms;
        int i;

        bt_size = backtrace(bt, 1024);
        bt_syms = backtrace_symbols(bt, bt_size);
        full_write(STDERR_FILENO, start, strlen(start));
        for (i = 1; i < bt_size; i++) {
                size_t len = strlen(bt_syms[i]);
                full_write(STDERR_FILENO, bt_syms[i], len);
                full_write(STDERR_FILENO, "\n", 1);
        }
        full_write(STDERR_FILENO, end, strlen(end));
    free(bt_syms);
}



void print_backtrace(const char* reason)
{
        static const char start[] = "\x1b[1;31m------------ MY-BACKTRACE ------------\x1b[0m\n";
        static const char end[] =   "\x1b[1;31m--------------------------------------\x1b[0m\n";

        void *bt[1024];
        int bt_size;
        char **bt_syms;
        int i;

        bt_size = backtrace(bt, 1024);
        bt_syms = backtrace_symbols(bt, bt_size);
        full_write(STDERR_FILENO, start, strlen(start));
        full_write(STDERR_FILENO, reason, strlen(reason));
        for (i = 1; i < bt_size; i++) {
                size_t len = strlen(bt_syms[i]);
                full_write(STDERR_FILENO, bt_syms[i], len);
                full_write(STDERR_FILENO, "\n", 1);
        }
        full_write(STDERR_FILENO, end, strlen(end));
    free(bt_syms);
}
