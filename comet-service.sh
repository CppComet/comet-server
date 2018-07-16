#!/bin/bash

echo "CppComet"

PATH=/sbin:/usr/sbin:/bin:/usr/bin
RUNDIR=/usr/bin
EXTRAOPTS=
PIDFILE=/var/run/cppcomet.pid


cd ${RUNDIR} 

case "$1" in
    start)  
        if test -f "$PIDFILE"
        then
            echo "CppComet already run"
        else
            echo "Starting CppComet"
            ulimit -n 100000
            /usr/bin/cppcomet --conf /etc/comet-server/comet.ini > /var/log/cpp_comet.log 2>/var/log/cpp_comet.log &
        fi
        ;;
    stop)
        if test -f "$PIDFILE"
        then
            echo "Stopping CppComet"
            killall cppcomet
            kill `cat $PIDFILE`
            `rm -rf $PIDFILE`
        else
            echo "Stopping CppComet"
            killall cppcomet
            #echo "pid file does not exist" 
	        #echo 'exit' > /tmp/cpp.comet
        fi
        ;;
       *)
        if test -f "$PIDFILE"
        then
            echo "Stopping CppComet"
            killall -9 cppcomet
            kill `cat $PIDFILE`
            `rm -rf $PIDFILE`
        else
            echo "pid file does not exist" 
        fi
        echo "Starting CppComet"
	sleep 15
        ulimit -n 100000
        /usr/bin/cppcomet --conf /etc/comet-server/comet.ini > /var/log/cpp_comet.log 2>/var/log/cpp_comet.log &
        ;;
esac