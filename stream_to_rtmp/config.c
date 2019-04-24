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

    if ((cxml = ezxml_child(rxml, "server_ip")) != 0x0)      /* codec exec dir */
        strcpy(config->server_ip, cxml->txt);

    if ((cxml = ezxml_child(rxml, "stream_server_ip")) != 0x0)      /* codec exec dir */
        strcpy(config->stream_server_ip, cxml->txt);

    if ((cxml = ezxml_child(rxml, "stream_server_http_port")) != 0x0)      /* codec exec dir */
       config->stream_server_http_port = atoi(cxml->txt);

    if ((cxml = ezxml_child(rxml, "sh_exec_dir")) != 0x0)      /* codec exec dir */
        strcpy(config->sh, cxml->txt);

    if ((cxml = ezxml_child(rxml, "stream_exec_dir")) != 0x0)      /* codec exec dir */
        strcpy(config->ffmpeg, cxml->txt);

    if ((cxml = ezxml_child(rxml, "kill_sh_dir")) != 0x0)      /* codec exec dir */
        strcpy(config->kill_sh, cxml->txt);

    if ((cxml = ezxml_child(rxml, "logo_path")) != 0x0)      /* codec exec dir */
        strcpy(config->logo_path, cxml->txt);

    if ((cxml = ezxml_child(rxml, "logo_pos")) != 0x0)      /* codec exec dir */
        config->logo_pos = atoi(cxml->txt);
    else
	config->logo_pos = -1;

    if ((cxml = ezxml_child(rxml, "log_path")) != 0x0)      /* codec log path */
        strcpy(config->log_path, cxml->txt);

}



