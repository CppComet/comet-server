#!/bin/bash
### BEGIN INIT INFO
# Provides:          starComet
# Required-Start:    $local_fs $network $remote_fs $syslog
# Required-Stop:     $local_fs $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: comet service comet-server.com
# Description:       comet service comet-server.com
### END INIT INFO

PATH=/sbin:/usr/sbin:/bin:/usr/bin
RUNDIR=/etc/comet-server/
EXTRAOPTS=
PIDFILE=/var/run/cppcomet.pid

echo "CppComet"

cd ${RUNDIR}

case "$1" in
    start) 
        if [test -f "$PIDFILE"]
        then
            echo "CppComet already run"
        else
            echo "Starting CppComet"
            nohup ./comet-start.sh > /var/log/cpp_comet.log &
        fi
        ;;
    stop)
        echo "Stopping CppComet"
        kill `cat $PIDFILE`
        `rm -rf $PIDFILE`
        ;;
    *)
        echo "Stopping CppComet"
        kill `cat $PIDFILE`
        `rm -rf $PIDFILE`

        echo "Starting CppComet"
        nohup ./cpp_comet > /var/log/cpp_comet.log &
        ;;
esac