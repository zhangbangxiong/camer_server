#include "stdio.h"
#include "dirent.h"
#include "stdlib.h"
#include "unistd.h"
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <getopt.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "config.h"
#include "util.h"
#include "ezxml.h"
#include "arm_xml.h"

void configure(struct Config *config)
{
    ezxml_t rxml;
    ezxml_t cxml;

    rxml = ezxml_parse_file("etc.xml");
   
    if ((cxml = ezxml_child(rxml, "server_id")) != 0x0)      /* codec exec dir */
        strcpy(config->server_id, cxml->txt);

    if ((cxml = ezxml_child(rxml, "server_port")) != 0x0)      /* codec exec dir */
       strcpy(config->server_port, cxml->txt);

    if ((cxml = ezxml_child(rxml, "stream_server_http_port")) != 0x0)      /* codec exec dir */
       config->stream_server_http_port = atoi(cxml->txt);

    if ((cxml = ezxml_child(rxml, "stream_server_ip")) != 0x0)      /* codec exec dir */
       strcpy(config->stream_server_ip, cxml->txt);

    if ((cxml = ezxml_child(rxml, "store_server_ip")) != 0x0)      /* codec exec dir */
        strcpy(config->store_server_ip, cxml->txt);

    if ((cxml = ezxml_child(rxml, "store_download_port")) != 0x0)      /* codec exec dir */
       config->store_download_port = atoi(cxml->txt);

    if ((cxml = ezxml_child(rxml, "vod_server_ip")) != 0x0)      /* codec exec dir */
        strcpy(config->vod_server_ip, cxml->txt);

    if ((cxml = ezxml_child(rxml, "vod_m3u8_port")) != 0x0)      /* codec exec dir */
       config->vod_m3u8_port = atoi(cxml->txt);

    if ((cxml = ezxml_child(rxml, "mysql_ip")) != 0x0)      /* codec exec dir */
        strcpy(config->mysql_ip, cxml->txt);

    if ((cxml = ezxml_child(rxml, "mysql_port")) != 0x0)      /* codec exec dir */
       config->mysql_port = atoi(cxml->txt);

    if ((cxml = ezxml_child(rxml, "mysql_user")) != 0x0)      /* codec exec dir */
        strcpy(config->mysql_user, cxml->txt);

    if ((cxml = ezxml_child(rxml, "mysql_passwd")) != 0x0)      /* codec exec dir */
        strcpy(config->mysql_passwd, cxml->txt);

    if ((cxml = ezxml_child(rxml, "m3u8_path")) != 0x0)      /* codec exec dir */
        strcpy(config->m3u8_path, cxml->txt);

    if ((cxml = ezxml_child(rxml, "sh_exec_dir")) != 0x0)      /* codec exec dir */
        strcpy(config->sh, cxml->txt);

    if ((cxml = ezxml_child(rxml, "stream_exec_dir")) != 0x0)      /* codec exec dir */
        strcpy(config->ffmpeg, cxml->txt);

    if ((cxml = ezxml_child(rxml, "kill_sh_dir")) != 0x0)      /* codec exec dir */
        strcpy(config->kill_sh, cxml->txt);

    if ((cxml = ezxml_child(rxml, "logo_pre_path")) != 0x0)      /* codec exec dir */
        strcpy(config->logo_pre_path, cxml->txt);

    if ((cxml = ezxml_child(rxml, "log_path")) != 0x0)      /* codec log path */
        strcpy(config->log_path, cxml->txt);

    if ((cxml = ezxml_child(rxml, "vod_servers")) != 0x0)      /* codec exec dir */
    {
	int i = 0;
        ezxml_t vod_server;
	vod_server = cxml->child;
	for(;vod_server != NULL; vod_server = vod_server->next, i++)
	{
		strcpy(config->vs[i].ip, vod_server->txt);
		config->vs[i].port = atoi(ezxml_attr(vod_server, "port"));
		printf("ip   = %s\n", config->vs[i].ip);
		printf("port = %d\n", config->vs[i].port);
	}
	config->vod_server_nums = i;
	printf("vod_server_nums = %d\n", config->vod_server_nums);
    }

    free(rxml);
    free(cxml);
}



