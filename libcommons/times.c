#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "sys/time.h"
#include "times.h"
#include "log.h"

//按照不同的方式格式化当前times
void t_formats(char *s,const char*fmats)
{
	time_t now;
	time(&now);
	tm_now = localtime(&now);
	strftime(s, 20, fmats, tm_now);
}

//获取当前时间
void t_ftime(char *tsname)
{
	t_formats(tsname,"%Y%m%d%H%M%S");
}
//获取当前日期
void t_fdate(char *s)
{
	t_formats(s,"%Y%m%d");
}

//获取当前时间
void t_stime(char *times)
{
	t_formats(times,"%Y-%m-%d %H:%M:%S");
}

//执行时间
int disTime()
{
	//执行一次处理的时间
	gettimeofday(&tv,NULL);
	if(tv_prv.tv_sec==0)
	{
		tv_prv = tv;
		return 1;
	}
	//秒 微秒
	double second = (tv.tv_sec-tv_prv.tv_sec)+(double)(tv.tv_usec-tv_prv.tv_usec)/1000000;//秒

	//printfs("%s\t%d\t[DEBUG]:Seconds:%ld Microseconds:%ld Seconds:%ld Microseconds:%ld  耗时秒:%lf",__FILE__,__LINE__,tv.tv_sec,tv.tv_usec,tv_prv.tv_sec,tv_prv.tv_usec,second);

	tv_prv = tv;

	return 0;
}

//192.168.1.70
int ntpdate(const char *f_ip)
{
	char ntpd[64];
	snprintf(ntpd,sizeof(ntpd),"sudo /usr/sbin/ntpdate %s &",f_ip);
	system(ntpd);
	return 0;
}

//秒:纳秒
void sys_ms(char *ms)
{  
     struct timeval tv;  
     gettimeofday(&tv,NULL);  
     snprintf(ms, 30, "%d%ld",tv.tv_sec,tv.tv_usec);
}  

