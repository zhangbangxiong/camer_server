#ifndef _MSG_INFO_H
#define _MSG_INFO_H


#define MSG_ADDR_LEN   128
#define MSG_TOKEN_LEN  64
#define MSG_DATA_LEN   1024

typedef struct msg_info
{
	char addr[MSG_ADDR_LEN];
	char token[MSG_TOKEN_LEN];
	int  len;
	char *data;
	struct msg_info* next;	
}msg_info;

#endif
