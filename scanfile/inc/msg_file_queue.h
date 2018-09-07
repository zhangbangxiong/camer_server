#ifndef _MSG_FILE_QUEUE_H
#define _MSG_FILE_QUEUE_H

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "msg_file_info.h"


class msg_file_queue
{
public:
	msg_file_queue();
	~msg_file_queue();
	
	int put(msg_file_info* info);
	msg_file_info* get();
	uint64_t get_size();
	int flush();
	
private:
	volatile uint64_t m_size;
	msg_file_info* m_first;
	msg_file_info* m_last;
	pthread_mutex_t m_mutex;
	pthread_cond_t  m_cond;	
};

#endif 
