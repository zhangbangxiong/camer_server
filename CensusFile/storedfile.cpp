#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include "storedfile.h"
#include "getpath.h"
#include "errorcode.h"
int is_dir(const char* path)
{
	struct stat buf;
	if (stat(path, &buf) == -1)
	{
		return -1;
	}

	return S_ISDIR(buf.st_mode);
}

int storedfile::create_dirs(const char* path)
{
        int i, len;
        char *dir_path = NULL;
        len = strlen(path);

        dir_path = (char*)malloc(len+1);
        memset(dir_path, 0, len+1);
        strncpy(dir_path, path, len);

        for (i=0; i<len; i++)
        {
                if (dir_path[i] == '/' && i > 0)
                {
                        dir_path[i] = '\0';
                        if (access(dir_path, F_OK) < 0)
                        {
                                if (mkdir(dir_path, 0777) < 0)
                                {
                                        printf("mkdir=%s, error=%s\n", dir_path, strerror(errno));
                                        free(dir_path);
                                        return FILE_DIS_CREATEDIRERR;
                                }

                        }
                        dir_path[i] = '/';
                }
        }
        free(dir_path);
        return 0;
}

int storedfile::copy_file(char *spathname,char *tpathname)
{
        int sfd, tfd, filelen, ret=1;
        struct stat s;
        char buf[4096];
        sfd=open(spathname,O_RDONLY);
        tfd=open(tpathname,O_RDWR|O_CREAT);
	filelen = lseek(sfd, 0L, SEEK_END);
        lseek(sfd, 0L, SEEK_SET);
        while (1)
        {
                bzero(buf, 4096);
                ret = read(sfd, buf, 4096);
                if (ret == -1)
                {
                        printf("read src file: %s error!\n", spathname);
                        close(sfd);
                        close(tfd);
                        return FILE_DIS_OPENSRCERR;
                }
                int num = write(tfd, buf, ret);
		if (num != ret)
		{
			printf("write file: %s  error!\n", tpathname);
			return FILE_DIS_WRITEDSTERR;
		}
                filelen -= ret;
		if (filelen == 0)
		{
			break;
		}		
        }
        fstat(sfd,&s);
        chown(tpathname,s.st_uid,s.st_gid);
        chmod(tpathname,s.st_mode);

        close(sfd);
        close(tfd);
	return 0;
}

int storedfile::copy_files(char* sdirect, char* tdirect)
{
	struct dirent *sp;
   	char spath[1024]={0}, tpath[1024]={0}, temp_spath[1024]={0}, temp_tpath[1024]={0};
   	struct stat sbuf, temp_sbuf;
	int ret = 0, iscopy = 1;
   	DIR *dir_s,*dir_t;

   	dir_s=opendir(sdirect);
   	if (dir_s==NULL)
   	{
		printf("path: %s cannot open!", sdirect);
      		return FILE_DIS_ACCESSSRCPATHERR;
   	}
   	if (stat(sdirect,&sbuf)!=0)
   	{
		printf("path: %s cannot access!", sdirect);
      		return  FILE_DIS_ACCESSDSTPATHERR;
   	}
   	dir_t=opendir(tdirect);
   	if (dir_t==NULL)
   	{
      		mkdir(tdirect,sbuf.st_mode);
      		chown(tdirect,sbuf.st_uid,sbuf.st_gid);
      		dir_t=opendir(tdirect);
   	}
   	else
   	{
      		chmod(tdirect,sbuf.st_mode);
      		chown(tdirect,sbuf.st_uid,sbuf.st_gid);
   	}
   	strcpy(spath,sdirect);
   	strcpy(tpath,tdirect);
   	strcpy(temp_spath,sdirect);
   	strcpy(temp_tpath,tdirect);

	printf("------------begin copy--------------\n");
   	while ((sp=readdir(dir_s))!=NULL)
   	{
      		if (strcmp(sp->d_name,".")!=0&&strcmp(sp->d_name,"..")!=0)
      		{
          		strcat(temp_spath,"/");
          		strcat(temp_spath,sp->d_name);
          		//strcat(temp_tpath,"/");
          		strcat(temp_tpath,sp->d_name);
          		lstat(temp_spath,&sbuf);
          		temp_sbuf.st_mode=sbuf.st_mode;
          		if (S_ISLNK(temp_sbuf.st_mode))
          		{
              			printf("%s is a symbolic link file\n",temp_spath);
          		}
          		else if ((S_IFMT&temp_sbuf.st_mode)==S_IFREG)
          		{
				if (iscopy)
				{
					int trytimes = 0;
              				printf("copy file << %s >> \n",temp_spath);
	
RECOPY:
              				ret = copy_file(temp_spath,temp_tpath);
					if (ret != 0)
					{	
						if (trytimes < 3)
						{
							trytimes++;
							goto RECOPY;
						}
						strcat(m_failedpath, "/");
						strcat(m_failedpath, sp->d_name);
						iscopy = 0;
						m_failed++;
						return ret;
					}
					else
					{
						m_succeed ++;
					}
				}
				else
				{
					m_failed++;
				}
              			strcpy(temp_tpath,tpath);
              			strcpy(temp_spath,spath);
          		}
          		else if ((S_IFMT&temp_sbuf.st_mode)==S_IFDIR)
          		{
				
          		}
      		}
   	}

   	printf("----------------copy end!-----------------\n");
   	closedir(dir_t);
   	closedir(dir_s);
	return 0;	
}

storedfile::storedfile(char* srcpath, char* prefix, char* vid, char* outpath)
{
	strcpy(m_srcpath, srcpath);
	strcpy(m_vid, vid);	
	strcpy(m_prefix, prefix);
	strcpy(m_outpath, outpath);
	m_succeed = 0;
	m_failed  = 0;
	strcpy(m_failedpath, prefix);
}

storedfile::~storedfile()
{

}

int storedfile::copyfiles()
{
	int ret = 0;
        char* path;
	char srcpath[1024] = {0};
	char outpath[1024] = {0};

	strcpy(srcpath, m_srcpath);
	strcpy(outpath, m_outpath);
        path = getpath(m_vid);

        if (srcpath[strlen(srcpath)-1] != '/')
        {
                strcat(srcpath, "/");
        }
        if (outpath[strlen(outpath)-1] != '/')
        {
                strcat(outpath, "/");
        }
        strcat(srcpath, m_prefix);

        strcat(outpath, path);

        if (outpath[strlen(outpath)-1] != '/')
        {
                strcat(outpath, "/");
        }

        strcat(outpath, m_prefix);

        if (outpath[strlen(outpath)-1] != '/')
        {
                strcat(outpath, "/");
        }

        //printf("src: %s, out: %s \n", m_srcpath, m_outpath);
        ret = create_dirs(outpath);
	if (ret != 0)
	{
		return ret;
	}

        ret = copy_files(srcpath, outpath);
	/*
	char command[2048] = {0};
	sprintf(command, "cp -rf %s %s", m_srcpath, m_outpath);

	FILE* fp = popen(command, "r");

	if (!fp)
	{
		return -1;
	}*/
	
        return ret;
}

