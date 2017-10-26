#!/bin/bash

echo "CppComet"

PATH=/sbin:/usr/sbin:/bin:/usr/bin
RUNDIR=/etc/comet-server/
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
            ./cpp_comet > /var/log/cpp_comet.log 2>/var/log/cpp_comet.log &
        fi
        ;;
    stop)
        if test -f "$PIDFILE"
        then
            echo "Stopping CppComet"
            kill `cat $PIDFILE`
            `rm -rf $PIDFILE`
        else
            echo "pid file does not exist" 
	        echo 'exit' > /tmp/cpp.comet
        fi
        ;;
    *)
        if test -f "$PIDFILE"
        then
            echo "Stopping CppComet"
            kill `cat $PIDFILE`
            `rm -rf $PIDFILE`
        else
            echo "pid file does not exist" 
        fi
        echo "Starting CppComet"
        ./cpp_comet > /var/log/cpp_comet.log 2>/var/log/cpp_comet.log &
        ;;
esac


