#ifndef _FILE_MANAGE_EX_H
#define _FILE_MANAGE_EX_H

#include "msg_file_info.h"

int  mxfm_init(fm_result_fun fun);
int  mxfm_put(msg_file_info* info);
void mxfm_uninit();
	
#endif
