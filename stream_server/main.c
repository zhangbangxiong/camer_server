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
#include <sys/wait.h>
#include <time.h>

#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "util.h"
#include "smp_md5.h"
#include "cpumem.h"
#include "md5.h"
#include "config.h"
#include "cjson.h"
#include "mongoose.h"
#include "zlog.h"
#include "http_post.h"

#define API_CSTATUS 		"/cstatus"
#define API_CHANGE_CONFIG       "/cconfig"
#define API_VOD_M3U8            "/getm3u8"
#define API_RECORD              "/recorded"
#define API_GETCAMERA           "/getcamera"
#define API_GETSAVETIME         "/getsavetime"

#define FILE_NAME_LENGHT 10
#define MG_FALSE      1
#define MG_TRUE       0
#define FILE_STREAM_LEN 60      //SECONDS

#define MAX_FILE_NUM    600
#define TIME_STR        12
#define TIME_SECOND     12
#define TIME_HOUR       8
#define TIME_MINITE     10

typedef struct file_info{
	int  sequenceno;	
	int  fileno;
};

static volatile char STOP = 0;
static volatile char STOP_OK = 0;

char local_ip[128] = "0.0.0.0";
config_t  _config;

char * record = "/home/record";

int get_local_ip()
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
			if(strcmp(ifAddrStruct->ifa_name, "ens192") == 0)
				memcpy(local_ip, addressBuffer, strlen(addressBuffer));
		} 
		else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6) 
		{ 
			// check it is IP6
			// is a valid IP6 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			if(strcmp(ifAddrStruct->ifa_name, "ens192") == 0)
				memcpy(local_ip, addressBuffer, strlen(addressBuffer));
		} 

		ifAddrStruct=ifAddrStruct->ifa_next;
	}

 	local_ip[strlen(local_ip)] = '\0';
	printf("local_ip = %s\n", local_ip);

/*
    char hname[128];

    struct hostent *hent = NULL;
    int i;

    gethostname(hname, sizeof(hname));

    hent = gethostbyname(hname);

    for (i = 0; hent->h_addr_list[i]; i++) 
    {
	int len = strlen(inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])));
	if (len > 0)
		memcpy(local_ip, inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])), len);
        printf("%s\n", inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])), i);
    }
*/
    return 0;
}

int get_camero_status(char *path, char *stream_id)
{
        struct stat buf;
        int  result = 0;
	char file[255] = {0};

	sprintf(file, "%s/%s.log", path, stream_id);
	if (access(file, F_OK) == -1)
        {
               dzlog_info("file %s is not exist", path);
               return -1;
        }

        result = stat(file, &buf);
        if ( result != 0 )
	{
		return -2;
	}
        else
        {
		long long now_time = _gettime_s();	
		if (now_time - buf.st_atime < 30)
			return 0;
                int dif_time = _gettime_s() - buf.st_mtime;
		if (dif_time < 5)
			return 1;
		else
			return 0;
        }
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
                        if (line != NULL && strlen(line) != 0)
                        {
                                char *p = strrchr(line, ':');
                                char *q = strrchr(line, '.');
                                char csec[8] = {0};
                                char msec[8] = {0};
                                memcpy(csec, p + 1, q - p - 1);
                                memcpy(msec, q + 1, strlen(line) -(q - line));
                                *time_len = atoi(csec) * 1000 + atoi(msec);
                        }
                }

                pclose(fp);
        }

        return 0;
}

static int customTs(const struct dirent *pDir)
{
    if ((NULL != strstr(pDir->d_name, "ts")))
        return 1;

    return 0;
}

static int recorddir(const struct dirent *pDir)                                                                                                                                  
{
    //printf("d_type = %d %s\n", pDir->d_type, pDir->d_name);
    //if ((NULL != strstr(pDir->d_name, "2018")))
    if (pDir->d_type != 8 && (strstr(pDir->d_name, ".") == NULL && strstr(pDir->d_name, "..") == NULL))
        return 1;

    return 0;
}

