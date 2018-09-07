#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <stddef.h>
#include <ctype.h>
#include <sys/stat.h>
#include <pthread.h>

#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "util.h"
#include "smp_md5.h"
#include "cpumem.h"
#include "md5.h"
#include "config.h"
#include "cjson.h"
#include "mongoose.h"
#include "zlog.h"
#include "client_func.h"
#include "fdfs_client.h"
#include "hiredis.h"  
#include "util.h"  

int fast_upload_file(void *data, char *cl_pre) 
{
	struct distribute_info *info = (struct distribute_info *)data;
	char	conf_filename[32] = {0};//"client.conf";	
	char 	local_filename[256] = {0};	
	char 	file_id[128]    = {0};		
	char 	group_name[FDFS_GROUP_NAME_MAX_LEN + 1] = {0};	

	ConnectionInfo *pTrackerServer = NULL;	

	int result 	     = 0;	
	int store_path_index = 0;	

	ConnectionInfo storageServer;	

	//long long start_time = _gettime_s();
	sprintf(conf_filename, "%s_client.conf", cl_pre);

	if ((result=fdfs_client_init(conf_filename)) != 0)	
	{		
		return result;	
	}	

	pTrackerServer = tracker_get_connection();	
	if (pTrackerServer == NULL)	
	{		
		fdfs_client_destroy();		
		return errno != 0 ? errno : ECONNREFUSED;	
	}	

	*group_name = '\0';	

        if ((result=tracker_query_storage_store(pTrackerServer, \
                        &storageServer, group_name, &store_path_index)) != 0)
        {
                fdfs_client_destroy();
                fprintf(stderr, "tracker_query_storage fail, " \
                        "error no: %d, error info: %s\n", \
                        result, STRERROR(result));
                return result;
        }

	int i = 0;	
	for(i = 0; i < info->chips_info->ts_chips_nums; i++)
	{
		sprintf(local_filename, "%s/%s", info->video_src_path, info->chips_info->ts_chips_filename[i]);
        	result = storage_upload_by_filename1(pTrackerServer, \
                        &storageServer, store_path_index, \
                        local_filename, NULL, \
                        NULL, 0, group_name, file_id);
        	if (result == 0)
        	{
			memcpy(info->chips_info->ts_chips_distribute_path[i], file_id, strlen(file_id));
        	}
        	else
        	{
                	fprintf(stderr, "upload file fail, " \
                        	"error no: %d, error info: %s\n", \
                        	result, STRERROR(result));
        	}
	}

        tracker_disconnect_server_ex(pTrackerServer, true);
        fdfs_client_destroy();

	//long long end_time = _gettime_s();

	//printf("start_time = %lld, end_time = %lld, upload_time = %lld\n", start_time, end_time, end_time - start_time);
	return 0;
}
