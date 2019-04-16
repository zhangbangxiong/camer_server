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

#define HTTP_HEADER_LEN         1024

int set_nonblock(int sd)
{
    int flags;

    flags = fcntl(sd, F_GETFL, 0);
    if (flags < 0) {
        return flags;
    }
    return fcntl(sd, F_SETFL, flags | O_NONBLOCK);
}

static int _connect(char *ip, int port)
{
    struct sockaddr_in addr;
    int skt;

    skt = socket(AF_INET, SOCK_STREAM, 0);

	printf("ip = %s\n", ip);
    memset(&addr, 0, sizeof(addr) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
	printf("port = %d\n", port);

    connect(skt, (struct sockaddr *)&addr, sizeof(addr));
    return skt;
}

int send_msg(char *ip, int port, char *addr, char *res)
{
	char mess[256] = {0};
	int val = 0;
	int mess_len = 0;
	char res1[1024] = {0};
	int skt = _connect(ip, port);
	mess_len = sprintf(mess, "POST /%s HTTP/1.0\r\n", addr);
	mess_len += sprintf(mess + mess_len, "Host:%s\r\n", ip);
	mess_len += sprintf(mess + mess_len, "Content-Type:application/x-www-form-urlencoded\r\n");
	mess_len += sprintf(mess + mess_len, "Content-Length:%d\r\n\r\n", 0);

	printf("mess = %s\n", mess);
	val = send(skt, mess, mess_len, 0);
	if (val < 0)
	{
		goto END;
	}

	val = recv(skt, res1, sizeof(res1), 0);
	printf("res1 = %s\n", res1);
	if (val < 0)
	{
		goto END;
	}
	else
	{
		if (strstr(res1, "result"))
			strcpy(res, strstr(res1, "\r\n\r\n")+4);
	}
	
END:
	close (skt);
	return val;
}

