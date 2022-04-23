#define _XOPEN_SOURCE
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "sys/time.h"
#include "string.h"

#include "errno.h"
#include "unistd.h"
#include "pthread.h"

#include "sys/stat.h"
#include "sys/socket.h"
#include "sys/types.h"

#include "netdb.h"
#include "netinet/in.h"
#include "arpa/inet.h"

//#include "times.h"
#include "thpool.h"
#include "log.h"

#include "csocket.h"

/*
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "string.h"
#include "time.h"
#include "unistd.h"
#include "fcntl.h"
#include "signal.h"

#include "pthread.h"

#include "sys/time.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "sys/wait.h"
#include "dirent.h"

#include "linux/ip.h"

#include "mcheck.h"

#include "netinet/in.h"
#include "arpa/inet.h"
#include "asm/byteorder.h"

#include "zlib.h"

#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

#include "times.h"
#include "log.h"
#include "thpool.h"
#include "linklist.h"
#include "websocket.h"
#include "sfline.h"
*/
/*
int main(int argc, char** argv, char *envp[]) {

	char *s1 = "aaaaa";
	char *s2 = "bbbbbbbbb";
	//char **s ={"aaaaa","bbbbb","ddddd"};
	char **s = malloc(sizeof(char **));
	*s = s1;
	*(s+1) = s2;
	printf("%s %s \n",*s,*(s+1));
	return 0;
}*/

typedef struct _st
{
	int a;
	int b;
}st_t;

static st_t st;

void swap_value(int x,int y)
{
	int temp;
	temp = x;
	x = y;
	y = temp;   
	printf("传值函数内的输出 %d %d \n",x,y);
}

void swap_address(int *x,int *y)
{
	int temp;
	temp = *x;
	*x = *y;
	*y=temp;   
	printf("传址函数内的输出 %d %d \n",*x,*y);
}

void change(int **x){
	*x = malloc(sizeof(int));
	**x = 300;
	
	printf("传址函数内的输出 %p %p %d\n",x,*x,**x);
}

void change_val(int *x){
	int *p =x;
	*x = 234;
	//printf("传址函数内的输出 %p %p %d\n",x,*x,**x);
}

void change2(char *s,int size){
	snprintf(s,size,"%s","987654321");
}

void change1(char *s,int size){
	snprintf(s,size,"%s","123456789");
	change2(s,size);
}



/*
int* function() {
	char buffer[4];//4 bytes 8bytes
	//int ax=100;
	int *ret;//4 bytes 8bytes
	//rbp 4 bytes=12  8bytes =24
	ret = (int*)(buffer + 24); //64bits 24  32bits 12
	(*ret) += 7; //x86=7 arm=4

	register int a = 88;
	*/
/*
	__asm__ __volatile__("\
		movl $1,%eax\n\
		xor %rbx,%rbx\n\
		int $0x80"
		);*/
/*
	__asm__ __volatile__(
		"movl $10,%%eax\n"
		"movl $0x10,%%eax\n"
		//"mov %%rip,%%rax\n"
		:"=a"(a)
		);

	printf("a = %d\n",a);

	return ret; //popl rip
}
*/
/*
int fx() { 
	int x; 
	x = 20; 
	function();//push rip
	x = 1;
	printf("x=%d\n",x); 
}*/

/*int ch1(match_video_t *p) { */
/*	p->matching=5555.55;*/
/*	printf("ch1 x=%0.4lf\n",p->matching); */
/*}*/

/*int ch(match_video_t *p) { */
/*	p->matching=1234.55;*/
/*	//ch1(p);*/
/*	printf("ch x=%0.4lf\n",p->matching); */
/*}*/

void display(void *v)
{
	int *p = (int *)v;
	printf("main x=%d %d %d\n",*p,*(p+1),*(p+2));
}

