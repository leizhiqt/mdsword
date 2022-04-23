#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "unistd.h"
#include "pthread.h"
#include "signal.h"
#include "string.h"
#include "fcntl.h"
 
#include "sys/stat.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "sys/epoll.h"
 
#include "arpa/inet.h"
#include "netdb.h"
  
#include "dtype.h"
#include "log.h"
#include "times.h"
#include "thpool.h"
#include "sfline.h"

#define ECHO_LEN	1025
 
#define MAX_EVENTS 32768

#define MAXBUF 1024

#define MAX_EPOLLSIZE 1024

#define NI_MAXHOST  1025
#define NI_MAXSERV	32

#define MY_UDP_PORT	2514

#define MAXDATASIZE 2048 /*每次最大数据传输量 */

#define K 1024
#define M 1024 * K
#define G 1024 * M

static uchar *bbuf;
static uchar *wp;

static uint forever;

thpool_t thpool;
	
//UDP模式
int epoll_process()
{
	bbuf = malloc(G);
	
	printfs("%s\t%d\t[DEBUG]:malloc %p",__FILE__,__LINE__,bbuf);
	
	memset(bbuf,'\0',G);
	wp = bbuf;

	int listenfd,epollfd, nfds, n, curfds;
	socklen_t len;
	struct sockaddr_in my_addr, their_addr;

	struct epoll_event ev;
	struct epoll_event events[MAX_EPOLLSIZE];

//SOCK_STREAM         Provides sequenced, reliable, two-way, connection-based byte streams.   //用于TCP
//SOCK_DGRAM          Supports datagrams (connectionless, unreliable messages ). //用于UDP
//SOCK_RAW              Provides raw network protocol access.  //RAW类型，用于提供原始网络访问

	/* 开启 socket 监听 */
	//AF_INET IPV4
	//PF_INET IPV6
	if ((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket create failed ！\n");
		exit(1);
	}
	//printf("socket create  success \n");


	/*设置socket属性，端口可以重用*/
	int opt=SO_REUSEADDR;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	
	//setnonblocking – 设置句柄为非阻塞方式
	if (fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFD, 0)|O_NONBLOCK) == -1)
	{
		return -1;
	}
   
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = PF_INET;
	my_addr.sin_port = htons(MY_UDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(listenfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("bind");
		exit(1);
	} 
	//printf("IP and port bind success \n");

	/* 创建 epoll 句柄，把监听 socket 加入到 epoll 集合里 */
	epollfd = epoll_create(MAX_EPOLLSIZE);
	len = sizeof(struct sockaddr_in);
	ev.events = EPOLLIN;
	ev.data.fd = listenfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) 
	{
		fprintf(stderr, "epoll set insertion error: fd=%d\n", listenfd);
		return -1;
	}
	//printf("listen socket added in  epoll success \n");

	char buf[4096];

	long ec=0;
	forever=1;
	while (forever) 
	{
		/* 等待有事件发生 */
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1)
		{
			 perror("epoll_wait");
			 break;
		}
		//printf("捕捉事件数:%d\n",nfds);
		/* 处理所有事件 */
		for (int n = 0; n < nfds; ++n)
		{
			//events[n].events & EPOLLIN
			//printfs("%s\t%d\t[DEBUG]:events[n].events:%d events[n].data.fd:%d listenfd:%d",__FILE__,__LINE__,(events[n].events & EPOLLIN),events[n].data.fd,listenfd);
			//printfs("%s\t%d\t[DEBUG]:EPOLLIN:文件描述符可以读:%d EPOLLOUT:文件描述符可以写:%d EPOLLPRI:带外数据到来:%d EPOLLERR:文件描述符发生错误:%d EPOLLHUP:文件描述符被挂断:%d EPOLLET:EPOLL设为边缘触发:%d EPOLLONESHOT:只监听一次事件:%d",__FILE__,__LINE__,EPOLLIN,EPOLLOUT,EPOLLPRI,EPOLLERR,EPOLLHUP,EPOLLET,EPOLLONESHOT);

			//printfs("%s\t%d\t[DEBUG]:EPOLLIN:%d EPOLLOUT:%d EPOLLPRI:%d EPOLLET:%d EPOLLPRI:%d EPOLLHUP:%d",__FILE__,__LINE__,(events[n].events & EPOLLIN),events[n].data.fd,listenfd);
			//EPOLLIN		表示对应的文件描述符可以读
			//EPOLLOUT		表示对应的文件描述符可以写
			//EPOLLPRI		表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）
			//EPOLLERR		表示对应的文件描述符发生错误
			//EPOLLHUP		表示对应的文件描述符被挂断
			//EPOLLET		将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的
			//EPOLLONESHOT	只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里
	
			if (events[n].data.fd == listenfd) 
			{
				if(events[n].events & EPOLLIN){//只要socket读缓存中还有未读的数据，这段代码就会触发
					struct sockaddr_storage client_addr;
					socklen_t addr_size = sizeof(client_addr);
			
					//读取UDP报头
					memset(buf,'\0',sizeof(buf));

					int ret = recvfrom(listenfd, buf,sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_size);
					//printfs("%s\t%d\t[DEBUG]:accept_udp pack:%s %d",__FILE__,__LINE__, buf,ret);
					if(ret>0){
						if(buf[ret-1]=='\n'){
							buf[ret] = '\0';
							
							if(wp+ret+1 < bbuf+G){
								memcpy(wp,buf,ret);
								wp +=  ret;
							}
						}else{
							buf[ret] = '\n';
							buf[ret+1] = '\0';
							
							if(wp+ret+2 < bbuf+G){
								memcpy(wp,buf,ret+1);
								wp +=  ret+1;
							}
						}
					}
				}
				//printfs("%s\t%d\t[DEBUG]:listenfd:%d",__FILE__,__LINE__,listenfd);
			}else{
		        printfs("%s\t%d\t[DEBUG]:something else happened",__FILE__,__LINE__);
			}
		}
	}
	close(listenfd);
	
	printf("epoll_process exit \n");
	
	return 0;
}

void *console()
{
    char sendBuf[MAXDATASIZE];

	fprintf(stdout, "====Welcome lepoll console=====\n");
    while(1){
        printf("console$>");
        fgets(sendBuf, MAXDATASIZE, stdin);
        
        if(strcmp(sendBuf,"exit\n")==0){
        	forever = 0;
        	break;
        }
        
        if(strcmp(sendBuf,"w\n")==0){
        	save_append("warn.txt",bbuf);
        	printf("cache save to warn.txt done\n");
        }

		if(strcmp(sendBuf,"show\n")==0){
        	printf("console$>%ld bytes\n",(wp-bbuf));
        }
        
        usleep(50);
    }
	printf("console$>bye!\n");
    sleep(2);
    //pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	//线程池
	thpool_init(&thpool,75,25);
	usleep(50);

	//启动测试用例
	//test_start(&client);
	if(thpool_add_work(&thpool,console,NULL)<0)
	{
		printfs("%s\t%d\t[DEBUG]:thpool full",__FILE__,__LINE__);
	}
	sleep(1);

	epoll_process();
	
	printf("start finsh\n");

	thpool_destroy(&thpool);
	printf("thpool_destroy\n");

	//等待其他线程执行完
	usleep(1000);

	printf("Main App exit\n");

	return 0;
}

