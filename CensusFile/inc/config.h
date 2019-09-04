/* This is tools of tvmservice
 * author: zhangbangxiong
 * mail: zhangbangxiong@tvmining.com
 * date: 2012-06-26 17:00
 */

#define b_len 256
#define s_len 64
#define OUTPUTVIDEOMULS   100
#define CODECSERVERCOUNTS 100
#include <sys/time.h>
#include <sys/types.h>

typedef struct OutputCodecParam
{
    char xvideobitrate[16];
    char xvideowidth[16];
    char xvideoheight[16];
}outputcodecparam;

typedef struct redis_server
{
    char xip[32];
    char xhost[128];
    char xport[32];
    char xpassword[32];
}REDIS_SERVER;

typedef struct report_server
{
    char ximage_upload_server[b_len];
    char ximage_upload_port[16];
    char ximage_http_url[b_len];
    char xreport_server[b_len];
    char xreport_port[16];
    char xreport_token[s_len];

}REPORT_SERVER;

typedef struct server_Param
{
	REDIS_SERVER  _redis_server[CODECSERVERCOUNTS];
	REPORT_SERVER _report_server;
    	int  xredis_server_count;
	
}SERVER_PARAM;

typedef struct stream_info
{
	char sh[b_len];
	char stream_id[b_len];
	char stream_name[b_len];
	char out_stream_addr[b_len];
	char in_stream_addr[b_len];
	char protocol[s_len];
	char logo[b_len];
	char encode[s_len];
	int  hasaudio;
	char  bitrate[s_len];
	char  frame_rate[s_len];
	char  width[s_len];
	char  height[s_len];
	int  status;
	pthread_t thread_id;
}STREAMINFO;

typedef struct Config{
    char id[16];
    char server_ip[64];
    char server_id[16];
    char server_port[16];
    char store_server_ip[64];
    int  store_download_port;
    int  store_m3u8_port;
    char stream_server_ip[64];
    int  stream_server_http_port;
    char mysql_ip[64];
    int  mysql_port;	
    char mysql_user[32];
    char mysql_passwd[32];
    char m3u8_path[32];
    char ffmpeg[b_len];
    char sh[b_len];
    char kill_sh[b_len];
    char logo_pre_path[b_len];
    char log_path[b_len];
    char store_path[b_len];
    int m3u8_time;
}config_t;

void configure(struct Config *config);
