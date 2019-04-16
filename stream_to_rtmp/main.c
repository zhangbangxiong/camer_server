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
#include <sys/wait.h>
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
#include <getopt.h>
#include <mysql/mysql.h>

#include "curl/curl.h"
#include "util.h"
#include "tutil.h"
#include "smp_md5.h"
#include "cpumem.h"
#include "md5.h"
#include "config.h"
#include "cjson.h"
#include "mongoose.h"
#include "zlog.h"
#include "hiredis.h"  

#define API_CSTOP               "/stop"
#define API_CSTART              "/start"
#define API_CSTATUS             "/status"
#define API_CHANGE_CONFIG       "/config"

#define FILE_NAME_LENGHT 10
#define S_HTTP_PORT      "18080"
#define MG_FALSE      1
#define MG_TRUE       0
#define TIME_ORG      1499270400

#define VERSION "1.0.4"

#define GET_TOKEN "https://open.ys7.com/api/lapp/token/get"
#define GET_RTMP  "https://open.ys7.com/api/lapp/live/address/limited"

#define RTMP_P "accessToken=%s&deviceSerial=C06324069&channelNo=1"

static volatile char STOP = 0;
static volatile char STOP_OK = 0;

pthread_mutex_t threadsmtx  	= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t datamtx     	= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t limitcond    	= PTHREAD_COND_INITIALIZER;
pthread_mutex_t infomtx  	= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  infocond 	= PTHREAD_COND_INITIALIZER;
pthread_mutex_t convertmtx  	= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  convertcond 	= PTHREAD_COND_INITIALIZER;

struct list_head task_list;

config_t  _config;

char *edition      = NULL;
char *server_id    = NULL;
char local_ip[128] = "0.0.0.0";
int  thread_nums   = 0;

int isvalid_ip(char *ip)
{
    if (ip==NULL)
        return -1;

    char temp[4];
    int count=0;
    while (1)
    {
        int index=0;
        while (*ip!='\0' && *ip!='.' && count<4)
        {
            temp[index++]=*ip;
            ip++;
        }

        if (index==4)
            return -1;

        temp[index]='\0';
        int num=atoi(temp);

        if (!(num>=0 && num<=255))
            return -1;

        count++;
        if (*ip=='\0')
	{
            if(count==4)
                return 1;
            else
                return -1;
        }
	else
            ip++;
    }

    return -1;
}

static int get_local_ip()
{
	struct ifaddrs * ifAddrStruct=NULL;
	void * tmpAddrPtr=NULL;

	getifaddrs(&ifAddrStruct);

	while (ifAddrStruct!=NULL) 
	{
		if (ifAddrStruct->ifa_addr->sa_family == AF_INET) 
		{ 
			// check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if(strncmp(ifAddrStruct->ifa_name, "ens", 3) == 0 || strncmp(ifAddrStruct->ifa_name, "eth0", 4) == 0)
				memcpy(local_ip, addressBuffer, strlen(addressBuffer));
		} 
		else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6) 
		{ 
			// check it is IP6
			// is a valid IP6 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			//if(strncmp(ifAddrStruct->ifa_name, "ens", 3) == 0 || strncmp(ifAddrStruct->ifa_name, "eth0", 4) == 0)
			//	memcpy(local_ip, addressBuffer, strlen(addressBuffer));
		} 

		ifAddrStruct=ifAddrStruct->ifa_next;
	}

 	local_ip[strlen(local_ip)] = '\0';
	printf("local_ip = %s\n", local_ip);

	return 0;
}

int report_camera_status(char *camera_id, int status)
{
        char data[256] = {0};
        char url[128]  = {0};

	sprintf(url, "http://%s:%d/api/camera/callback", _config.stream_server_ip, _config.stream_server_http_port);
        sprintf(data, "--data \"camera_id=%s&stream_status=%d\"", camera_id, status);
        send_data(data, url);

        return 0;
}

