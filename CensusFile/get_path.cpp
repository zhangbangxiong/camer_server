#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <strings.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "md5_jf.h"

using namespace std;

char *MD5_data (char *data, int md5_len)
{	
	MD5_CTX mdContext;
	int bytes;
	char *file_md5;
	int i;

	MD5Init (&mdContext);

	MD5Update (&mdContext, data, strlen(data));

	MD5Final (&mdContext);

	file_md5 = (char *)malloc((md5_len + 1) * sizeof(char));
	if(file_md5 == NULL)
	{
		fprintf(stderr, "malloc failed.\n");
		return NULL;
	}
	memset(file_md5, 0, (md5_len + 1));

	if(md5_len == 16)
	{
		for(i=4; i<12; i++)
		{
			sprintf(&file_md5[(i-4)*2], "%02x", mdContext.digest[i]);
		}
	}
	else if(md5_len == 32)
	{
		for(i=0; i<16; i++)
		{
			sprintf(&file_md5[i*2], "%02x", mdContext.digest[i]);
		}
	}
	else
	{		
		free(file_md5);
		return NULL;
	}
	
	return file_md5;
}

char* getpath(char *videoid)
{
	char *md5;
	char temp[32] = {0};
	uint64_t vid;
	int ret;
	std::string path;
	
	md5 = MD5_data(videoid, 32);

	if (md5 == NULL)
	{
		printf("md5 failed!\n");
		return NULL;
	}

	path = string(md5);
	free(md5);

	path = path.substr(0, 2);
	path = path + "/";	
	
	vid = atoll(videoid);
	
	sprintf(temp, "%d", vid % 3000);
	path = path + string(temp) + "/";

	vid = vid*vid;

	sprintf(temp, "%d", vid % 3000);
	path = path + string(temp) + "/";	
	
	return (char*)path.c_str();
}
