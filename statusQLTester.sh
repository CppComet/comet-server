#!/bin/bash
 
 
# nohup ./statusQLTester.sh > statusQLTester.out &
 
# Максимально допустимое время которое проверяемый сервер может не отвечать правильно.
MaxDounTime=3
 
greenTime=400
warnTime=1000
redTime=2000

qlHost=127.0.0.1
qlPort=3307

timestamp() {
  date +"%s%N"
}


timestampU() {
  date +"%s"
}

echo "Запуск мониторинга на `hostname` для $qlHost:$qlPort" | mail -s "[COMET-INFO]Запуск мониторинга на `hostname` для $qlHost:$qlPort" root@comet-server.ru

# Проверяет ответ сервера полученый в результате запроса статуса
pingResult() {

    

    maxTimeout=2   
    startLoad=$(timestamp)
    echo "show status" | mysql -h$qlHost -P$qlPort -uroot -pjT7P20e5EUd8DXK3RgIJrthrDGrtvUzUqFc2ZEgL4LSb5Ml7nwGdfVmS5N3o0zAx -DCometQL_v1 --connect-timeout=1  --skip-ssl > ./qlTester.log
    endLoad=$(timestamp)

    resTime=$((endLoad/1000000 - startLoad/1000000))
     
    if grep -q "Variable_name" ./qlTester.log  ## Регулярка для проверки правильности ответа от сервера который мы мониторим  ## 
    then 
        echo "$resTime"
    else
        echo "-1"
    fi
}
 
# Проверяет ответ сервера полученый в результате запроса статуса
yandexTest() { 
    maxTimeout=2  
    wget  --tries=3 --timeout=$maxTimeout --dns-timeout=$maxTimeout --connect-timeout=$maxTimeout --read-timeout=$maxTimeout --waitretry=1 --no-dns-cache --output-document=/tmp/qlTesterData.log --append-output=/dev/null http://yandex.ru 

    if grep -q "yandex" /tmp/statusTesterData.log   ## Регулярка для проверки правильности ответа от яндекса ## 
    then 
        echo "1"
    else
        echo "-1"
    fi
}


logTime=$(timestampU)
echo "$logTime:start" >> statusQLTester.log

YaTest=0
while true
do
 
    ping=$(pingResult)

    if [ "$ping" != "-1" ] # если ответ пришол правильный
    then
        if  [ "$ping" -lt $greenTime ]
        then 
                echo "\033[1;32mping:$ping\033[0m" 
        else 
            if  [ "$ping" -lt $warnTime ]
            then 
                echo "\033[1;33mping:$ping\033[0m"
            else 
                if  [ "$ping" -lt $redTime ]
                then 
                    echo "\033[1;31mping:$ping\033[0m" 
                else
                    echo "\033[5;31mping:$ping\033[0m"  
                fi
            fi
        fi

        if [ "$YaTest" -gt "0" ] 
        then
            ## Действия при возобновлении сети у сервера на котором выполняется скрипт ## 
            YaTestEnd=$(timestampU)
            YaTest=$((YaTestEnd - YaTest))
            YaTestH=$((YaTest/3600))
            echo "\033[1;33mСвязь востановлена прошло $YaTestH часов или $YaTest секунд\033[0m" 
            echo "$YaTestEnd:Связь востановлена прошло $YaTestH часов или $YaTest секунд" >> statusQLTester.log
            echo "Связь востановлена  $YaTestEnd прошло $YaTestH часов или $YaTest секунд" | mail -s "[COMET-INFO]Связь востановлена на ноде `hostname` для $qlHost:$qlPort в $YaTestEnd прошло $YaTestH часов или $YaTest секунд" root@comet-server.ru
        fi
        YaTest=0
        ctime=$(timestampU)
        echo "$ctime $ping" >> statusPing.log
    else
        echo "\033[1;31mНе правильный ответ от сервера 1\033[0m"
        
        ctime=$(timestampU)
        echo "$ctime 0" >> statusPing.log
        
        i=1
        ping=$(pingResult)
        while [ "$ping" -eq "-1" ]
        do
            ctime=$(timestampU)
            echo "$ctime 0" >> statusPing.log
            if [ "$i" -gt $MaxDounTime ]
            then
            
                #pingYa=$(yandexTest)      
                #if [ "$pingYa" -gt "0" ]
                #then
                    ## Действия при падении сервера который мы мониторим ## 
                    echo "\033[1;31mНе правильный ответ от сервера больше $MaxDounTime\033[0m"
                    #echo "\033[1;33mЗапуск bind9 для работы на резервном сервере\033[0m" 
                    
                    logTime=$(timestampU)
                    echo "$YaTest: Не правильный ответ от сервера больше $MaxDounTime" >> statusQLTester.log
                    #echo "$YaTest: Запуск bind9 для работы на резервном сервере" >> statusQLTester.log
                    
                    #rm -rf /etc/bind/db.app_comet_server_ru
                    #ln -s etc/bind/error.app_comet_server_ru /etc/bind/db.app_comet_server_ru
                    #service bind9 restart
                    
                    #wget  --output-document=sms.log "http://gate.smsaero.ru/send/?to=79244269357&text=BindRestart-to-node3&user=levhav@gmail.com&password=0e311e5b9704f28b4e8557e8fa3fbe7d&from=CometServer"
                                    
                    #echo "Перезапущен Bind на http://109.234.34.208/ с http://185.4.72.57/ " | mail -s "[COMET-INFO]Bind restart to node 3" levhav@yandex.ru     
                    echo "Перезапущен comet server на `hostname` " | mail -s "[COMET-INFO]`hostname` restart для $qlHost:$qlPort" Levhav@ya.ru
                    #echo "service bind9 restart"


                    echo "Перезапуск комет сервера"
                    echo 'exit' > /tmp/star.comet
                    i=-30
                    #exit;
                #fi
                
                 
                #if [ "$YaTest" -eq "0" ] 
                #then 
                    ## Действия при потери сети у сервера на котором выполняется скрипт ## 
                    YaTest=$(timestampU)
                    echo "\033[1;33mСвязь потеряна в $YaTest\033[0m"
                    echo "$YaTest: Связь потеряна" >> statusQLTester.log
                    echo "Связь потеряна в $YaTest" | mail -s "[COMET-INFO]Связь потеряна на ноде `hostname` для $qlHost:$qlPort в $YaTest" root@comet-server.ru
                #fi
            fi
            
            sleep 1
            i=$(($i+1))
            ping=$(pingResult)
            echo "\033[1;31mНе правильный ответ от сервера $i\033[0m"
        done 
    fi
    sleep 2
done

# Создание

#chmod 777 ./statusQLTester.sh && nohup ./statusQLTester.sh > statusQLTester.out &
#rm -rf /etc/bind/db.app_comet_server_ru
#ln -s  /etc/bind/base.app_comet_server_ru /etc/bind/db.app_comet_server_ru