void sort(struct file_info a[], int len)
{
	int i = 0;
	int j = 0;
	for(i = 0; i < len - 1; i++)
        {
            for(j = 0; j < len - 1 -i; j++)// j开始等于0，
            {
                if(a[j].fileno > a[j + 1].fileno)
                {
		    int temp = 0;

                    temp    = a[j].fileno;
                    a[j].fileno = a[j + 1].fileno;
                    a[j + 1].fileno = temp;

                    temp    = a[j].sequenceno;
                    a[j].sequenceno = a[j + 1].sequenceno;
                    a[j + 1].sequenceno = temp;
                }
            }
        }

}

static int get_m3u8_from_other_server(char *id,  char *start, char *end, char *result)
{
	printf("get_m3u8_from_other_server id = %s start = %s\n", id, start);
	int i = 0;
	int r = 0;
	for (i = 0;i<_config.vod_server_nums;i++)
	{
		char data[64] = {0};
		char res[512] = {0};
		sprintf(data, "channel_id=%s&start_time=%s&end_time=%s", id, start, end);
		r = send_msg(_config.vs[i].ip, _config.vs[i].port, data, res);
		if (r > 0 && strstr(res, "m3u8"))
		{
			memcpy(result, res, strlen(res));
			r = 100;
			break;
		}
	}

	return r;
}

static int m3u8_get_files(char *path, char *id,  char *start, char *end, char *m3u8)
{
	struct file_info g_file_info[MAX_FILE_NUM];
	char g_file_list[MAX_FILE_NUM][64];
        int i = 0;
        int j = 0;
        int n = 0;
        int num = 0;
        int sum = 0;
        int num_files = 0;
	int start_file_num = 0;
	int end_file_num = 0;
        char file[128] = {0};
        struct dirent **namelist = NULL;
        struct dirent **namesub  = NULL;

	char channel_path[128] = {0};
   	char m3u8_file[512]    = {0};    
	char tmp[512]          = {0};
	char dir[256]          = {0};

	char start_hour[64]    = {0};
	char end_hour[64]      = {0};

	printf("start = %s\n", start);
        struct tm* local; //本地时间   
	int64_t s = atoi(start);
	printf("s = %lld", s);
	int64_t e = atoi(end);
        local = localtime(&s); //转为本地时间
	sprintf(start_hour, "%d%02d%02d%02d", local->tm_year+1900, local->tm_mon+1, local->tm_mday, local->tm_hour);
	int sh = atoi(start_hour);
	printf("sh = %d\n", sh);
        local = localtime(&e); //转为本地时间
	sprintf(end_hour, "%d%02d%02d%02d", local->tm_year+1900, local->tm_mon+1, local->tm_mday, local->tm_hour);
	int eh = atoi(end_hour);
	printf("eh = %d\n", sh);

	printf("%s %s\n", start_hour, end_hour);
	sprintf(channel_path,"%s/%s", path, id);
	snprintf(m3u8_file, 512, "%s/%s_%s.m3u8", channel_path, start, end);    
	if (access(m3u8_file, F_OK) == 0)
	{
		printf("has this m3u8 file %s\n", m3u8_file);
		sprintf(m3u8, "http://%s:%d/live/%s/%s_%s.m3u8", _config.vod_server_ip, _config.vod_m3u8_port, id, start, end);
		printf("00 start_file_num = %d, sum = %d %s\n", start_file_num, sum, m3u8);
		return 1;
	}

	sprintf(channel_path,"%s/%s", path, id);
        num = scandir(channel_path, &namelist, recorddir, alphasort);
	printf("num  = %d\n", num);
        if (num < 0)
        {
                return -1;
        }

	for (j = 0; j < num; j++)
	{
		if (atoi(namelist[j]->d_name) < sh || atoi(namelist[j]->d_name) > eh)
			continue;
		n = sprintf(dir, "%s/%s", channel_path, namelist[j]->d_name);
		dir[n] = '\0';
		num_files = scandir(dir, &namesub, customTs, alphasort);
		if (num_files < 0)
		{
			continue;
		}
		for(i = 0; i < num_files; i++)
		{
			struct stat buf;
			int  result = 0;

			n = sprintf(file, "%s/%s", dir, namesub[i]->d_name);
			result = stat(file, &buf);
			if ( result != 0 )
			{
				return -2;
			}
			else
			{
				if (buf.st_mtime >= atoi(start) && buf.st_mtime <= atoi(end))
				{
					char tmp[8] = {0};
                                	memcpy(tmp, namesub[i]->d_name, strchr(namesub[i]->d_name, '.') - namesub[i]->d_name);
					g_file_info[sum].fileno = atoi(tmp);
					g_file_info[sum].sequenceno = sum;
					memset(g_file_list[sum], 0, 64);
					sprintf(g_file_list[sum], "%s/%s", namelist[j]->d_name, namesub[i]->d_name);
					sum ++;
					if (sum >= MAX_FILE_NUM)
						break;
				}
			}
		}

		while (num_files--)
		{
			if (namesub[num_files])
				free(namesub[num_files]);
		}    
		if (namesub)
			free(namesub);
	}

        while (num--)                                                                                                                                                            
        {    
		if (namelist[num])
                	free(namelist[num]);
        }    
	if (namelist)
        	free(namelist);

	sort(g_file_info, sum);

	FILE *fp = NULL;    
	snprintf(m3u8_file, 512, "%s/%s_%s.m3u8", channel_path, start, end);    
	fp = fopen(m3u8_file, "wb");    
	snprintf(tmp, sizeof(tmp),  
			"#EXTM3U\n"                     
			"#EXT-X-VERSION:3\n"                     
			"#EXT-X-TARGETDURATION:10\n");    
	fwrite(tmp, strlen(tmp), 1, fp);    
	for (i = 0; i < sum;i++)
	{        
		memset(tmp, 0, sizeof(tmp));        
		snprintf(tmp, sizeof(tmp),"#EXT-X-DISCONTINUITY\n#EXTINF:%.3f,\n%s\n", 10.0, g_file_list[g_file_info[i].sequenceno]);                    
		fwrite(tmp, strlen(tmp), 1, fp);    
	}    
	memset(tmp, 0, sizeof(tmp));    
	snprintf(tmp, sizeof(tmp), "#EXT-X-ENDLIST\n");    
	fwrite(tmp, strlen(tmp), 1, fp);    
	fclose(fp);
	sprintf(m3u8, "http://%s:%d/live/%s/%s_%s.m3u8", _config.vod_server_ip, _config.vod_m3u8_port, id, start, end);

        return sum;
}

