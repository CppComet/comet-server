#!/bin/bash

 # Запуск с coverage
cmake .
make

# Запуск под valgrind
./cpp_comet_coverage --conf ./cometN1.ini

