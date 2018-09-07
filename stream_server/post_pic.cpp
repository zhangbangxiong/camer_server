#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <post_pic.h>

#include <string>
#define HTTPHEAD_LEN    1024
#define SERVER_PORT     7500
#define TIME_OUT_TIME   3
#define CDN_ADDR        "http://%s:%d/v1/tfs"
#define POSTSTR         "POST /v1/tfs?suffix=.jpg HTTP/1.1\r\n"
#define HOSTSTR         "HOST: %s:%d \r\n"
#define CONTENTLENSTR   "Content-Length: %d \r\n"
#define DATESTR         "Date: %s \r\n\r\n"

using namespace std;

int init_sock(char* ip, int port)
{
    int sock = 0;
    struct sockaddr_in addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    
    struct timeval tm;

    tm.tv_sec = TIME_OUT_TIME;
    tm.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tm, sizeof(tm));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tm, sizeof(tm));
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        printf("connect timeout!\n");
        return -1;
    }

    return sock;   
}

int get_addr(char* path, char* buf, int len, char* addr)
{
    int begin, end;
    string str = string((const char*)buf);
    begin = str.find_first_of("TFS_FILE_NAME");

    if ((unsigned int)begin == string::npos)
    {
        printf("recv buf error!\n");
        return -1;
    }

    end = str.length();

    if (end < 20)
    {
        printf("recv too little info!\n");
        return -1;
    }

    begin = str.find_first_of("{");
    end   = str.find_last_of("}");
    str = str.substr(begin, end);
    
    begin = str.find_first_of(":");  
    end = str.find_last_of("}");
    str = str.substr(begin, end);

    //get the file name
    begin = str.find_first_of("\"");
    end = str.find_last_of("\"");
    
    str = str.substr(begin+1, end);
    end = str.find_first_of("\"");

    str = str.substr(0, end);        

    //printf("jpg addr: %s/%s.jpg\n", cdn_addr.c_str(), str.c_str());
    sprintf(addr, "%s/%s.jpg", path, str.c_str());
    //printf("jpg addr: %s\n", addr);
    return 0;
}

int upload_proc(char* ip, int port, int sock, char* path, int  vid, int index, char* image)
{
    char buf[1024] = {0};
    char* data = NULL;
    int  len = 0;
    int  ret = 0;   
    struct stat stat;
    int fd;
    int sended = 0, recved = 0;
    int trytime=0;
    string file = string(path);
    int pos = file.rfind('/');
    
    if ((unsigned int)pos != (file.length()-1))
    {
        file += "/";
    }
    sprintf(buf, "%d_%d.jpg", vid, index);
    file += string(buf);

    time_t timep;
    time(&timep);

    fd = open(file.c_str(), O_RDONLY);

    if (fd < 0)
    {
        printf("file:%s open error\n", file.c_str());    
        ret = -1;
        goto EXIT; 
    }

    fstat(fd, &stat);
    len = HTTPHEAD_LEN;

    data = (char*)malloc(len+stat.st_size+1);
    memset(data, 0, (len+stat.st_size)+1);
   
    len = 0;
    len += sprintf(data+len, POSTSTR);
    len += sprintf(data+len, HOSTSTR, ip, port);
    //len += sprintf(data+len, "Content-Length: %d \r\n", stat.st_size);
    len += sprintf(data+len, CONTENTLENSTR, (int)stat.st_size);
    len += sprintf(data+len, DATESTR, ctime(&timep));
    
    ret = read(fd, data+len, stat.st_size);
    close(fd);
    
    if (ret < 0)
    {
        printf("read file:%s error!\n", file.c_str());
        goto EXIT;
    }
    
    len = len + stat.st_size;
    do
    {
        sended = 0;
        recved = 0;
           
        while (sended < len)
        {
            ret = send(sock, data+sended, len-sended, 0);
        
            if (ret == -1)
            {
                printf("send error!\n");
                trytime++;
                continue;
            }
            sended += ret;
        }
    
        while (1)
        {
            ret = recv(sock, buf, 1024, 0);

            if (ret <= 0)
            {
                break;
            }
            recved += ret;
        }

        if (recved <= 0)
        {
            printf("cannt recved data!\n");
            ret = -1;
            trytime++;
            continue;
        }
        else
        {
            break;
        }
        
    }while(trytime<3);
    
    if (recved <= 0)
    {
        printf("send or recv  error!\n");
        ret = -1;
    }
    else
    {
	char server_path[256]={0};
		
    	sprintf(server_path, CDN_ADDR, ip, port);
        ret = get_addr(server_path, buf, recved, image);
    }

EXIT:
    if(data)
        free(data);

    return ret;
}

char* getipbyhost(char* hostname)
{
        struct hostent* host;

        host = gethostbyname(hostname);
        if (host == NULL)
        {
                printf("error\n");
                return NULL;
        }
        char* ip = inet_ntoa(*((struct in_addr*)host->h_addr));
        return ip;
}

int upload_pic(char* host, int port, char* path, int vid, char* image1, char* image2)
{
    if (host == NULL || path == NULL || vid <= 0 || image1 == NULL || image2 == NULL)
    {
        printf("input one or more invalid param!\n");
        return -1;
    }
 
    char *ip =  getipbyhost(host);
    if (ip == NULL)
    {
	printf("get host ip err!\n");
	return -1;
    } 
    int sock = -1, trytime = 0;
    do
    {
        sock = init_sock(ip, port);
        if (sock > 0)
        {
            break;
        }

        trytime++;
    }while(trytime<3);

    if (sock < 0)
    {
        printf("cannt connect server!\n");
        return sock;
    }    
    
    int ret = upload_proc(ip, port, sock, path, vid, 1, image1);

    if (ret < 0)
    {
        close(sock);
        return ret;
    }

    ret = upload_proc(ip, port, sock, path, vid, 2, image2); 
    
    if (ret < 0)
    {
        close(sock);
        return ret;
    }
    close(sock);
    printf("%s\r\n%s\r\n", image1, image2);
    return ret;   
}
