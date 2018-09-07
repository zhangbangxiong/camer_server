#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "md5.h"

int MD5_data (char *data, char *result, int len)
{	
	MD5_CTX mdContext;
	int i = 0;
	char dec[36];
	char decrypt[16];

	MD5Init (&mdContext);
	MD5Update (&mdContext, (unsigned char*)data, strlen(data));
	MD5Final (&mdContext, (unsigned char *)decrypt);

	for(i=0; i < 16; i++)
	{
		sprintf(dec + (i * 2), "%02x", decrypt[i]);
	}
	
        strncpy(result, dec, len);

	return 0;
}

int get_path(int id, char *res)
{
	char _md5[48] = {0};
	char temp[32] = {0};
	long long vid = 0;
	int pos = 0;
	char path[128] = {0};
	
	char tid[16] = {0};
	sprintf(tid, "%d", id);
	MD5_data(tid, _md5, sizeof(_md5));

	memcpy(path, _md5, 2);
	pos += 2;
	path[pos] = '/';	
	pos += 1;
	
	vid = id;
	
	sprintf(temp, "%lld/", vid % 3000);
	memcpy(path + pos, temp, strlen(temp));
	pos += strlen(temp);

	vid = vid*vid;

	memset(temp, 0, sizeof(temp));
	sprintf(temp, "%lld/", vid % 3000);
	memcpy(path + pos, temp, strlen(temp));
	
	memcpy(res, path, strlen(path));

	return 0;
}
