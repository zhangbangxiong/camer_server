#!/bin/sh
pro="stream_server"

while [ 1 ]
do
        num=`/bin/ps -ef | /bin/grep ${pro} | /bin/grep -v grep | /usr/bin/wc -l`
        if [ $num -eq 0 ];then
            /usr/local/pserver/stream_server /usr/local/pserver/server.conf /usr/local/pserver/etc.xml
        fi
    	sleep 5
done


