#ifndef _HTTP_POST_MSG_H_
#define _HTTP_POST_MSG_H_

#ifdef __cplusplus  
extern "C" {  
#endif  
  
int send_msg(void *data, char *addr, char *host, int port);
  
#ifdef __cplusplus  
}  
#endif  

#endif
