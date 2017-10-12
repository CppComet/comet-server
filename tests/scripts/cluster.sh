#!/bin/bash

cd ../../coverage
cmake .
make

cd ../tests/scripts

print "start N1"
nohup ./cpp_comet --conf ./cometN1.ini > ./cpp_cometN1.log &

print "start N2"
nohup ./cpp_comet --conf ./cometN2.ini > ./cpp_cometN2.log &

print "start N3"
nohup ./cpp_comet --conf ./cometN3.ini > ./cpp_cometN3.log &


echo "INSERT INTO pipes_messages (name, event, message)VALUES('web_MainPageChat', '', '{ \"text\":\"`cat /proc/loadavg`\",\"name\":\"AVG `uname -n`\"}' );" | mysql -h test.comet.su -uroot -p0000000000000000000000000000000000000000000000000000000000000000 -DCometQL_v1 --skip-ssl