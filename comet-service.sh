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


echo "Комет сервер"
 
cd ${RUNDIR}
 
case "$1" in
    start)    
        startCount=`ps auxH | grep "comet-start.sh" | wc -l`
        if [ $startCount -ne 2 ]
        then
            count=`ps auxH | grep "cpp_comet" | wc -l`
            if [ $count -le 40  ]
            then
                echo "Запуск комет сервера"
                nohup ./comet-start.sh > mfcs.out &
            else
                echo "Комет сервер уже запущен"
            fi
        else
            echo "comet-start.sh уже запущен"
        fi
        ;;
    stop)
        echo "Остановка комет сервера"
        ps aux | grep -E "comet-start.sh" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        ps auxH | grep "comet-start.sh" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        
        
        ps aux | grep -E "cpp_comet" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        ps auxH | grep "cpp_comet" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        
        echo "" > starComet-old.log
        #cp mfcs.out starComet-old.log
        echo "" > mfcs.out
        ;;
    restart)
        echo "Остановка комет сервера"
        ps aux | grep -E "comet-start.sh" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        ps auxH | grep "comet-start.sh" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        
        ps aux | grep -E "cpp_comet" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        ps auxH | grep "cpp_comet" | grep -o -E "root +([0-9]+)" | grep -o -E "[0-9]+" | xargs kill 
        echo "" > starComet-old.log
        #cp mfcs.out starComet-old.log
        echo "" > mfcs.out

        startCount=`ps auxH | grep "comet-start.sh" | wc -l`
        if [ $startCount -ne 2 ]
        then
            count=`ps auxH | grep "cpp_comet" | wc -l`
            if [ $count -le 40  ]
            then
                echo "Запуск комет сервера"
                nohup ./comet-start.sh > mfcs.out &
            else
                echo "Комет сервер уже запущен"
            fi
        else
            echo "comet-start.sh уже запущен"
        fi
        ;;
    *)
        echo "Не верная команда"
        ;;
esac
