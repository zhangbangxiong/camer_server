#ifndef _FILE_MANAGE_CORE_H
#define _FILE_MANAGE_CORE_H
#include <pthread.h>
#include "msg_file_queue.h"
//#include "httppost.h"


class file_manage
{
public:
	file_manage(fm_result_fun fun);
	~file_manage();
	
	int start();
	int stop();

	int put_msg(msg_file_info* msg);
	
	//int build_msg(msg_info* info, char* content);
	msg_file_queue m_msgque;
	//httppost  m_postmsg; 
	volatile int m_abort;	
	fm_result_fun  m_fun;
	//char 	m_ip[32];
	//int    	m_port;
private:
	pthread_t m_threadid;			
};
#endif