static size_t write_callback2(void *ptr, size_t size, size_t nmemb, void *stream)
{
	//printf("callback1 = %s\n", ptr);
    	int len  = size * nmemb;
    	dzlog_info("callback ptr = %s\n", ptr);
  	cJSON * pjson,*psub,*psub1;

	char deviceSerial[32] = {0};
	char rtmp_addr[256]   = {0};
  	int icount=0;
  	if(NULL == ptr || len <= 0)
  	{
        	return -1;
  	}

  	pjson = cJSON_Parse(ptr);  /* 解析 json 放入 pJson*/
  	if(NULL == pjson)
  	{
    		return -1;
  	}
	psub1 = cJSON_GetObjectItem(pjson, "code");
	//printf("res = %s\n", psub1->valuestring);
	if (atoi(psub1->valuestring) == 200)
        {
		psub = cJSON_GetObjectItem(pjson, "data");
		psub1 = cJSON_GetObjectItem(psub, "deviceSerial");
                memcpy(deviceSerial, psub1->valuestring, strlen(psub1->valuestring));
		psub1 = cJSON_GetObjectItem(psub, "rtmp");
                memcpy(rtmp_addr, psub1->valuestring, strlen(psub1->valuestring));
		//printf("res = %s\n", psub1->valuestring);

		struct stream_info *info  = NULL;
		struct stream_info *_info = NULL;

		list_for_each_entry_safe(info, _info, &task_list, list)
		{
			if (strcmp(deviceSerial, info->stream_name) == 0)
			{
				memset(info->in_stream_addr, 0, sizeof(info->in_stream_addr));
				memcpy(info->in_stream_addr, rtmp_addr, strlen(rtmp_addr));

			        memset(info->protocol, 0, sizeof(info->protocol));
				memcpy(info->protocol, "rtmp", 4);

				info->status = 1;
				break;
			}
		}

	}
	return 0;
}

