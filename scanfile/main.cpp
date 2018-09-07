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
#include <getopt.h>

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
#include "cjson.h"
#include "mongoose.h"
#include "zlog.h"
#include "http_post.h"

#include <iostream>  
#include <string>  
#include <vector>  
#include <sys/stat.h>  
#include <regex.h>  
#include <libgen.h>  
#include <dirent.h>  
#include <assert.h>  
#include <string.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include "curl/curl.h"
extern "C" {
#include "config.h"
}

using namespace std;  
vector<string> v_file_path;  
vector<string> v_file_name;  

#define DOWNLOAD_URL_PRE "http://27.223.92.102:7070/"

int save_time = 66400;
config_t  _config;

long long _gettime_s(void)       //unit: s
{
    struct timeval tv;

    gettimeofday(&tv,NULL);
    signed long long ts = (signed long long)tv.tv_sec;
    return ts;
}

int regex_match(const char *buffer, const char *pattern)  
{  
    	int ret = 0;  
    	char errbuf[1024] = {0};  
    	regex_t reg;  
    	regmatch_t pm[1] = {0};  
    	ret = regcomp(&reg, pattern, REG_EXTENDED | REG_ICASE);  

    	if (ret != 0) 
	{  
        	regerror(ret, &reg, errbuf, sizeof(errbuf));  
        	fprintf(stderr, "%s:regcom(%s)\n", errbuf, pattern);  
        	return -1;  
    	}
  
    	if (regexec(&reg, buffer, 1, pm, 0) == 0) 
	{  
        	regfree(&reg);  
        	return 0;   
    	}  
    	else 
	{  
        	regfree(&reg);  
        	return -1;  
    	}  
}  

int scan_dirpath(char *path, char *pattern)
{  
    	char file_path[512] = {0};  
    	char file[512] 	    = {0};  
    	DIR *dir = NULL;  

    	struct dirent *ptr = NULL;  
    	struct stat buf;  
    	int i = 0, j = 0;
  
    	if ((dir = opendir(path)) == NULL) 
	{  
        	perror("opendir failed!");  
        	return -1;  
    	}  

    	while((ptr = readdir(dir)) != NULL) 
	{  
        	if (ptr->d_name[0] != '.') 
		{
            		strcpy(file_path, path);  
            		if (path[strlen(path) - 1] != '/')  
				strcat(file_path, "/");  

            		strcat(file_path, ptr->d_name);

            		int ret = stat(file_path, &buf);  
            		if (ret != 0)  
				continue;

			printf("00 file_path = %s\n", file_path);
            		if(S_ISREG(buf.st_mode)) 
			{        
                		for(i = 0; i < (int)strlen(file_path); i++) 
				{  
                    			if(file_path[i] == '/') 
					{  
                        			memset(file, 0, strlen(file));  
                        			j = 0;  
                        			continue;  
                    			}	  
                    			file[j++] = file_path[i];  
                		}	  

                		if (regex_match(file, pattern) == 0) 
				{ 
                    			v_file_path.push_back(file_path);  
                		}	  
            		}  
            		else if(S_ISDIR(buf.st_mode)) 
			{     
                		scan_dirpath(file_path, pattern);  
            		}  
        	}  
    	}
	closedir(dir);  
  
    	return 0;  
}  
  
int url_encode(const char* src, const int srcsize, char* dst, int dstsize)
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

static size_t write_callback_1(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int len = size * nmemb;
    return len;
}

#define FILENAME "/tmp/curlposttest.log"   

int send_info(char* url, char* data)
{
    FILE *fptr;   
    //struct curl_slist *http_header = NULL;   
  
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
    	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_1);   
    	//curl_easy_setopt(curl, CURLOPT_WRITEDATA, fptr);   
    	curl_easy_setopt(curl, CURLOPT_POST, 1);   
    	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);   
    	curl_easy_setopt(curl, CURLOPT_HEADER, 1);   
    	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);   
    	//curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/tmp/curlposttest.cookie");   
        res = curl_easy_perform(curl);
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

int send_data(char *url, char *data)
{
	char  arg[256] = {0};
	sprintf(arg, "curl_1 -l -H \"Content-type: application/json\" -X POST -d \'%s\' %s", data, url);
	printf("arg = %s\n", arg);
	//system(arg);
	return 0;

}

