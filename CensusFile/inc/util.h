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

#ifndef _SOOONER_UTIL_H_
#define _SOOONER_UTIL_H_

#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#define MD5_LEN		36
#define	LEN_EVENTID		64
#define	LEN_PARAM		64
#define	LEN_OBDID		32
#define	LEN_TOKEN		128
#define	LEN_VIDEO		128
#define MAX_BUFFER_SZIE         1024*1024

#define INPUT_BUFFER_SIZE 1024
#define NOMARL_BUFFER_SIZE 256
#define BUFFER_SIZE 128
#define DATA_SIZE 512

struct sockinfo {
    int       family;              /* socket address family */
    socklen_t addrlen;             /* socket address length */
    union {
        struct sockaddr_in  in;    /* ipv4 socket address */
        struct sockaddr_in6 in6;   /* ipv6 socket address */
        struct sockaddr_un  un;    /* unix domain address */
    } addr;
};

 
int sdk_resolve(char *name, int port, struct sockinfo *si);

struct node  
{  
	char 	md5[MD5_LEN];	//key
    int 	pts;		//Ê±¼äÆ«ÒÆÁ¿
    struct node	*next;//Á´Óò  
      
};  

char *url_encode(char const *s, int len, int *new_len);
int parse_week_info(char *week, int winfo[]);
time_t _gettime_w(int *week);       //unit: s
long long _gettime_s(void) ;
time_t str_to_time(char *_t);
#endif
