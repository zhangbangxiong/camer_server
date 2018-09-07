#ifndef _MSG_MANAGE_EX_H
#define _MSG_MANAGE_EX_H

#include "msg_info.h"

extern "C" int  msgm_init();
extern "C" int  msgm_put(msg_info* info);
void msgm_uninit();
	
#endif
