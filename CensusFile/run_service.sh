#!/bin/sh

PROGNAME="rtsp_to_rtmp"
mkdir -p /mnt/log/
logdir=$1

if [ -z $1 ];then
	echo "input error"
        echo "---------------------------------------"
        echo "please input the startup parameter.eg:"
        echo "start:"
        echo "run_service.sh start"
        echo "stop:"
        echo "run_service.sh stop"
        echo "---------------------------------------"
        exit 0
elif [ $1 == "stop" ];then
        echo "stop ...."
elif [ $1 == "status" ];then
        echo "status ...."
fi

start()
{
	nohup /home/pak/rtsp_to_rtmp > /mnt/log/rtsp_to_rtmp.log &
	nohup /home/pak/listen.sh /mnt/log > /mnt/log/listen.log &
}

stop()
{
	num=`ps -ef | grep  rtsp_to_rtmp | grep -v grep | wc -l`
	proid=`ps -ef | grep rtsp_to_rtmp | grep -v grep | awk '{print $2}' | head -n 1` 
	if [ $num -gt 0 ] && [ -n $proid ];then
		kill -9  $proid
	fi
	killall -9 rtsp_to_rtmp
	killall -9 ffmpeg
	killall -9 listen.sh
        echo "stop done"
}

status()
{
        num=`ps -ef|grep $PROGNAME |grep -v grep | wc -l`
        if [ $num -lt 0 ]
        then
                echo "progamme is not running"
        else
                echo "progamme is running"
        fi
}

case $1 in
   start)  start        ;;
   stop)   stop ;;
   status) status;;
   *)     echo "Usage: $0 {start|stop|status}";;
esac

exit 0
