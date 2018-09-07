#ifndef _MSG_MANAGE_CORE_H
#define _MSG_MANAGE_CORE_H
#include <pthread.h>
#include "msg_queue.h"
#include "http_post.h"


class msg_manage
{
public:
	msg_manage();
	~msg_manage();
	
	int start();
	int stop();

	int put_msg(msg_info* msg);
	
	int build_msg(msg_info* info, char* content);
	msg_queue m_msgque;
	httppost  m_postmsg; 
	volatile int m_abort;	
	char m_ip[32];
	int    m_port;
private:
	pthread_t m_threadid;			
};
#endif


