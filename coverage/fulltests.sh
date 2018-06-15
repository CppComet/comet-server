#!/bin/bash

cmake .
make

./jstestrun.sh &

# Запуск под valgrind
./cppcomet_coverage --conf ./cometN1.ini --cli < ./cli.txt  
#./cppcomet_coverage --conf ./cometN2.ini --cli < ./cli.txt  
 
./coverage.sh