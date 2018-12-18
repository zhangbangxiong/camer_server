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
#include <mysql/mysql.h>

#include "curl/curl.h"
#include "util.h"
#include "cjson.h"
#include "config.h"
#include "md5.h"
#include "getpath.h"
#include "http_post.h"
#include "copyfiles.h"
#include "zlog.h"
#include "libavformat/avformat.h"

#define TAG ","

extern config_t _config;

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

time_t str_to_time_1(char *_t)
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

static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int len = size * nmemb;
    printf("ptr = %s\n", ptr);
    return len;
}

int sendinfo(char* url, char* data)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *slist = 0;
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
        curl_slist_free_all(slist);
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

int executesql(MYSQL *g_conn, const char * sql) 
{
        /*query the database according the sql*/
        if (mysql_real_query(g_conn, sql, strlen(sql))) // 如果失败
                return -1; // 表示失败

        return 0; // 成功执行
}

int read_time_mysql()
{
        MYSQL *g_conn; 
        MYSQL_ROW g_row; 
        MYSQL_RES *g_res;
        g_conn = mysql_init(NULL);

        // connect the database
        printf("ip = %s\n", _config.mysql_ip);
        printf("user = %s\n", _config.mysql_user);
        if(!mysql_real_connect(g_conn, _config.mysql_ip, _config.mysql_user, _config.mysql_passwd, "bpls", _config.mysql_port, NULL, 0))
        {
                printf("mysql connect failed\n");
        	mysql_close(g_conn); // 关闭链接
                return -1;
        }

	char select_sql[64] = {0};
	sprintf(select_sql, "select save_time from bpls_record_config");
	if (executesql(g_conn, select_sql))
	{
		printf("select error\n");
        	mysql_close(g_conn); // 关闭链接
		return -1;
	}

	g_res = mysql_store_result(g_conn); 

	int iNum_rows = mysql_num_rows(g_res);
	int iNum_fields = mysql_num_fields(g_res);

	g_row=mysql_fetch_row(g_res);

	mysql_free_result(g_res); // 释放结果集
        mysql_close(g_conn); // 关闭链接
	printf("read time frome mysql g_row = %s\n", g_row[0]);
	return atoi(g_row[0]);
}

int mysql_ishave_data(int server_id, const char *ip)
{
        MYSQL *g_conn; 
        MYSQL_ROW g_row; 
        MYSQL_RES *g_res;
        g_conn = mysql_init(NULL);

        // connect the database
        if(!mysql_real_connect(g_conn, _config.mysql_ip, _config.mysql_user, _config.mysql_passwd, "bpls", _config.mysql_port, NULL, 0))
        {
                printf("mysql connect failed\n");
        	mysql_close(g_conn); // 关闭链接
                return -1;
        }

	char select_sql[64] = {0};
	sprintf(select_sql, "select * from bpls_stream_machine where stream_machine_id=%d and ip=\"%s\"", server_id, ip);
	if (executesql(g_conn, select_sql))
	{
		printf("select error\n");
        	mysql_close(g_conn); // 关闭链接
		return -1;
	}

	g_res = mysql_store_result(g_conn); 

	int res = mysql_num_rows(g_res);

	mysql_free_result(g_res); // 释放结果集
        mysql_close(g_conn); // 关闭链接
	return res;
}