int setdate(char *s0,char *s1)
{
	struct tm tm0;
	struct tm tm1;

	char buf[255];

	memset(&tm0, 0, sizeof(struct tm));
	strptime(s0, "%Y%m%d", &tm0);

	memset(&tm1, 0, sizeof(struct tm));
	strptime(s1, "%Y%m%d", &tm1);
	
	strftime(buf,8, "%Y%m%d", &tm0);
	strftime(buf,8, "%Y%m%d", &tm1);
	
	long unixtime0 = mktime(&tm0);
	long unixtime1 = mktime(&tm1);

	//time_t curTime;
	//time(&curTime);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm0);

	printf("%s %ld %ld\n",buf,unixtime0,unixtime1);

	return 0;
}

void m_malloc(){
	int i,j;
	i=9;
	j=5;
	char ch1[i][j];
	
	char **pp = malloc(sizeof(char *)*i);
	for(int i1=0;i1<i;i1++){
		*(pp+i1) = malloc(sizeof(char)*(i1+1));
		snprintf(*(pp+i1),i1+1,"%s","abcdefghijklmn");
	}

	for(int i1=0;i1<i;i1++){
		printf("%s\n",*(pp+i1));
		free((*(pp+i1)));
	}
	
	free(pp);
/*
	for(int i1=0;i1<i;i1++){
		for(int j1=0;j1<j;j1++){
			ch1[i1][j1]=i1*10+j1;
		}
	}

	for(int i1=0;i1<i;i1++){
		for(int j1=0;j1<j;j1++){
			printf("%d %d %d\n",i1,j1,ch1[i1][j1]);
		}
	}

	printf("================\n");
	char (*p)[j] = ch1;

	for(int i1=0;i1<i;i1++){
		for(int j1=0;j1<j;j1++){
			printf("%d %d %d\n",i1,j1,*(*(p+j1)+i1));
		}
	}*/

	/*printf("================\n");
	char (*p1)[i][j] = &ch1;
	for(int i1=0;i1<i;i1++){
		for(int j1=0;j1<j;j1++){
			printf("%d %d %d\n",i1,j1,*(*(p1+j1)+i1));
		}
	}*/

	//printf("传址函数内的输出 %p %p %d\n",x,*x,**x);
}

void nprint(){
	char s1[128];
	snprintf(s1,sizeof(s1),"%s/%s","100","1000");
	printf("%s\n",s1);
}
void ttoken(){
	char buf[]=": : : : : :";
	char *token=strtok(buf,":");
	int k=0;
	printfs("%s\t%d\t[DEBUG]: recv_cmd ERROR k:%d token:%s",__FILE__,__LINE__,k,token);
	while(token!=NULL){
		printfs("%s\t%d\t[DEBUG]: recv_cmd ERROR k:%d token:%s",__FILE__,__LINE__,k,token);
		token=strtok(NULL,":");
		k++;
	}
}

void tstrstr(){
	char buf[]="12.jpg";
	char *p=strstr(buf,".jpg");
	printfs("%s\t%d\t[DEBUG]: tstrstr:%s",__FILE__,__LINE__,p);
}

typedef struct _people
{
	int age;
	char name[64];
}people_t;

