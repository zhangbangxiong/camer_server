#!/bin/sh
log_path=$1
pro="ffmpeg"

if [ -z $log_path ];then
	echo "please input log path"
	exit 0
fi

if [ ! -d $log_path ];then
	echo "log path is not exist"
	exit 0
fi

while [ 1 ]
do
	log=`ps -ef | grep -v grep | grep mx.encode.sh | awk '{print $NF}'`
        _log_arr=(`echo $log | cut -d " "  --output-delimiter=" " -f 1-`)	
        num=${#_log_arr[@]}
	for((i=0;i<$num;i++))
	do
		stream_id=${_log_arr[$i]}
		echo $stream_tag
		log_file="$log_path$stream_id.log"
		echo $log_file
		if [ ! -f $log_file ];then
			continue
		fi
   		_time=`stat -c %y $log_file | awk '{print $2}'`
    		timestamp=`date -d $_time  "+%s"` 
		echo $timestamp
        	nowtime=`date "+%s"`
		echo "==$nowtime"
		
		res=`expr $nowtime - $timestamp`
		echo $res
		#res=$(($nowtime-$timestamp))
        	if [[ $res -gt 10 ]];then
			let stream_tag=10000+$stream_id
			_pid=`ps -ef |grep -Enr $pro | grep -v grep | grep flv | grep $stream_tag| awk '{print $2}'`
			echo "---------------------------------------------"
			kill -9 $_pid
			echo "$(date +%Y-%m-%d:%H:%M:%S) kill this stream $stream_id"
			echo "---------------------------------------------"
		fi
	done
	echo "..."
    	sleep 2
done