static void mg_reply_messge(struct mg_connection *c, char *data)
{
	char reply[1024*10] = {0};
	/* Send the reply */
	snprintf(reply, sizeof(reply),  "%s", data);
	mg_printf(c, "HTTP/1.1 200 OK\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: %d\r\n"
		"\r\n"
		"%s",
		(int) strlen(reply), reply);
}

int send_data(char *url, char *data)
{
        char  arg[256] = {0};
        sprintf(arg, "curl -l -H \"Content-type: application/json\" -X POST -d \'%s\' %s", data, url);
        printf("arg = %s\n", arg);
        system(arg);
        return 0;
}

static int report_file_info(char *filepath, char *camera_id)
{
        char curl[64] = {0};//"http://192.168.102.248:80/api/record/create";
	char durl[256] = {0};
        char _data[256] = {0};
        char edata[512] = {0};

        struct stat buf;
        int  result = 0;
        long long start_time = 0;
        long long end_time   = 0;

        char tmp[512] = {0};
        char filename[64] = {0};

        memcpy(tmp, filepath, strlen(filepath));
        printf("00 tmp = %s\n", tmp);
        if (tmp[strlen(tmp) - 1] == '/')
        {
               tmp[strlen(tmp) - 1] = '\0';
        }
        printf("01 tmp = %s\n", tmp);

        char *q = strrchr(tmp, '/');
        if (q != NULL)
        {
               memcpy(filename, q + 1, strlen(tmp) - (q - tmp));
        }

        printf("02 tmp = %s\n", tmp);
        if (access(filepath, F_OK) == -1)
        {
               dzlog_info("file %s is not exist", filepath);
               return -1;
        }

	sprintf(curl, "http://%s:%d/api/record/create", _config.stream_server_ip, _config.stream_server_http_port);
	char tmp_time[64] = {0};
	char *p = strchr(filename, '_'); 
	if (p == NULL)
		return -1;
	q = strchr(filename, '.'); 
	if (q == NULL)
		return -1;
	memcpy(tmp_time, p + 1, q - p - 1);
	printf("tmp_time = %s\n", tmp_time);
        struct tm tm;
        strptime(tmp_time, "%Y-%m-%d-%H-%M-%S", &tm);
        time_t pp = mktime(&tm);
	printf("pp = %d\n", (int)pp);

        result = stat(filepath, &buf);
        if ( result != 0 )
        {
                return -2;
        }
        else if (buf.st_size > 0)
        {
                dzlog_info("file name    = %s",filename);
                dzlog_info("camera_id    = %s",camera_id);
		int duration = get_file_duration(filepath);
                start_time = pp;
		if (duration > 0)
			end_time = start_time + duration;
		else
                	end_time = buf.st_mtime;
		int id = atoi(camera_id) - 10000;
 		if (id > 0 && start_time > 0 && end_time > 0 && (end_time - start_time) > 120)
		{
			sprintf(durl, "http://%s:%d/%s", _config.store_server_ip, _config.store_download_port, filename);
                	sprintf(_data, "{\"camera_id\":\"%d\",\"start_time\":%lld,\"end_time\":%lld,\"download_url\":\"%s\"}", id, start_time, end_time, durl);
                	url_encode_2(_data, strlen(_data), edata, 1024);
                	dzlog_info("_data = %s", _data);
                	send_data(curl, _data);
		}
	}

	return 0;
}

