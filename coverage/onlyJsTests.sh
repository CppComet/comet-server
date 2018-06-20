#!/bin/bash
 
./jstestrun.sh > /tmp/last-tests.log 
 
cat /tmp/last-tests.log | grep -A 4 "Coverage summary" 

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

 