static size_t write_callback1(void *ptr, size_t size, size_t nmemb, void *stream)
{
    	int len  = size * nmemb;
  	cJSON * pjson,*psub,*psub1 ;
  	int icount=0;
  	if(NULL == ptr || len <= 0)
  	{
        	return -1;
  	}

  	pjson = cJSON_Parse(ptr);  /* 解析 json 放入 pJson*/
  	if(NULL == pjson)
  	{
    		return -1;
  	}
	psub1 = cJSON_GetObjectItem(pjson, "code");
	if (atoi(psub1->valuestring) == 200)
        {
		psub1 = cJSON_GetObjectItem(pjson, "data");
		psub1 = cJSON_GetObjectItem(psub1, "accessToken");
		get_rtmp(GET_RTMP, psub1->valuestring);
	}
	return 0;
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int loop = 0;
    	int len  = size * nmemb;
    	dzlog_info("callback ptr = %s\n", ptr);
  	cJSON * pjson,*psub,*psub1 ;
  	int icount=0;
  	if(NULL == ptr || len <= 0)
  	{
        	return -1;
  	}

  	pjson = cJSON_Parse(ptr);  /* 解析 json 放入 pJson*/
  	if(NULL == pjson)
  	{
    		return -1;
  	}
  	icount = cJSON_GetArraySize(pjson);
	
	for (; loop < icount; loop ++)
	{
		psub = cJSON_GetArrayItem(pjson, loop);
		psub1 = cJSON_GetObjectItem(psub, "status");
		if (atoi(psub1->valuestring) != 0)
		{
			int status = 0;

			struct stream_info *info  = NULL;
			struct stream_info *_info = NULL;

			pthread_mutex_lock(&infomtx);
			list_for_each_entry_safe(info, _info, &task_list, list)
			{
				psub1 = cJSON_GetObjectItem(psub, "stream_id");
				if (strcmp(info->stream_id, psub1->valuestring) == 0)
				{
					
					if (strcmp(info->width, cJSON_GetObjectItem(psub, "width")->valuestring) != 0  || 
					    strcmp(info->height, cJSON_GetObjectItem(psub, "height")->valuestring) != 0 || 
					    strcmp(info->bitrate, cJSON_GetObjectItem(psub, "bitrate")->valuestring) != 0) 
					{
						psub1 = cJSON_GetObjectItem(psub, "width");
						memset(info->width, 0, sizeof(info->width));
						memcpy(info->width, psub1->valuestring, strlen(psub1->valuestring));

						psub1 = cJSON_GetObjectItem(psub, "height");
						memset(info->height, 0, sizeof(info->height));
						memcpy(info->height, psub1->valuestring, strlen(psub1->valuestring));

						psub1 = cJSON_GetObjectItem(psub, "bitrate");
						memset(info->bitrate, 0, sizeof(info->bitrate));
						memcpy(info->bitrate, psub1->valuestring, strlen(psub1->valuestring));
					
						info->restart = 1;
					}

					status = 1;
					break;
				}
			}
			pthread_mutex_unlock(&infomtx);

			if (status)
				continue;

			struct stream_info *stream_news = NULL;

			stream_news = (struct stream_info *)malloc(sizeof(struct stream_info));

			psub1 = cJSON_GetObjectItem(psub, "stream_id");
			memset(stream_news->stream_id, 0, sizeof(stream_news->stream_id));
			memcpy(stream_news->stream_id, psub1->valuestring, strlen(psub1->valuestring));

			psub1 = cJSON_GetObjectItem(psub, "name");
			memset(stream_news->stream_name, 0, sizeof(stream_news->stream_name));
			memcpy(stream_news->stream_name, psub1->valuestring, strlen(psub1->valuestring));

			psub1 = cJSON_GetObjectItem(psub, "logo");
			memset(stream_news->logo, 0, sizeof(stream_news->logo));
			memcpy(stream_news->logo, psub1->valuestring, strlen(psub1->valuestring));

			psub1 = cJSON_GetObjectItem(psub, "width");
			memset(stream_news->width, 0, sizeof(stream_news->width));
			memcpy(stream_news->width, psub1->valuestring, strlen(psub1->valuestring));

			psub1 = cJSON_GetObjectItem(psub, "height");
			memset(stream_news->height, 0, sizeof(stream_news->height));
			memcpy(stream_news->height, psub1->valuestring, strlen(psub1->valuestring));

			psub1 = cJSON_GetObjectItem(psub, "bitrate");
			memset(stream_news->bitrate, 0, sizeof(stream_news->bitrate));
			memcpy(stream_news->bitrate, psub1->valuestring, strlen(psub1->valuestring));

			psub1 = cJSON_GetObjectItem(psub, "encode");
			memset(stream_news->encode, 0, sizeof(stream_news->encode));
			memcpy(stream_news->encode, psub1->valuestring, strlen(psub1->valuestring));

			psub1 = cJSON_GetObjectItem(psub, "in_stream_addr");
			memset(stream_news->in_stream_addr, 0, sizeof(stream_news->in_stream_addr));
			memcpy(stream_news->in_stream_addr, psub1->valuestring, strlen(psub1->valuestring));
			memset(stream_news->protocol, 0, sizeof(stream_news->protocol));
			if (strstr(stream_news->in_stream_addr, "rtsp"))
			{
				stream_news->status  = 1;
				memcpy(stream_news->protocol, "rtsp", 4);
			}
			else if (strstr(stream_news->in_stream_addr, "rtmp"))
			{
				stream_news->status  = 1;
				memcpy(stream_news->protocol, "rtmp", 4);
			}
			else
		        {	
				stream_news->status  = 5;//need get rtmp addr,and start push stream
				memcpy(stream_news->protocol, "http", 4);
			}

                        printf("stream_news->protocol = %s\n", stream_news->protocol);
			psub1 = cJSON_GetObjectItem(psub, "out_stream_addr");
			memset(stream_news->out_stream_addr, 0, sizeof(stream_news->out_stream_addr));
			memcpy(stream_news->out_stream_addr, psub1->valuestring, strlen(psub1->valuestring));

			pthread_mutex_lock(&infomtx);
			stream_news->restart = 0;
			list_add_tail(&stream_news->list, &task_list);
			pthread_cond_signal(&infocond);
			pthread_mutex_unlock(&infomtx);
		}
		else
		{
			struct stream_info *info  = NULL;
			struct stream_info *_info = NULL;

			pthread_mutex_lock(&infomtx);
			psub1 = cJSON_GetObjectItem(psub, "stream_id");
			list_for_each_entry_safe(info, _info, &task_list, list)
			{
				if (strcmp(info->stream_id, psub1->valuestring) == 0)
					info->status = 0;
			}
			pthread_mutex_unlock(&infomtx);
		}

	}

    	return len;
}

