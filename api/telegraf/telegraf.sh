#!/bin/bash
 
# telegraf --config-directory=/etc/telegraf --test --input-filter=exec

COMETUSER="root"
COMETPASS="password"
COMETHOST="127.0.0.1"
COMETPORT=3306
COMETDB="CometQL_v1"
  
result=$(echo "show global status" | mysql  --skip-column-names -s --skip-ssl  -u$COMETUSER -p$COMETPASS -h$COMETHOST -P$COMETPORT -D$COMETDB)

j=0
k=0
key=""
 
count=${#result[@]}
regex="^[0-9.]+$"

echo -n "{"

for i in $result
do
    if [ $j -eq 1 ]
    then
        j=0
        if [[ $i =~ $regex ]]
        then
            echo -n "\"$key\":$i,"
        fi
    else
        j=1
        key=$i
    fi
done

echo -n "\"end\":true}"
