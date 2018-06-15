#!/bin/bash

# Запуск с coverage
cmake .
make

# Запуск под valgrind
./cppcomet_coverage --conf ./cometN1.ini

