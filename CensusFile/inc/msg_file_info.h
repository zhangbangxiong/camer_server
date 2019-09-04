#ifndef _MSG_FILE_INFO_H
#define _MSG_FILE_INFO_H


#define SRC_PATH_LEN   	1024
#define VID_LEN  	32
#define PREFIX_LEN      128
#define OUT_PATH_LEN    1024

struct stored_info
{
	char prefix[PREFIX_LEN];
};

/*
 *info: contain info.
 *retcode: return code such as succeed or error code.
 */
typedef int (*fm_result_fun)(stored_info* info, int retcode);

typedef struct msg_file_info
{
	char src_path[SRC_PATH_LEN];
	char vid[VID_LEN];
	char prefix[PREFIX_LEN];
	char out_path[OUT_PATH_LEN];
	msg_file_info* next;	
}msg_file_info;

#endif