char *read_mysql(int server_id, const char *ip)
{
        MYSQL *g_conn; 
        MYSQL_ROW g_row; 
        MYSQL_RES *g_res;
        g_conn = mysql_init(NULL);

        // connect the database
        if(!mysql_real_connect(g_conn, _config.mysql_ip, _config.mysql_user, _config.mysql_passwd, "bpls", _config.mysql_port, NULL, 0))
        {
                printf("mysql connect failed\n");
        	mysql_close(g_conn); // 关闭链接
                return NULL;
        }

	char select_sql_m[64] = {0};
	char select_sql_a[64] = {0};
	char select_sql_s[64] = {0};
	/*
	sprintf(select_sql_m, "select * from bpls_stream_machine where stream_machine_id=%d and ip=\"%s\"", server_id, ip);
	if (executesql(g_conn, select_sql_m))
	{
		printf("select error\n");
        	mysql_close(g_conn); // 关闭链接
		return NULL;
	}

	g_res = mysql_store_result(g_conn); 
	int res = mysql_num_rows(g_res);
	if (res <= 0)
		return NULL;

	mysql_free_result(g_res); // 释放结果集
	*/

	sprintf(select_sql_a, "select * from bpls_camera where stream_machine_id=%d", server_id);
	if (executesql(g_conn, select_sql_a))
	{
		printf("bpls_camera select error\n");
        	mysql_close(g_conn); // 关闭链接
		return NULL;
	}

	g_res = mysql_store_result(g_conn); // 从服务器传送结果集至本地，mysql_use_result直接使用服务器上的记录集

	int iNum_rows = mysql_num_rows(g_res); // 得到记录的行数
	int iNum_fields = mysql_num_fields(g_res); // 得到记录的列数

	cJSON  *pJsonArry = NULL;
	cJSON  *pJsonsub  = NULL;
    	pJsonArry=cJSON_CreateArray();   /*创建数组*/

	int stream_server_id = -1;
	char stream_id[32] = {0};
	while ((g_row=mysql_fetch_row(g_res))) // 打印结果集
	{
		stream_server_id = atoi(g_row[6]);
		if (stream_server_id <= 0)
			continue;

		cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* 给创建的数组增加对对象*/
		memcpy(stream_id, g_row[0], strlen(g_row[0]));
		cJSON_AddStringToObject(pJsonsub, "stream_id", g_row[0]);      /* 给对象增加内容 */
		cJSON_AddStringToObject(pJsonsub, "logo", "nologo");      /* 给对象增加内容 */
		cJSON_AddStringToObject(pJsonsub, "width", g_row[13]);      /* 给对象增加内容 */
		cJSON_AddStringToObject(pJsonsub, "height", g_row[14]);      /* 给对象增加内容 */
		cJSON_AddStringToObject(pJsonsub, "protocol", g_row[4]);      /* 给对象增加内容 */
		cJSON_AddStringToObject(pJsonsub, "bitrate", g_row[10]);      /* 给对象增加内容 */
		cJSON_AddStringToObject(pJsonsub, "encode", g_row[12]);      /* 给对象增加内容 */
		cJSON_AddStringToObject(pJsonsub, "in_stream_addr", g_row[3]);      /* 给对象增加内容 */
		cJSON_AddStringToObject(pJsonsub, "status", g_row[9]);      /* 给对象增加内容 */
		cJSON_AddStringToObject(pJsonsub, "name", g_row[1]);      /* 给对象增加内容 */

		//find rtmp server ip and port
		char output_stream_addr[256] = {0};
		char stream_ip[32]   = {0};
		char stream_port[32] = {0};
        	MYSQL_ROW stream_row;
        	MYSQL_RES *stream_res;
		sprintf(select_sql_s, "select * from bpls_stream_server where stream_server_id=%d", stream_server_id);
		if (executesql(g_conn, select_sql_s))
		{
			printf("bpls_camera select error\n");
			mysql_close(g_conn); // 关闭链接
			return NULL;
		}

		stream_res = mysql_store_result(g_conn); 
		while ((stream_row=mysql_fetch_row(stream_res))) // 打印结果集
		{
			memcpy(stream_ip, stream_row[1], strlen(stream_row[1]));
			memcpy(stream_port, stream_row[2], strlen(stream_row[2]));
		}
		mysql_free_result(stream_res); // 释放结果集

		sprintf(output_stream_addr, "rtmp://%s:%s/live/1000%s", stream_ip, stream_port, stream_id);
		//find rtmp server ip and port end 
		cJSON_AddStringToObject(pJsonsub, "out_stream_addr", output_stream_addr);      /* 给对象增加内容 */
	}

    	char * str = cJSON_Print(pJsonArry);
    	if(NULL == str)
    	{
        	cJSON_Delete(pJsonArry);
		mysql_free_result(g_res); // 释放结果集
        	mysql_close(g_conn); // 关闭链接
        	return NULL;
    	}
    	printf("read mysql str = %s\n", str);
    	cJSON_Delete(pJsonArry);
	mysql_free_result(g_res); // 释放结果集
        mysql_close(g_conn); // 关闭链接

        return str;
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

