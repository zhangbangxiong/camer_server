/* This is tools of tvmservice
 * author: zhangbangxiong
 * mail: zhangbangxiong@tvmining.com
 * date: 2012-06-26 17:00
 */
#include "stdio.h"
#include "dirent.h"
#include "stdlib.h"
#include "unistd.h"
#include "list.h"

#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <getopt.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>


#define GM_GETPROGRAMTYPE         "getinfoprogramtype"
#define GM_GETMEDIAFILE           "getinfomediafile"
#define GM_GETCHANNELINFO         "getinfochannelinfo"
#define GM_GETMEDIALIST           "getinfomedialist"
#define GM_SETCHANNELINFO         "setchannel"
#define GM_GETPROGRAMMEINFO       "getinfoprogramme"
#define GM_MAKEPROGRAMMELIST      "makeprogramme"
#define GM_UPLOADMEDIAFILE        "upload"
#define GM_DISABLEMEDIA           "disablemedia" 
#define GM_EXTRACTPIC             "extractpic"



#define RS_GETPROGRAMTYPE         "type_id:%d type_name:%s #"
#define RS_GETMEDIAFILE           "media_id:%d media_name:%s former_name:%s pinyin:%s initial_letter:%s media_size:%d media_ext:%s m3u8_url:%s m3u8_name:%s media_duration:%d program_type:%d is_valid:%d media_status:%d media_url:%s thumb_url:%s media_tag:%s creat_user_id:%d #"
#define RS_GETCHANNELINFO         "channel_id:%d channel_name:%s log_url:%s play_url:%s remark:%s #"
#define RS_GETMEDIALIST           "media_id:%d meida_name:%s upload_name:%s pinyin:%s initial:%d size:%d duration:%d #"
#define RS_SETCHANNELINFO         "channel_id:%d play_url:%s #"
#define RS_GETPROGRAMMEINFO       "channel_id:%d play_date:%s media_id:%d start_time:%s duration:%d #"
#define RS_MAKEPROGRAMMELIST      "play_url:%s #"
#define RS_UPLOADMEDIAFILE        "media_id:%d filename:%s md5:%d media_url:%s create_time:%s guid:%d #"
#define RS_DISABLEMEDIA       
#define RS_EXTRACTPIC             "media_id:%d from:%d to:%d num:%d pic1_url:%s #"



#define nGMGETPROGRAMTYPE        0x0001       
#define nGMGETMEDIAFILE          0x0002       
#define nGMGETCHANNELINFO        0x0003       
#define nGMGETMEDIALIST          0x0004       
#define nGMSETCHANNELINFO        0x0005       
#define nGMGETPROGRAMMEINFO      0x0006     
#define nGMMAKEPROGRAMMELIST     0x0007    
#define nGMUPLOADMEDIAFILE       0x0008  
#define nGMDISABLEMEDIA          0x0009
#define nGMEXTRACTPIC             0x000a  

typedef struct PicNews{
    int   id;
    int   from;
    int   to;
    int   height;
    int   width;
    int   start;
    int   duration;
    char  playtime[255];
    char  media_name[255];
    char  prefix[255];
    char  pic1_url[255];
}picnews;

typedef struct MediaNews{
    int        id;
    int        start;
    int        duration;
    char       playtime[255];
    char       prefix[255];
    char       pic1url[255];
    char       media_name[255];
    char       media_url[255];
    char       thumb_url[255];
    char       tag[255];
    int16_t    create_user_id;
    char       create_time[50];
    char       bitrate[50];
    char       guid[255];
    char       remark[65525];
}medianews;

typedef struct Message{
    int nguid;
    int nid;
    int nprogram_type;
    int nline1_no;
    int nmax_lines;
    int nprogram_id;
    int nchannel_id;
    int nmedia_id;
    int nstart;
    int nduration;
    int nmedia_size;
    int ntag;
    int ninitial;
    int nfrom;
    int nto;
    int nprefix;
    int nheight;
    int nwidth;
    int nlines;
    int nprogrametype;
    int ndeal_status;
    int update_status;

    char nobject[255];
    char nname[255];
    char nplay_date[255];
    char nstart_time[255];
    char nupload_date[255];
    char nfilename[255];
    char nmedia_ext[255];
    char nremark[255];
    char nthumburl[255];
    char nlogourl[255];
    char nmedia_name[255];
    char nupload_name[255];
    char npinyin[255];
    char *result;

}message;


typedef struct Program_Type{
    int16_t        id;
    char           name[255];
}program_type;


typedef struct Live_Channel{
    int16_t        channel_id;
    char           channel_name[255];
    char           lodo_url[255];
    char           paly_url[255];
    char           m3u8_name[255];
    char           remark[255];
}live_channel;

typedef struct Program_Info{
    int16_t        program_id;
    int16_t        channel_id;
    char           paly_date[255];
    char           start_time[255];
    int            meida_id;
    char           media_name[255];
    int16_t        media_duration;
    char           remark[255];
}program_info;

void handle_event(void *argvs);
int32_t getinfocode(char *info);


