#!/bin/bash

cmake .
make

./jstestrun.sh > /tmp/last-tests.log &

# Запуск под valgrind
valgrind ./cppcomet_coverage --conf ./cometN1.ini --cli < ./cli.txt  > /tmp/last-cppcomet-N1-tests.log 
#./cppcomet_coverage --conf ./cometN2.ini --cli < ./cli.txt  
 
./coverage.sh > /tmp/last-cppcomet-N1-test-coverage.log 

#cat /tmp/last-tests.log | grep "\[js-test-done-ok\]" | wc
 
cat /tmp/last-tests.log | grep -A 4 "Coverage summary" 
cat /tmp/last-cppcomet-N1-test-coverage.log | grep -A 2 "Overall coverage rate:"


testIsOk=`cat /tmp/last-tests.log | grep '\[js-test-done-ok\]' | wc -l`

if [ "$testIsOk" == "1" ];
then
    echo "Tests completed"
    exit 0
else
    cat  /tmp/last-tests.log
    echo "Tests failed"
    exit -1
fi

 