void *case_start(void *v)
{
	//int k = *(int *)v;
	
	people_t people = *(people_t *)v;
	
	pthread_t pid = getpid();
	pthread_t tid = pthread_self();
	
	printfs("%s\t%d\t[DEBUG]:case_start 开始:pid:%u pthread_id:%u (0x%x)",__FILE__,__LINE__,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
	
	while(1)
	{
		printfs("%s\t%d\t[DEBUG]:case_start 运行:k:%d pid:%u pthread_id:%u (0x%x)",__FILE__,__LINE__,people.age,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
		sleep(1);
	}
	printfs("%s\t%d\t[DEBUG]:case_start 结束:pid:%u pthread_id:%u (0x%x)",__FILE__,__LINE__,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
}

void send_plc(char* cmd,int c_size)
{
	client_t client;
	client.host=gethostbyname("192.168.1.50");
	client.port=4196;

	memset(client.buf, '\0',sizeof(client.buf));
	client.buf_size=c_size;

	memcpy(client.buf,cmd,c_size);

	short_send(&client);
}

void open_plc()
{
	//01 05 00 10 ff 00 8d ff	开
	//01 05 00 10 00 00 cc 0f	关
	char open_plc[] = "\x01\x05\x00\x10\xff\x00\x8d\xff";
	char close_plc[] = "\x01\x05\x00\x10\x00\x00\xcc\x0f";

	send_plc(open_plc,8);

	sleep(1);
	send_plc(close_plc,8);
}

int main(int argc, char** argv, char *envp[]) {
/*	char buf[8];*/
/*	change1(buf,sizeof(buf)-1);*/
/*	printfs("%s\t%d\t[DEBUG]:%s",__FILE__,__LINE__,buf);*/
	
	open_plc();
	
/*	struct tm tm;
	char buf[255];

	memset(&tm, 0, sizeof(struct tm));
	strptime("2017-11-12 18:31:01", "%Y-%m-%d %H:%M:%S", &tm);
	//strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
	long unixtime = mktime(&tm);

	time_t curTime;
	time(&curTime);
	printf("%s %ld %ld\n",buf,unixtime,curTime);*/
	//setdate("20171112","20171112");
	/*while(1)
	{
	m_malloc();
	}*/
	//nprint();
	//ttoken();
	//tstrstr();
	//线程池
/*	thpool_t thpool;*/
/*	*/
/*	thpool_init(&thpool,25,5);*/
/*	//usleep(50);*/
/*	sleep(1);*/
/*	*/
/*	people_t people;*/
/*	*/
/*	for(int i=0;i<25;i++)*/
/*	{*/
/*		 printfs("%s\t%d\t[DEBUG]:test_case[%d]",__FILE__,__LINE__,i);*/
/*		 people.age=i;*/
/*		 */
/*		 if(thpool_add_work(&thpool,case_start,&people)<0)*/
/*		 {*/
/*			 printfs("%s\t%d\t[DEBUG]:thpool full",__FILE__,__LINE__);*/
/*		 }*/
/*		 sleep(1);*/
/*	}*/
/*	*/
/*	printf("thpool_destroy\n");*/
/*	thpool_destroy(&thpool);*/

/*	//等待其他线程执行完*/
/*	sleep(1);*/
/*	printf("Main App exit\n");*/
	return 0;
}

//01:102:18,test4:02
//01:104:10,fulltest:0201:102:18,test4:02
int main__(int argc, char** argv, char *envp[]) {
	/*int k = exist_line("data/137/1/1.txt","2017-03-01 17:07:13");
	printf("main x=%d\n",k);
	k = exist_line("data/137/1/1.txt","2017-03-01 17:07:08");
	printf("main x=%d\n",k);*/
	/*char *s = "HTTP/1.1 101 Switching Protocols\
Upgrade: websocket\
Connection: Upgrade\
Sec-WebSocket-Accept: bUhrHw==";
	int k = recv_protocol(s);
	printf("recv_protocol=%d\n",k);*/
	
	//tarz("data/74/3/","data/74/3.tar.gz");
	//gzip_d("test","test.zip");
	//
	/*char buf[80];
	set_save_path((char *)&buf);
	printf("task_flush=%s\n",buf);
	*/
	//mkdir_p("data/tmp/abcd");
	/*link_list_t *errors=NULL;//传指针的地址
	linklist_init(&errors);
	linklist_push(&errors,"1");
	linklist_push(&errors,"2");*/
/*	linklist_push(&errors,"1");
	linklist_init(&(errors->msg));
	linklist_push(&(errors->msg),"11");
	linklist_push(&(errors->msg),"12");
	printf("task_flush=%d\n",linklist_size(&(errors->msg)));

	linklist_push(&errors,"2");
	linklist_init(&(errors->msg));
	linklist_push(&(errors->msg),"21");
	linklist_push(&(errors->msg),"22");
	linklist_push(&(errors->msg),"23");*/
	/*printf("task_flush=%d\n",linklist_size(&errors));

	linklist_destroy(&errors);*/

/*	task_init(10);
	int k = task_flush("abcd");
	printf("task_flush=%d\n",task_exist("abcd3"));
	printf("task_flush=%d\n",task_exist("abcd"));
	k=task_flush("abcd");
	printf("task_flush=%d\n",k);
	k=task_flush("abcd");
	printf("task_flush=%d\n",k);
	k=task_flush("abcd3");
	printf("task_flush=%d\n",k);
	k=task_flush("abcd5");
	printf("task_flush abcd5=%d\n",task_exist("abcd3"));

	printf("task1=%d\n",task_size());
	task_remove("abcd3");
	task_remove("abcd5");
	task_remove("abcd");
	task_remove("abcd");
	printf("task2=%d\n",task_size());
	task_destroy();
	printf("task3=%d\n",task_size());
	*/
/*	char ch[] = "01:104:10,fulltest:02\n\rbbb\n\r";*/
/*	printfs("%s\t%d\t[DEBUG]: received analyData:%s< %d",__FILE__,__LINE__,ch,strlen(ch));*/
/*	brline(ch,strlen(ch));*/
/*	printfs("%s\t%d\t[DEBUG]: received analyData:%s< %d",__FILE__,__LINE__,ch,strlen(ch));*/
	
	//printfs("%s\t%d\t[DEBUG]: received analyData:%d",__FILE__,__LINE__,recv_protocol(ch));
	//char *ch = "01:101:154,82,288,254:02";
	//char *ch = "01:100:rrrr:02";
	//recv_cmd(ch);
	/*char buf[] = "abcd0";
	char *p = buf;
	for(int i=0;i<sizeof(buf);i++)
	printfs("%s\t%d\t[DEBUG]: received analyData:0x%02X",__FILE__,__LINE__,*(p+i));*/
	/*task_match_t task_match;
	task_match.matching = 999.999;
	ch(&task_match);
	printfs("%s\t%d\t[DEBUG]: p->matching:%0.4lf",__FILE__,__LINE__,task_match.matching);
	*/
	/*int i[3]={'2','3','4'};
	task_match_t task_match;
	
	printf("main x=%d\n",sizeof(task_match.msg));
	*/
	//printbytes((char *)&i,3);
	/*int i[3]={2,3,4};
	display(i);
	printf("main x=%d %d %d\n",i[0],i[1],i[2]);*/
	//char *fname = ex();
	//printf("main fname=%s\n",fname);
	/*int x=90;
	int *p = &x;
	change_val(p);
	printf("main x=%d\n",x);*/
	/*task_match_t mmatch;
	mmatch.matching = 20.1234;
	printf("main x=%0.4lf\n",mmatch.matching); 
	ch(&mmatch);
	printf("main x=%0.4lf\n",mmatch.matching);*/
	
	/*char buf[64];
	char s1[] = "NO. warning!!!!";
	snprintf(buf,3,"%s","NO. warning!!!!");
	printf("buf:%s\n",buf);
	*/
	
	//printf("main x=%d\n",abs(2,3));
	/*int x = 1;
	int y = 2;

	printf("x y \n");
	printf("初值 %d %d \n",x,y);
	//传值子程序调用(交换xy) 
	swap_value(x,y);
	printf("传值函数外调用 %d %d \n",x,y);

	int* x1=NULL;
	
	printf("传值函数外调用 %p \n",x1);
	change(&x1);
	printf("传值函数外调用 %p %d\n",x1,*x1);
	
	//传地址字程序调用(交换x,y) 
	swap_address(&x,&y);
	printf("传址函数外调用 %d %d \n",x,y);
	
	int *p = &x;
	change_val(p);
	printf("传址函数外调用 %d %d \n",x,y);*/
	//fx();
	return 0;
}
