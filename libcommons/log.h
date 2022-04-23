#ifndef _HLOG_

#define _HLOG_

void logInit(const char *s,int level);

void logsk(const int sk);

void printfs(const char * fmt, ...);

void printbytes(const char * bytes,const int n);

#endif
//demo
//printfs("%s\t%d\t[DEBUG]: argc=%d",__FILE__,__LINE__,argc);