static void ev_handler(struct mg_connection *conn, int ev, void *p) 
{
	if (ev == MG_EV_HTTP_REQUEST) 
	{
    		struct http_message *hm = (struct http_message *) p;
		
		if (!strncmp(hm->uri.p, API_CSTATUS, strlen(API_CSTATUS)))		
		{
			dzlog_info("---- get camero status ----");
			
			return;

		}
		else if (!strncmp(hm->uri.p, API_CHANGE_CONFIG, strlen(API_CHANGE_CONFIG)))		
		{
			dzlog_info("---- change camero config ----");
			return;

		}
		else if (!strncmp(hm->uri.p, API_VOD_M3U8, strlen(API_VOD_M3U8)))		
		{
			dzlog_info("---- get vod m3u8 ----");
			char msg[1280]  = {0};	
                	int  result 	= 0;
                	char id[24] 	= {0};
                	char start[24] 	= {0};
                	char end[24] 	= {0};
			char m3u8[1024] = {0};

                      	mg_get_http_var(&hm->query_string, "channel_id", id, 24);
                      	mg_get_http_var(&hm->query_string, "start_time", start, 24);
                      	mg_get_http_var(&hm->query_string, "end_time", end, 24);
			dzlog_info("channel_id = %s", id);
			dzlog_info("start      = %s", start);
			dzlog_info("end        = %s", end);

			if (strlen(id) == 0 || strlen(end) == 0 || strlen(start) == 0)
			{
				dzlog_info("failed");
				mg_reply_messge(conn, "failed");
				return;
			}

                	result = m3u8_get_files(_config.m3u8_path, id, start, end, m3u8);
			printf("m3u8_get_files res = %d\n", result);
                	if(result <= 0)
                	{
				int res = get_m3u8_from_other_server(id, start, end, msg);
				if (res == 100)
				{
					printf("m3u8_get_files msg = %s\n", msg);
					mg_reply_messge(conn, msg);
				}
				else
				{
					dzlog_info("vod m3u8 failed");
					sprintf(msg, "{\"result\": failed}");
					mg_reply_messge(conn, "failed");
				}
                	}	
			else
			{
				dzlog_info("m3u8 = %s", m3u8);
				sprintf(msg, "{\"result\": \"%s\"}", m3u8);
				mg_reply_messge(conn, msg);
			}

		}
		else if (!strncmp(hm->uri.p, API_RECORD, strlen(API_RECORD)))		
		{
			dzlog_info("---- report record ----");
                	char path[512] = {0};
			char name[256] = {0};

                      	mg_get_http_var(&hm->body, "path", path, 512);
                      	mg_get_http_var(&hm->body, "name", name, 256);
			dzlog_info("path = %s\n", path);
			report_file_info(path, name);
		}
		else if (!strncmp(hm->uri.p, API_GETCAMERA, strlen(API_RECORD)))		
		{
			dzlog_info("---- get camera ----");
			char server_id[32] = {0};
			char server_ip[32] = {0};
                      	mg_get_http_var(&hm->query_string, "server_id", server_id, 32);
                      	//mg_get_http_var(&hm->query_string, "ip", server_ip, 32);
			printf("id = %s\n", server_id);
			//printf("ip = %s\n", server_ip);
			//if (atoi(server_id) <= 0 || strlen(server_id) == 0 || strlen(server_ip) < 7)
			if (atoi(server_id) <= 0 || strlen(server_id) == 0)
			{
				mg_reply_messge(conn, "failed");
				return;
			}

			char *msg = NULL;
                       	msg = read_mysql(atoi(server_id), server_ip);

			if (msg == NULL)
			{
				mg_reply_messge(conn, "failed");
				free(msg);
			}
			else
				mg_reply_messge(conn, msg);
		}
		else if (!strncmp(hm->uri.p, API_GETSAVETIME, strlen(API_RECORD)))		
		{
			char msg[32] = {0};
                        int _time = read_time_mysql();
			sprintf(msg, "%d", _time);
			if (msg == NULL)
				mg_reply_messge(conn, "failed");
			else
				mg_reply_messge(conn, msg);
		}
	} 

	return;

}