int check_file(const char *file)
{
        struct stat buf;
        int  result = 0;
        long long now_time   = 0;
        long long start_time = 0;
        long long end_time   = 0;
	int  dif_time	     = 0;

        if (access(file, F_OK) == -1)
        {
               dzlog_info("file %s is not exist", file);
               return -1;
	}
	
        result = stat(file, &buf);
        if ( result != 0 )
        {
                return -2;
        }
        else
        {
		start_time = buf.st_atime;
		end_time   = buf.st_mtime;
                now_time = _gettime_s();
                dif_time = _gettime_s() - buf.st_mtime;
		//dzlog_info("file = %s, start_time = %lld, end_time = %lld, now_time = %lld\n", file, start_time, end_time, now_time);
	}

	return dif_time;
}

int report_file_info(char *filepath, char *filename)
{
	char curl[] = "http://192.168.102.248:80/api/record/create";
        char _data[256] = {0};
        char edata[512] = {0};

        struct stat buf;
        int  result = 0;
        char file[255] = {0};
	long long start_time = 0;
	long long end_time   = 0;

	char tmp[512] = {0};
	char camera_id[64] = {0};

	memcpy(tmp, filepath, strlen(filepath));
	printf("tmp = %s\n", tmp);
	if (tmp[strlen(tmp) - 1] == '/')
	{
		tmp[strlen(tmp) - 1] = '\0';
	}
	char *q = strrchr(tmp, '/');
	if (q != NULL)
	{
		memcpy(camera_id, q + 1, strlen(tmp) - (q - tmp));
	}

        sprintf(file, "%s%s.ts", filepath, filename);
        if (access(file, F_OK) == -1)
        {
               dzlog_info("file %s is not exist", filepath);
               return -1;
        }

        result = stat(file, &buf);
        if ( result != 0 )
        {
                return -2;
        }
        else
        {
		printf("file name    = %s\n", file);
		printf("buf.st_mtime = %ld\n", buf.st_mtime);
		printf("buf.st_mtime = %ld\n", buf.st_ctime);
		start_time = buf.st_atime;
		end_time   = buf.st_mtime;
                //long long now_time = _gettime_s();
                //int dif_time = _gettime_s() - buf.st_mtime;
    		sprintf(_data, "{\"camera_id\":\"%s\",\"start_time\":%lld,\"start_time\":%lld,\"download_url\":\"%s\"}", "4", start_time, end_time, "test");
		url_encode(_data, strlen(_data), edata, 1024);
		printf("edata = %s\n", edata);
		send_data(curl, _data);
        }

	return 0;
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
        int len  = size * nmemb;
        printf("ptr = %s\n", (char *)ptr);
    	save_time = atoi((char *)ptr);

	return len;
}

int get_save_time()
{
        CURL *curl;

        char url[256] = {0};
        sprintf(url, "http://%s:%s/getsavetime", _config.server_ip, _config.server_port);
	printf("url == %s\n", url);

        curl = curl_easy_init();
        if (curl)
        {
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
                curl_easy_perform(curl);
        }
	
	curl_easy_cleanup(curl); 

        return 0;
}

int main(int argc, char **argv)  
{  
	char pattern[32] = ".*";
        int rc = 0;
        rc = dzlog_init("scan_log.conf", "scan");
        if (rc)
        {
                fprintf(stderr, "dzlog init error\n");
                return 1;
        }
        dzlog_info("starting...");

	configure(&_config);
	get_save_time();

	printf("store_path = %s\n", _config.store_path);
	if (_config.store_path == NULL)
	{
		printf("please add store path in config file\n");
		return -1;
	}

	while (1)
	{
		v_file_path.clear();	

    		scan_dirpath(_config.store_path, pattern);  
		dzlog_info("v_file_path.size() = %d, save_time = %d", (int)v_file_path.size(), save_time);

		dzlog_info("--------------------------------");
    		for (int i = 0; i < (int)v_file_path.size(); i++) 
		{  
			int  res = check_file(v_file_path[i].c_str());
			if (res >= save_time)
			{
				if (remove(v_file_path[i].c_str()) == 0)
				{
					dzlog_info("del the file:%s success", v_file_path[i].c_str());
					continue;
				}
				else
				{
					dzlog_info("del the file:%s failed", v_file_path[i].c_str());
				}
			}

    		}
		dzlog_info("================================\n");
		sleep(3);
	}
  
    	return 0;  
}  

