#!/bin/bash
stream_id=$1

log=`ps -ef | grep -v grep | grep mx.encode.sh | grep $stream_id | awk '{print $2}'`
echo $log
_log_arr=(`echo $log | cut -d " "  --output-delimiter=" " -f 1-`)
echo $_log_arr
num=${#_log_arr[@]}
for((i=0;i<$num;i++))
do
	pid=${_log_arr[$i]}
	kill -9 $pid
done

ffmpeg_id=`ps -ef | grep -v grep | grep ffmpeg | grep $stream_id | awk '{print $2}'`
kill -9 $ffmpeg_id


