/* This is tvmservice.h for tvmservicecodec
 * file: tvmcodec.h
 * author: zhangbangxiong
 * mail: zhangbangxiong@tvmining.com
 * date: 2011-01-10 14:30:00
*/

//#define SUCCESS          0      /* Operation is successful*/
#define VERSION          "1.0.1"

#define THREAD_COUNT     2
#define INPUT_BUFFER_SIZE 1024
#define NORMAL_BUFFER_SIZE 512


#define ERROR           -1     /* The other error */
#define ERRIN            1      /* Input file do not exist */
#define ERROUT           2      /* Output file path not exist */
#define ERRBMP           3      /* Bmp file is wrong */
#define ERRMP4DEC        4      /* Mp4 file dec is wrong */
#define ERRAVIDEC        5      /* Avi file dec is wrong*/
#define ERRTSDEC         6      /* Ts  file dec is wrong */
#define ERRPCMCVT        7      /* PCM convert is wrong */
#define ERRRAWCVT        8      /* RAW convert is wrong */
#define ERRPCMENC        9      /* PCM enc is wrong */
#define ERRRAWENC        10     /* RAW enc is wrong */
#define ERRMP4MUX        11     /* Mp4 file mux is wrong */
#define ERRMOVMUX        12     /* Mov file mux is wrong */
#define ERRTSMUX         13     /* Ts  file mux is wrong */
#define ERRDUMPAUDIO     14     /* Mp4 aduio dump error */
#define ERRDUMPVIDEO     15     /* Mp4 video dump error */
#define EBUSY            16      /* Device or resource busy */
#define EEXIST           17      /* File exists */
#define EXDEV            18      /* Cross-device link */
#define ENODEV           19      /* No such device */
#define ENOTDIR          20      /* Not a directory */
#define EISDIR           21      /* Is a directory */
#define EINVAL           22      /* Invalid argument */
#define ENFILE           23      /* File table overflow */
#define EMFILE           24      /* Too many open files */
#define ENOTTY           25      /* Not a typewriter */
#define ETXTBSY          26      /* Text file busy */
#define EFBIG            27      /* File too large */
#define ENOSPC           28      /* No space left on device */
#define ESPIPE           29      /* Illegal seek */
#define EROFS            30      /* Read-only file system */
#define EMLINK           31      /* Too many links */
#define EPIPE            32      /* Broken pipe */
#define EDOM             33      /* Math argument out of domain of func */
#define ERANGE           34      /* Math result not representable */

#define  LIMIT_COUNT     200
#define  TIME_STAMP      90
#define  VIDEO_VFR       1
#define  VIDEO_CFR       0
#define  UNIT_MS         1
#define  UNIT_CLOCK      90


typedef struct Videonews {

     struct list_head list;
     char        *videofile;
     char        *outfile;
     int         len;
     int         thread_loop;
     int         media_id;
     int         disconn;
     int         av_sync;
     int         nframe;
     int         flags; 
     int         stream_index;

}videonews;

typedef struct param_t
{
    char   *m3u8;
    char   *prog_news;
    char   *channel;
    char   *ymd;
    char      *filelist[1024];
    //char      **filelist;
    int  m3u8ret;
    int  first_number;
    int  nb_filelist;
    int  second;
    struct    tvm_st *tvm;

}param_t;

typedef struct THeadParam
{
    char   *infile;
    char   *outfile; 
    char   *ofilepath;
    char   *outfilename; //do not include ext
    char   *m3u8file;
    char   *inpath;
    char   *outpath;
    char    durationstream[1024];
    char    *videofile[THREAD_COUNT];
    pid_t   status[THREAD_COUNT];
    
    struct  list_head videohead;

    pthread_mutex_t videomutex;
    pthread_cond_t  videocond;

    pthread_mutex_t rawencmutex;
    pthread_cond_t  rawenccond;

    pthread_mutex_t pcmencmutex;
    pthread_cond_t  pcmenccond;

    pthread_mutex_t adumpmutex;
    pthread_cond_t  adumpcond;

    pthread_mutex_t vdumpmutex;
    pthread_cond_t  vdumpcond;

    pthread_mutex_t condmutex;
    pthread_cond_t  condcond;

    pthread_mutex_t limitmutex;
    pthread_cond_t  limitcond;

    int  video_counts;
    int  thread_counts;
    int  thread_loop[THREAD_COUNT];
 
    int  avjpeg;
    int  jpegframeno;
    int  jpegnumbers;
    int  jpegstarttime; /* start output jpeg start time */

    param_t *param;

    int   video_flag;
    int   video_unit;
    int   differ_count;
    int   media_id;
    config_t  *config;

}THreadParam;

typedef struct THREAD_NEWS
{
    char *videofile[THREAD_COUNT];
    char *outfile[THREAD_COUNT];
    char *m3u8file[THREAD_COUNT];
    pid_t   status[THREAD_COUNT];
}threadnews;

typedef struct Media_File{
    int16_t        file_id;
    int16_t        user_id;
    char           FileNameLocal[255];
    char           FileNameRemote[255];
    char           FilePathLocal[512];
    char           FilePathRemote[512];
    char           FilePathRelative[512];
    char           FileMD5[32];
    int            status;
    int            FileLength;
    char           FileSize[10];
    int16_t        FilePos;
    int16_t        iPostedLength;
    char           PostedPercent[6];
    int            PostComplete;
    char           PostedTime[255];
    int            IsDeleted;
    int            Sort;
}media_file;
 
typedef struct av_argvs_st {
    char       *inpath;     /* input file path */
    char       *outpath;    /* output file path */
    char       *cfg;        /* config file path*/
}*av_argvs_t;

typedef struct Date{

    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;

}date;


void servicecodec(void *argvs);

//#endif //_TVM_UTIL_H_

