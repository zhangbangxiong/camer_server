/*
 * twemproxy - A fast and lightweight proxy for memcached protocol.
 * Copyright (C) 2011 Twitter, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
#include <sys/stat.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <pthread.h>

#include "util.h"
#include "cjson.h"
#include "config.h"
#include "md5.h"
#include "zlog.h"
#include "libavformat/avformat.h"

#define TAG ","

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

		for (i = 0; i < 7; i++)
		{
			int j = 1;
			for(j = 1; j <= 7; j++)
				if (j >= fir[i] && j <= sec[i])
					winfo[j - 1] = 1;
		}

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

long long _gettime_s(void)       //unit: s
{
    struct timeval tv;

    gettimeofday(&tv,NULL);
    signed long long ts = (signed long long)tv.tv_sec;
    return ts;
}

int get_hour()
{
	time_t timep;
	time(&timep);

	struct tm* local; //本地时间
        local = localtime(&timep); //转为本地时间

	return local->tm_hour;
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

time_t str_to_time(char *_t)
{
	int hour = 0;
	int min = 0;
	int sec = 0;

	struct tm tm_ = {0};
	time_t t_;

	struct tm *local_time;
	time_t lt;
	lt = time(NULL);
	local_time = localtime(&lt);

	sscanf(_t, "%d:%d:%d", &hour, &min, &sec); 

	tm_.tm_sec = sec;
	tm_.tm_min = min;
	tm_.tm_hour = hour;
	tm_.tm_mday = local_time->tm_mday;
	tm_.tm_mon  = local_time->tm_mon;
	tm_.tm_year = local_time->tm_year;

	tm_.tm_isdst = -1;
	t_ = mktime(&tm_); 

	tm_ = *localtime(&t_);

	
	return t_;
}

int url_encode_2(const char* src, const int srcsize, char* dst, int dstsize)
{
    int i;
    int j = 0;
    char ch;

    if ((src==NULL) || (dst==NULL) || (srcsize<=0) || (dstsize<=0))
    {
        return 0;
    }

    for (i=0; (i<srcsize)&&(j<dstsize); ++i)
    {
        ch = src[i];
        if (((ch>='A') && (ch<'Z')) ||
            ((ch>='a') && (ch<'z')) ||
            ((ch>='0') && (ch<'9'))) {
            dst[j++] = ch;
        } else if (ch == ' ') {
            dst[j++] = '+';
        } else if (ch == '.' || ch == '-' || ch == '_' || ch == '*') {
            dst[j++] = ch;
        } else {
            if (j+3 < dstsize) {
                sprintf(dst+j, "%%%02X", (unsigned char)ch);
                j += 3;
            } else {
                return 0;
            }
        }
    }

    dst[j] = '\0';
    return j;
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

int get_file_duration(const char *filename)
{
    int ret = 0;
    unsigned int i = 0;
    AVFormatContext *ifmt_ctx = NULL;

    av_register_all();

    if ((ret =
         avformat_open_input(&ifmt_ctx, filename, NULL, NULL)) < 0) {
        printf("Cannot open input file\n");
        return NULL;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        printf("Cannot find stream information\n");
        return NULL;
    }

    int duration = ifmt_ctx->duration/1000000;
    if (duration < 0)
        duration = 0;


    if (ifmt_ctx)
    {
        avformat_free_context(ifmt_ctx);
    }
    return duration;
}


