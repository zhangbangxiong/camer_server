#ifndef _STORED_FILE_H
#define _STORED_FILE_H

class storedfile
{
public:
	storedfile(char* srcpath, char* prefix, char* vid, char* outpath);
	~storedfile();

	int copyfiles();
	int m_succeed;
	int m_failed;
	char m_failedpath[512];
private:
	int create_dirs(const char* path);
	int copy_file(char *spathname,char *tpathname);
	int copy_files(char* sdirect, char* tdirect);
	char m_srcpath[1024];
	char m_vid[32];
	char m_prefix[64];
	char m_outpath[1024];
	
};


#endif