static void *poll_fun(void *mgr)
{
	for (;;)
        {
        	//dzlog_debug("in poll fun  ######################");
                mg_mgr_poll((struct mg_mgr *) mgr, 3000);
        }
        dzlog_debug("poll fun exit ######################");
        mg_mgr_free(mgr);

        return NULL;
}

int thread_nums = 0;

void sig_ignore()
{    
	sigset_t sigSetMask;    
	sigfillset(&sigSetMask);    
	sigdelset(&sigSetMask, SIGUSR1);    
	sigdelset(&sigSetMask, SIGTERM);    
	sigdelset(&sigSetMask, SIGINT);    
	sigprocmask(SIG_SETMASK, &sigSetMask, 0);
}

void create_daemon(void)
{
    dzlog_info("damon ....");
    int i = 0, cnt = 0;
    pid_t pid = -1;
    
    pid = fork();
    if ( -1 == pid)
    {
        perror("fork");
        _exit(-1);
    }
    if ( pid > 0 )
    {
        _exit(0);
    }
    
    pid = setsid();
    if ( -1 == pid )
    {
        perror("setsid");
        _exit(-1);
    }
    
    chdir("/");
    
    umask(0);
    
    cnt = sysconf(_SC_OPEN_MAX);
    for ( i = 0; i < cnt; i++ )
    {
        close(i);
    }
    
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
}

void damon()
{    
	dzlog_info("damon ....");
	int ret = fork();
	dzlog_info("damon ret .... %d", ret);
	if(ret)        
	{
		dzlog_debug("parent proc %d", getpid());
		exit(0);
	}    


	dzlog_debug("child proc %d", getpid());
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
		dzlog_debug("%d............", getpid());
		if(pid)        
		{            
			waitpid(pid, NULL, 0);        
		}        
		else            
			break;    
	}

}

int main(int argc, char **argv) 
{
	struct sigaction sa;
        int rc = 0;    
	rc = dzlog_init("server.conf", "stream_server");    
	if (rc) 
	{        
		fprintf(stderr, "dzlog init error\n");        
		return 1;    
	}    
	dzlog_info("starting...");

	damon();
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0 );

        configure(&_config);

  	struct mg_mgr mgr;
  	struct mg_connection *nc;

  	dzlog_debug("Starting multi-threaded server on port %s\n", _config.server_port);
  	mg_mgr_init(&mgr, NULL);
  	nc = mg_bind(&mgr, _config.server_port, ev_handler);
  	mg_set_protocol_http_websocket(nc);

  	/* For each new connection, execute ev_handler in a separate thread */
  	mg_enable_multithreading(nc);

	mg_start_thread(poll_fun, &mgr);

	while(1)
        {
		sleep(2);
        }

	return 0;

}
