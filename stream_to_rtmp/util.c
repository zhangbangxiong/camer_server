#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <pthread.h>
#include "util.h"
#include "cjson.h"
#include "config.h"
#include "md5.h"
#include "getpath.h"
#include "http_post.h"
#include "copyfiles.h"
#include "fast.h"
#include "zlog.h"

/*
 *  * logo pisition 
 *  * 1-left_top, 2-right_top,3-right_bootom, 4-left_bootom
 *  */
#define TAG ","

extern int thread_nums;
extern config_t _config;
extern char local_ip[128];
extern char *edition;

extern int upload_pic(char *http, char *addr, int port, char* path, int id, char* image1, char* image2);

extern pthread_mutex_t infomtx;
extern pthread_cond_t  infocond;

extern pthread_mutex_t convertmtx;
extern pthread_cond_t  convertcond;

extern pthread_mutex_t threadsmtx;

char *strRemov(char* dst, const char* src, char ch)
{
	int i = -1, j = 0;
	while (src[++i])
		if (src[i] != ch)
			dst[j++] = src[i];
	dst[j] = '\0';
	return dst;
}

//1-4;5-6
int parse_week_info(char *week, int winfo[])
{
	int i = 0;
	int counts = 0;
	int fir[7] = { 0 };
	int sec[7] = { 0 };
	char new_week[32] = { 0 };
	strRemov(new_week, week, ' ');
	char *ptr = new_week;
	char *ptr_1 = strchr(ptr, ';');
	if (ptr_1 == NULL)
	{
		counts = 1;
		char *p = ptr;
		char fir_day[4] = { 0 };
		char sec_day[4] = { 0 };
		char *q = strchr(p, '-');
		if (q == NULL)
			return 1;
		memcpy(fir_day, p, 1);
		memcpy(sec_day, q + 1, 1);
		if (atoi(sec_day) < atoi(fir_day))
			return 1;
		fir[counts - 1] = atoi(fir_day);
		sec[counts - 1] = atoi(sec_day);
	}
	else
	{
		int out = 0;
		do
		{
			counts++;
			char fweek[4] = { 0 };
			memcpy(fweek, ptr, 3);
			char *p = fweek;
			//printf("p = %s\n", p);
			char fir_day[4] = { 0 };
			char sec_day[4] = { 0 };
			char *q = strchr(p, '-');
			if (q == NULL)
				return 1;
			memcpy(fir_day, p, 1);
			//printf("q = %s\n", q);
			memcpy(sec_day, q + 1, 1);
			//printf("fir_day = %s\n", fir_day);
			//printf("sec_day = %s\n", sec_day);
			if (atoi(sec_day) < atoi(fir_day))
				return 1;
			fir[counts - 1] = atoi(fir_day);
			sec[counts - 1] = atoi(sec_day);
			ptr = ptr_1 + 1;
			if (out)
				break;
			ptr_1 = strchr(ptr, ';');
			if (ptr_1 == NULL)
				out = 1;
		} while (1);

	}

	for (i = 0; i < 7; i++)
	{
		int j = 1;
		for(j = 1; j <= 7; j++)
			if (j >= fir[i] && j <= sec[i])
				winfo[j - 1] = 1;
	}

	return 0;
}

unsigned long get_file_size(const char *path)  
{  
    unsigned long filesize = -1;      
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
}  

long long _gettime_s()       //unit: s
{
    struct timeval tv;

    gettimeofday(&tv,NULL);
    signed long long ts = (signed long long)tv.tv_sec;
    return ts;
}

time_t _gettime_w(int *week)       //unit: s
{
	struct tm *local_time;
	time_t lt;
	lt = time(NULL);
	local_time = localtime(&lt);

	if (local_time->tm_wday == 0)
		*week = 7;
	else
		*week = local_time->tm_wday;

	return lt;
}

int insert(struct node *list, char *md5, int pts)
{
	struct node *head = list;
	struct node *add = NULL;

	add = (struct node*)malloc(sizeof(struct node));
	if(NULL == add)	return -1;
	add->next= NULL;

	while( NULL != head->next )
	{
		head =  head->next;
	}

	memcpy(add->md5, md5, MD5_LEN);
	add->pts = pts;
	head->next = add;

	return 0;
}

char *url_encode(char const *s, int len, int *new_len)
{
        register unsigned char c;
        unsigned char *to, *start;
        unsigned char const *from, *end;
        unsigned char hexchars[] = "0123456789ABCDEF";

	if(s == NULL || strlen(s) == 0)
		return NULL;

        from = (unsigned char *)s;
        end  = (unsigned char *)s + len;
        start = to = (unsigned char *) calloc(1, 3*len+1);

        while (from < end)
        {
                c = *from++;

                if (c == ' ')
                {
                        *to++ = '+';
                }
                else if ((c < '0' && c != '-' && c != '.') ||
                        (c < 'A' && c > '9') ||
                        (c > 'Z' && c < 'a' && c != '_') ||
                        (c > 'z'))
                {
                        to[0] = '%';
                        to[1] = hexchars[c >> 4];
                        to[2] = hexchars[c & 15];
                        to += 3;
                }
                else
                {
                        *to++ = c;
                }
        }
        *to = 0;

        if (new_len)   
        {  
        	*new_len = to - start;  
        }  
        return (char *) start;
}