int get_rtmp(char* url, char *data)
{
	char p_data[1024] = {0};
        CURL *curl;
        CURLcode res;

	sprintf(p_data, RTMP_P, data);
        printf("p_data = %s\n", p_data);
        curl = curl_easy_init();
        if (curl)
        {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, p_data);
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback2);
                res = curl_easy_perform(curl);
        }

	curl_easy_cleanup(curl);  

        return 0;
}

int get_token(char* url, char *data)
{
        CURL *curl;
        CURLcode res;

        curl = curl_easy_init();
        if (curl)
        {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "appKey=d52fa532e89a4499923be9eed0d5d7fb&appSecret=bdada0a0f0c445c8e8f5f81e559ecb53");
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback1);
                res = curl_easy_perform(curl);
        }

	curl_easy_cleanup(curl);  

        return 0;
}

int get_camera_info(char* url)
{
        CURL *curl;
        CURLcode res;

        curl = curl_easy_init();
        if (curl)
        {
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
                res = curl_easy_perform(curl);
        }

	curl_easy_cleanup(curl);  

        return 0;
}

int get_camera_status(char *path, char *stream_id)
{
        struct stat buf;
        int  result = 0;
        char file[255] = {0};

        sprintf(file, "%s/%s.log", path, stream_id);
        if (access(file, F_OK) == -1)
        {
               dzlog_info("file %s is not exist", path);
               return 1;
        }

        result = stat(file, &buf);
        if ( result != 0 )
        {
                return 1;
        }
        else
        {
                long long now_time = _gettime_s();
                if (now_time - buf.st_atime < 30)
                        return 1;
                int dif_time = _gettime_s() - buf.st_mtime;
                if (dif_time < 5)
                        return 1;
                else
                        return 2;
        }
}

static void mg_reply_messge(struct mg_connection *c, char *data)
{
        char reply[1024*10] = {0};
        /* Send the reply */
        snprintf(reply, sizeof(reply),  "{\"result\": \"%s\"}", data);
        mg_printf(c, "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "%s",
                (int) strlen(reply), reply);
}

