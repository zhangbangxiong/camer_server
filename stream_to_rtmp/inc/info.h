/* This is tools of tvmservice
 * author: zhangbangxiong
 * mail: zhangbangxiong@tvmining.com
 * date: 2012-06-26 17:00
 */
#include "stdio.h"
#include "dirent.h"
#include "stdlib.h"
#include "unistd.h"

#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <getopt.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define NORMAL_BUFFER_SIZE 512



