#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "stdarg.h"
#include "fcntl.h"
#include "sys/types.h"

//#include "times.h"
#include "log.h"

int nk = 1024;

int _level = 0;

char logfile[64]="logfile.log";

void logInit(const char *s,int level)
{
	memset(logfile,'\0',sizeof(logfile));
	strncpy(logfile,s,strlen(s));
	
	_level = level;
}

void logsk(const int sk)
{
	nk = sk;
}

void printfs(const char * fmt, ...){

	int RET = -1;

	char buf[2048]    = "\0";
	char logbuf[4096] = "\0";
	char times[20]="\0";

	va_list va_alist;

	va_start(va_alist, fmt);

	vsnprintf(buf, sizeof(buf), fmt, va_alist);

	va_end (va_alist);

	//printf("logfile save:%s\n",logfile);

	int fd;
	fd=open(logfile,O_WRONLY|O_CREAT|O_APPEND,0644);

	int k=1024;
	long fsize = lseek(fd,0L,SEEK_END);
	//printf("logfile size is:%ld\n",fsize);
	if(fsize>nk*k)
	{
		close(fd);
		fd=open(logfile,O_WRONLY|O_CREAT|O_TRUNC,0644);
	}

	if(fd ==-1)
	{
		printf("open error\n");
		return;
	}

	t_stime(times);

	snprintf(logbuf,sizeof(logbuf),"%s %s\n",times,buf);
	
	//printf("_level:%d\n",_level);
	
	if(_level) printf("%s",logbuf);

	RET = write(fd, logbuf, strlen(logbuf));
	if (RET != strlen(logbuf))
	{
		printf("shit\n");
	}

	close(fd);
}

void printbytes(const char * bytes,const int n)
{
	for(int i=0;i<n;i++)
	{
		printf("bytes[%d]:%02X\n",i,bytes[i]);
	}
}