static void ev_handler(struct mg_connection *conn, int ev, void *p)
{
        if (ev == MG_EV_HTTP_REQUEST)
        {
                struct http_message *hm = (struct http_message *) p;

                if (!strncmp(hm->uri.p, API_CSTART, strlen(API_CSTART)))
		{
                        return;

		}
                else if (!strncmp(hm->uri.p, API_CSTOP, strlen(API_CSTOP)))
		{
                        dzlog_info("---- stop camero ----");
                        char var_name[LEN_PARAM]  = {0};
                        char file_name[LEN_PARAM] = {0};
                        const char *chunk = NULL;
                        size_t chunk_len = 0;
                        size_t n1 = 0;
                        size_t n2 = 0;

			char stream_id[LEN_PARAM] = {0};

                        memcpy(stream_id, "10001", 5);

/*
                        while ((n2 = mg_parse_multipart(hm->body.p + n1, hm->body.len - n1, var_name,
                                sizeof(var_name),file_name, sizeof(file_name), &chunk, &chunk_len)) > 0)
                        {
                        	if (!strcmp(var_name, "camera_id"))
				{
					memcpy(stream_id, chunk, chunk_len);
					stream_id[chunk_len] = '\0';
					printf("camera_id = %s\n", stream_id);
					break;
				}
                                n1 += n2;
                        }
*/
			struct stream_info *info  = NULL;
			struct stream_info *_info = NULL;

			pthread_mutex_lock(&infomtx);
			list_for_each_entry_safe(info, _info, &task_list, list)    
			{
				if (strcmp(info->stream_id, stream_id) == 0)
					info->status = 0;
			}
			pthread_mutex_unlock(&infomtx);

                        mg_reply_messge(conn, "OK");
                        return;
		}
                else if (!strncmp(hm->uri.p, API_CSTATUS, strlen(API_CSTATUS)))
                {
                        dzlog_info("---- get camero status ----");
                        cJSON *root;
                        /* Here we construct some JSON standards, from the JSON site. */
                        root=cJSON_CreateObject();

                        mg_reply_messge(conn, "ok");

                        return;

                }
                else if (!strncmp(hm->uri.p, API_CHANGE_CONFIG, strlen(API_CHANGE_CONFIG)))
                {
                        dzlog_info("---- change camero config ----");
                        dzlog_debug("ev_handler change config hm->body.p = %s", hm->body.p);

                        mg_reply_messge(conn, "OK");
                        return;

                }
        }
        return;
}

static void *poll_fun(void *mgr)
{
        for (;;)
        {
                mg_mgr_poll((struct mg_mgr *) mgr, 3000);
        }
        mg_mgr_free(mgr);

        return NULL;
}

void sig_ignore()
{
        sigset_t sigSetMask;
        sigfillset(&sigSetMask);
        sigdelset(&sigSetMask, SIGUSR1);
        sigdelset(&sigSetMask, SIGTERM);
        sigdelset(&sigSetMask, SIGINT);
        sigprocmask(SIG_SETMASK, &sigSetMask, 0);
}

void damon()
{
        int ret = fork();
        if(ret)
        {
                exit(0);
        }

        sig_ignore();
        setsid();
        int fd = open("/dev/null", O_RDWR);
        if (fd)
        {
                dup2(fd, 0);
                dup2(fd, 1);
                dup2(fd, 2);
                close(fd);
        }

        while (1)
        {
                pid_t pid = fork();
                if(pid)
                {
                        waitpid(pid, NULL, 0);
                }
                else
                        break;
        }

}

int executesql(MYSQL *g_conn, const char * sql) 
{
    	/*query the database according the sql*/
    	if (mysql_real_query(g_conn, sql, strlen(sql))) // 如果失败
        	return -1; // 表示失败

    	return 0; // 成功执行
}

int get_camera()
{
	while (1)
	{
		char url[256] = {0};
		sprintf(url, "http://%s:%s/getcamera?server_id=%s&&ip=%s", _config.server_ip, _config.server_port, _config.server_id, local_ip);
		printf("url = %s\n", url);
        	get_camera_info(url);
		sleep(30);
	}
	return 0;
}

int run_mediainfo(char *argv, int  *time_len)
{
        FILE *fp = NULL;
        char line[1024] = {0};

        fp=popen(argv, "r");
        if (fp == NULL)
        {
                dzlog_info("run_command error\n");
                return -1;
        }
        else
        {
                while(fgets(line, 1024, fp) != NULL)
                {
			printf("line = %s\n", line);
                }

                pclose(fp);
        }

        return 0;
}

