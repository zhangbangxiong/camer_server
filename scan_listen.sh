#!/bin/sh
pro="scan"
hasrestart=0

while [ 1 ]
do
        timeh=$(date "+%H")
        if [ $timeh -eq 8 -a $hasrestart -eq 0 ]; then
            echo "kill pro scan"
            pkill scan
            hasrestart=1
            /home/pstream/scan
        elif [ $timeh -eq 9 -a $hasrestart -eq 1 ]; then
            echo "hasrestart 0"
            hasrestart=0
        fi
    	sleep 2
done


