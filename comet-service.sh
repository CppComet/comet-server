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
RUNDIR=/root/comet/
CONFIG=/root/comet/comet.conf  
EXTRAOPTS=
PIDFILE=


echo "CppComet"
 
cd ${RUNDIR}
 
case "$1" in
    start)    
        startCount=`ps auxH | grep "comet-start.sh" | wc -l`
        if [ $startCount -ne 2 ]
        then
            count=`ps auxH | grep "cpp_comet" | wc -l`
            if [ $count -le 40  ]
            then
                echo "Starting CppComet"
                nohup ./comet-start.sh > cpp_comet.log &
            else
                echo "CppComet already run"
            fi
        else
            echo "CppComet already run"
        fi
        ;;
    stop)
        echo "Stopping CppComet"
        ps aux | grep -E "comet-start.sh" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        ps auxH | grep "comet-start.sh" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        
        
        ps aux | grep -E "cpp_comet" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        ps auxH | grep "cpp_comet" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
         
        ;;
    *)
        echo "Stopping CppComet"
        ps aux | grep -E "comet-start.sh" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        ps auxH | grep "comet-start.sh" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        
        ps aux | grep -E "cpp_comet" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        ps auxH | grep "cpp_comet" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
         
        startCount=`ps auxH | grep "comet-start.sh" | wc -l`
        if [ $startCount -ne 2 ]
        then
            count=`ps auxH | grep "cpp_comet" | wc -l`
            if [ $count -le 40  ]
            then
                echo "Starting CppComet"
                nohup ./comet-start.sh > cpp_comet.log &
            else
                echo "CppComet already run"
            fi
        else
            echo "CppComet already run"
        fi
        ;; 
esac