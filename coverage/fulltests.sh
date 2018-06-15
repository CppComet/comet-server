#!/bin/bash

cmake .
make

./jstestrun.sh > /tmp/last-tests.log &

# Запуск под valgrind
valgrind ./cppcomet_coverage --conf ./cometN1.ini --cli < ./cli.txt  
#./cppcomet_coverage --conf ./cometN2.ini --cli < ./cli.txt  
 
./coverage.sh

#cat /tmp/last-tests.log | grep "\[js-test-done-ok\]" | wc