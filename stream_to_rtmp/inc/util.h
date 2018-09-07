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

#define POST_CONVERT_ADRESS            "/Mq/convert.accept.ssp"
#define POST_CONVERT_START             "/Mq/convert.start.ssp"
#define POST_CONVERT_FINISH            "/Mq/convert.finish.ssp"
#define POST_CONVERT_DONE              "/Mq/convert.done.ssp"
#define POST_CUT_START                 "/Mq/cut.start.ssp"
#define POST_CUT_FINISH                "/Mq/cut.finish.ssp"
#define POST_CUT_DONEi                 "/Mq/cut.done.ssp"
#define POST_DISTRIBUTE_START          "/Mq/distribute.start.ssp"
#define POST_DISTRIBUTE_FINISH         "/Mq/distribute.finish.ssp"
#define POST_DISTRIBUTE_DONE           "/Mq/distribute.done.ssp"

#define EVENT_UPLOAD                   "upload"
#define EVENT_CONVERT                  "convert"
#define EVENT_CUT                      "cut"
#define EVENT_DISTRIBUTE               "distribute"

#define ACTION_START                   "start"
#define ACTION_FINISH                  "finish"
#define ACTION_DONE                    "done"

#define SYSTEM_NAME "fcodec" 

typedef struct ts_chips_info
{
    int  ts_chips_nums;
    //char (*ts_chips_filename)[LEN_VIDEO];
    char **ts_chips_filename;
    char **ts_chips_timelen;
    //char (*ts_chips_timelen)[LEN_VIDEO];
    char **ts_chips_distribute_path;
    //char (*ts_chips_distribute_path)[LEN_VIDEO];
}TSCHIPSINFO;

typedef struct delogo_info
{
	int x1;
	int y1;
	int x2;
	int y2;
}DELOGOINFO;

typedef struct task_info
{
    char video_name[LEN_VIDEO*5];
    int  video_id;
    char video_type[LEN_VIDEO];
    char video_path[LEN_VIDEO];
    char video_sha1[LEN_VIDEO];
    char video_size[LEN_VIDEO];
    char submit[LEN_VIDEO];
    int  image_width;
    int  image_height;
    struct  delogo_info _delogo[4];
    int  delogo_position[4];
    int  delogo_counts;

}TASKINFO;

typedef struct tscut_info
{
    int  video_id;
    char file_name[LEN_VIDEO];
    int  clarity;
    int  bitrate;
    char path_prefix[LEN_VIDEO*4];
    char *fileinfo;
    int  status;
}TSCUTINFO;


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

void *start_gome_transcoder(void *params);
int get_ts_chips_info(char *path, TSCHIPSINFO *data);
int creat_clarity_bitrate_json(char *data, int bit_rate);
int get_upload_finish_data(char *path, int id, char **data);
char *url_encode(char const *s, int len, int *new_len);
void *send_convert_info(void *ptr);
void *file_distrib_thrd(void *ptr);
int check_codec_log(char *path, int *error_no);
int parse_week_info(char *week, int winfo[]);
time_t _gettime_w(int *week);       //unit: s
time_t str_to_time(char *_t);
long long _gettime_s();   
#endif
