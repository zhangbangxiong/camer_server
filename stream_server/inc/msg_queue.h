#ifndef _MSG_QUEUE_H
#define _MSG_QUEUE_H

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "msg_info.h"


class msg_queue
{
public:
	msg_queue();
	~msg_queue();
	
	int put(msg_info* info);
	int get(msg_info* info);
	uint64_t get_size();
	int flush();
	
private:
	volatile uint64_t m_size;
	msg_info* m_first;
	msg_info* m_last;
	pthread_mutex_t m_mutex;
	pthread_cond_t  m_cond;	
};

#endif 
