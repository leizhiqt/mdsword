#ifndef _TIMES_H_
#define _TIMES_H_

//时间

//秒
struct tm *tm_now;

//毫秒 微秒
struct timeval tv;
struct timeval tv_prv;

time_t now;

char datetime[20];

//按照不同的方式格式化当前times
void t_formats(char *s,const char*fmats);
//获取当前时间
void t_ftime(char *tsname);
//获取当前日期
void t_fdate(char *s);
//获取当前时间
void t_stime(char *tsname);

//执行时间
int disTime();

int ntpdate(const char *f_ip);

void sys_ms(char *ms);

#endif