int get_pts(struct node *list, char *md5)
{
	struct node *head = list;

	while(NULL != head->next)
	{
		head = head->next;
		if(0 == strncmp(md5, head->md5, MD5_LEN))
			return head->pts;
	}

	return -1;
}

int del_key(struct node *list, char *md5)
{
	struct node *head = list;
	struct node *p = NULL;	//init=NULL
	

	while(NULL != head->next)
	{
		if(0 == strncmp(md5, head->md5, MD5_LEN))
		{
			p = head->next;
			head->next = p->next;
			free(p);
			return 0;
		}
		head = head->next;
	}

	return 0;
}

/*
int send_info(char* url, char* data)
{
    FILE *fptr;   
    struct curl_slist *http_header = NULL;   
  
    if ((fptr = fopen(FILENAME, "w")) == NULL) 
    {   
        fprintf(stderr, "fopen file error: %s\n", FILENAME);   
        exit(1);   
    }   

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    printf("data = %s\n", data);

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);   
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);   
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);   
        curl_easy_setopt(curl, CURLOPT_POST, 1);   
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);   
        curl_easy_setopt(curl, CURLOPT_HEADER, 1);   
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);   
        res = curl_easy_perform(curl);
        printf("9999999998\n");
        if (res != CURLE_OK)
        {
            printf("curl_easy_perform() return:%d\n",res);
            return 1;
        }
    }

    curl_easy_cleanup(curl);
    printf("send done!\n");
    return 0;
}
*/
int send_data(char *data, char *url)
{
        char  arg[256] = {0};
        sprintf(arg, "curl %s %s", data, url);
        printf("\narg = %s\n", arg);
        system(arg);
        return 0;

}

int run_command(struct stream_info *info, char *argv, char *outfile, int *timelen, int *video_width, int *video_height)
{
	FILE *fp = NULL;
	char line[1024] = {0};
	
	dzlog_info("[%s] : %s", info->stream_id, argv);

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
			if(strstr(line, "codec_error") != NULL)
			{
				break;
			}
                }
		
                pclose(fp);
        }

	return 0;
}

int check_codec_log(char *path, int *error_no)
{
        char line[INPUT_BUFFER_SIZE] = {0};
        char *p = NULL;

        if(access(path, F_OK) == -1)
        {
               dzlog_info("file %s is not exist", path);
               return -1;
        }

        FILE* fd = fopen(path, "rb");
	
        while(fgets(line, INPUT_BUFFER_SIZE, fd) != NULL)
        {
		if(strstr(line, "codec_error") != NULL)
		{
			if((p = strchr(line, ':')) != NULL)
			{
				*error_no = atoi(p + 1);
			}
			break;
		}

	} 
	return 0;
	
}

#define CODEC_VIDEO_STRING_0  "%s %s %s %s %s %s %s %s %d %s %s %s %d %s"
#define CODEC_JPEG_STRING   "%s -i %s -o %s -n 20 -@ %s"
#define TSCUT_STRING        "%s -i %s -d 10 -o %s -s %s -d 10 -x %s"
void *start_gome_transcoder(void *params)
{
        char  argv[INPUT_BUFFER_SIZE]    = {0};
        char  outfile[BUFFER_SIZE]       = {0};
        char  outfullfile[NOMARL_BUFFER_SIZE]    = {0};

	int   video_time_lenth = 0;
	int   video_width      = 0;
	int   video_height     = 0;
	struct stream_info *info = NULL;

	if(params != NULL)
	{
		info = (struct stream_info *)params;
	}
	else
	{
		goto EXIT;
	}

	char log_path[256] = {0};
	sprintf(log_path, "%s/%s.log", _config.log_path,info->stream_id);
		
	memset(outfullfile, 0, NOMARL_BUFFER_SIZE);
	memset(outfile, 0, BUFFER_SIZE);
	memset(argv, 0, INPUT_BUFFER_SIZE);

        snprintf(argv, INPUT_BUFFER_SIZE, CODEC_VIDEO_STRING_0, 
		_config.sh, 		_config.ffmpeg, info->in_stream_addr, 
		info->protocol, 	info->encode, 	info->bitrate,
		info->width, 		info->height, 	0, 
		info->out_stream_addr,	_config.logo_path, _config.log_path, 
		_config.logo_pos,       info->stream_id);

	dzlog_info("[%s]----- argv = %s", info->stream_id, argv);

	/*********************************************************
 			start codec
	********************************************************/
	run_command(info, argv, outfile, &video_time_lenth, &video_width, &video_height);

EXIT:
	dzlog_info("[%s] : thread exit ---", info->stream_id);
	info->status = 1;
	pthread_mutex_lock(&threadsmtx);
	thread_nums --;
        pthread_mutex_unlock(&threadsmtx);
	pthread_exit(0);  
}


