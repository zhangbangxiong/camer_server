#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>

#include <sys/time.h>
#include <stddef.h>
#include <netinet/tcp.h>

#include "util.h"
#include "http_post.h"
#include "config.h"
#include "zlog.h"

#define   RCVBUFLEN  4096

extern config_t _config;

static int sdk_resolve_inet(char *name, int port, struct sockinfo *si)
{
	int status;
	struct addrinfo *ai, *cai; /* head and current addrinfo */
	struct addrinfo hints;
	char *node, service[8];
	int found;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_NUMERICSERV;
	hints.ai_family = AF_INET;     /* AF_INET or AF_INET6 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;

	if (name != NULL) 
	{
		node = name;
	} 
	else 
	{
		node = NULL;
		hints.ai_flags |= AI_PASSIVE;
	}

	snprintf(service, 8, "%d", port);

	status = getaddrinfo(node, service, &hints, &ai);
	if (status < 0) 
	{
		return -1;
	}
	for (cai = ai, found = 0; cai != NULL; cai = cai->ai_next) 
	{
		si->family = cai->ai_family;
		si->addrlen = cai->ai_addrlen;
		memcpy(&si->addr, cai->ai_addr, si->addrlen);
		found = 1;
		break;
	}

	freeaddrinfo(ai);

	return !found ? -1 : 0;
}

/*
 *  Resolve a hostname and service by translating it to socket address and
 *  return it in si
 *  
 *  This routine is reentrant
 */
int sdk_resolve(char *name, int port, struct sockinfo *si)
{
	return sdk_resolve_inet(name, port, si);
}

int _connect()
{
	struct sockinfo sock;
	struct timeval tm;

	int m_sock = 0;
	int times  = 0;
	int ret    = 0;
	int port   = 0;

	port = atoi(_config.xreport_port);

	//printf("_config.xreport_server = %s, port = %d\n", _config.xreport_server, port);
	if(_config.xreport_server == NULL || port <= 0)
	{
		dzlog_info("upload server is error");
		return -1;
	}

	sdk_resolve(_config.xreport_server, port, &sock);
	m_sock = socket(AF_INET, SOCK_STREAM, 0);

	tm.tv_sec = 2;
	tm.tv_usec = 0;
	setsockopt(m_sock, SOL_SOCKET, SO_SNDTIMEO, &tm, sizeof(tm));
	setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &tm, sizeof(tm));

TODO:
	ret = connect(m_sock, (struct sockaddr *)&sock.addr, sock.addrlen);
	if (ret < 0)
	{
		if (times < 3)
		{
			dzlog_info("relink upload server ...");
			times++;
			sleep(1);
			goto TODO;
		}
		else
		{
			dzlog_info("connect %s:%d error!\n", _config.xreport_server, port);
			return -1;
		}
	}
	
	return m_sock;
}

int ipc_send(int m_sock, char* buf, int len)
{
	fd_set writefds;
	struct timeval tm;
	int ret, val, total = 0;

	tm.tv_sec = 3;
	tm.tv_usec = 0;

	while (1)
	{
		FD_ZERO(&writefds);
		FD_SET(m_sock, &writefds);

		val = select(m_sock + 1, NULL, &writefds, NULL, &tm);
		if (val < 0)
		{
			return -1;
		}
		if (val == 0)
		{
			continue;
		}

		if (FD_ISSET(m_sock, &writefds))
		{
			ret = send(m_sock, buf+total, len-total, 0);
			if (ret == 0)
			{
				return -1;
			}
			else if (ret < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				if ((errno != EAGAIN) || (errno != EWOULDBLOCK))
				{
					return -1;
				}
			}
			else
			{
				total += ret;
				if (total == len)
				{
					return 0;
				}
			}
		}

	}
	return 0;
}

int _send(const char* content)
{
	//printf("send data start ..\ncontent=%s\n",content);
	int val;
	int m_sock = 0;
	int sended = 0;
	char buf[RCVBUFLEN] = {0};
	int len = strlen(content);
	
	if (len == 0)
	{
		return 1;
	}

	m_sock = _connect();
	if(m_sock <= 0)
		return 1;	
RESEND:
	sended = 0;	
	while(sended < len)
	{
		val = send(m_sock, content+sended, len - sended, 0);
		if (val < 0)
		{	
			dzlog_info("send msg error!");
			sleep(1);
			m_sock = _connect();
			goto RESEND;
		}
		sended += val;
	}

RERECV:
	val = recv(m_sock, buf, RCVBUFLEN, 0);
	
	if (val > 0)
	{
		//printf("send end!\nrecv buf = %s\n", buf);	

		if(strstr(buf, "204") != NULL)
		{
			dzlog_debug("recv ok");
			return 0;
		}
		else 
		{
			dzlog_debug("recv failed");
			return 1;
		}

	}
	else
	{
		if ((val < 0) && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
		{
			//printf("re recv\n");
			goto RERECV;
		}
	}	
	return 0;
}

int send_msg(void *data)
{
	//printf("***** send_msg\n");
        CONVERTINFOLIST *info = (CONVERTINFOLIST *)data;

        char buf[256] = {0};
        char* content=(char*)malloc(info->data_len + 1024);
	//printf("send_msg len = %d, info->data = %s\n", info->data_len, info->data);

        //----------------------------

        memset(buf, 0, 256);
        sprintf(buf, "POST %s HTTP/1.0\r\n", info->addr);
        int len = 0;
	int ret = 0;
        len  = sprintf(content, "%s", buf);
        len += sprintf(content + len, "Host: %s\r\n", _config.xreport_server);
        len += sprintf(content + len, "Token:%s\r\n", info->token);
        len += sprintf(content + len, "Content-Length:%d\r\n\r\n", info->data_len);
        len += sprintf(content + len, "%s", info->data);

	dzlog_debug("--------send_msg data = (%s) -------------", info->data);
	ret = _send(content);

	return ret;
}

