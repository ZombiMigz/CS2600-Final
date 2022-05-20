#!/bin/bash
IP=192.168.4.61
RES=gibberish
TEMP_MAX=90
TEMP_MIN=80
HUM_MAX=70
HUM_MIN=60
TIME_MAX=2
ISSUE=0
TIME=0

cd "C:\Program Files\mosquitto"
( 
  while read -u 10 RES
  do
    echo "Result: $RES"
    if [[ ${RES::1} == "T" ]];
    then
      if [[ ${RES:1:2} -gt TEMP_MAX ]];
      then
        echo "TOO HOT"
        ./mosquitto_pub.exe -h "192.168.4.61" -t "controller/freeze" -m "1"
        ISSUE=1
      else 
        ./mosquitto_pub.exe -h "192.168.4.61" -t "controller/freeze" -m "0"
      fi

      if [[ ${RES:1:2} -lt TEMP_MIN ]];
      then
        echo "TOO COLD"
        ./mosquitto_pub.exe -h "192.168.4.61" -t "controller/heat" -m "1"
        ISSUE=1
      else 
        ./mosquitto_pub.exe -h "192.168.4.61" -t "controller/heat" -m "0"
      fi
    fi

    if [[ ${RES::1} == "H" ]];
    then
      if [[ ${RES:1:2} -gt HUM_MAX ]];
      then
        echo "TOO HUMID"
        ./mosquitto_pub.exe -h "192.168.4.61" -t "controller/dry" -m "1"
        ISSUE=1
      else 
        ./mosquitto_pub.exe -h "192.168.4.61" -t "controller/dry" -m "0"
      fi

      if [[ ${RES:1:2} -lt HUM_MIN ]];
      then
        echo "TOO DRY"
        ./mosquitto_pub.exe -h "192.168.4.61" -t "controller/humidify" -m "1"
        ISSUE=1
      else 
        ./mosquitto_pub.exe -h "192.168.4.61" -t "controller/humidify" -m "0"
      fi

      
      if [[ $ISSUE -eq 1 ]];
      then
        echo "ISSUE FOUND"
        let TIME++
        echo "Error Time: $TIME"
        
        if [[ $TIME -gt $TIME_MAX ]];
        then
        echo "MAX TIME EXCEEDED, SOUNDING ALARM"
        ./mosquitto_pub.exe -h "192.168.4.61" -t "controller/sound" -m "1"
        fi
      else
        echo "ISSUE NOT FOUND"
        ./mosquitto_pub.exe -h "192.168.4.61" -t "controller/sound" -m "0"
        TIME=0
      fi
      
    fi

  done 10< <(./mosquitto_sub.exe -h "192.168.4.61" -t "sensor/humidity" -t "sensor/temperature")
   
)