int main(int argc, char **argv) 
{
	struct sigaction sa;
        pthread_t sid;
        pthread_t cid;

        int optchar   = 0;	
	int task_nums = 0;

        struct mg_mgr mgr;
        struct mg_connection *nc;
	/** Command line parsing */
	while(1)
	{
		int option_index = 0;

		static struct option long_options[] =
		{	
			{"edition",             1, 0, 's'},
			{"debug",               0, 0, 'd'},
			{"help",                0, 0, '?'},
			{0, 0, 0, 0}
		};

		optchar = getopt_long (argc, argv, "s:v?", long_options, &option_index);
		if (optchar == -1)
			break;

		switch(optchar)
		{
			case 's':
				edition = optarg;
				break;
			case 'v':
			case '?': 
				fprintf(stdout, "Gomeipc version:%s\n", VERSION);
				return 1;
		}
	}

	get_local_ip();
	if (isvalid_ip(local_ip))
	{
		printf("get local ip success == %s\n", local_ip);
	}
	else
	{
		printf("get local ip failed %s\n", local_ip);
		return -1;
	}

        configure(&_config);
	server_id = _config.server_id;
	if (server_id == NULL)
	{
		fprintf(stderr, "please input server id.\n");
		exit(0);
	}

	damon();
	int rc = dzlog_init("log.conf", "stream_to_rtmp");    
	if (rc) 
	{        
		fprintf(stderr, "dzlog init error\n");        
		return 1;    
	}    
	dzlog_info("starting...");

	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0 );

	INIT_LIST_HEAD(&task_list);

	pthread_create(&cid, NULL, get_camera, NULL);

	while(1)
        {
		pthread_mutex_lock(&infomtx);
		list_entry_count(&task_list, task_nums);
		
		if(task_nums >= 1)
		{
			struct stream_info *info  = NULL;
			struct stream_info *_info = NULL;

			list_for_each_entry_safe(info, _info, &task_list, list)    
			{        
				dzlog_info("stream_id:%s status = %d", info->stream_id, info->status);
				if (info->status == 1 || info->status == 2)
				{
					int camera_status = 1;
					camera_status = get_camera_status(_config.log_path, info->stream_id);
					report_camera_status(info->stream_id, camera_status);
					if (camera_status == 2)
					{
						dzlog_info("camera:%s publish stream failed", info->stream_id);
					}
				}

				if(info->status == 1) 
				{
					dzlog_info("stream_id:%s start publish stream", info->stream_id);
					//if (strstr(info->in_stream_addr, "rtsp") && strstr(info->out_stream_addr, "rtmp"))
					if (1)
					{
						//pthread_create(&sid, NULL, start_transcoder, (void *)info);
						pthread_create(&sid, NULL, start_gome_transcoder, (void *)info);
				 		pthread_detach(sid);
					}
					info->thread_id = sid;
					info->status = 2;	
			        	pthread_mutex_lock(&threadsmtx);
        				thread_nums ++;
        				pthread_mutex_unlock(&threadsmtx);
				}
				else if (info->status == 0)
				{
					dzlog_info("stream_id:%s stop publish stream", info->stream_id);

					char arg[64] = {0};
					sprintf(arg, "%s %s", _config.kill_sh, info->in_stream_addr);
					printf ("arg ====== %s\n", arg);
					system(arg);

				        list_del(&info->list);            
					info->status = -1;

					if (info)
					{
						free(info);
						info = NULL;
					}
				}
				else if (info->status == 5)
				{
				        get_token(GET_TOKEN, NULL);	
				}

				if (info->status == 2 && info->restart == 1)
				{
					dzlog_info("stream_id:%s restart stream", info->stream_id);

					char arg[64] = {0};
					sprintf(arg, "%s %s", _config.kill_sh, info->in_stream_addr);
					printf ("arg ====== %s\n", arg);
					system(arg);
					info->restart = 0;
				}

			}
			pthread_mutex_unlock(&infomtx);    
                	sleep(5);

			dzlog_info("-------------------------------------end-----------------------------------------");

		}
                else
		{
			pthread_mutex_unlock(&infomtx);    
                	sleep(3);
		}
        }

	return 0;

}
