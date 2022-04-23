/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:  client to link the server
 *
 *        Version:  1.0
 *        Created:  2013/01/11 10时13分18秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */
/*
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

#include "errno.h"
#include "unistd.h"
#include "pthread.h"

#include "sys/stat.h"
#include "sys/socket.h"
#include "sys/types.h"

#include "netdb.h"
#include "netinet/in.h"
#include "arpa/inet.h"

#include "dtype.h"
#include "thpool.h"
#include "log.h"

#define MAXBUF 100

#define MAXDATASIZE 2048 /*每次最大数据传输量 */

//开辟线程池
static	thpool_t thpool;
static int max_cases;

void *teset_case_pthread(void *v)
{
	//client_t client = *(client_t *)v;

	pthread_t pid = getpid();
	pthread_t tid = pthread_self();

	printfs("%s\t%d\t[DEBUG]:case_start 开始:pid:%u pthread_id:%u (0x%x)",__FILE__,__LINE__,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);

	char buf[MAXDATASIZE];
	memset(buf,'\0',sizeof(buf));

	char *server = "127.0.0.1";
	char *serverport = "2514";
	char *echostring = "helloworld";

	struct addrinfo client,*servinfo;
	
	memset(&client,0,sizeof(client));
	client.ai_family = AF_INET;
	client.ai_socktype = SOCK_DGRAM;
	client.ai_protocol= IPPROTO_UDP;

	if(getaddrinfo(server,serverport,&client,&servinfo)<0)
	{
		printf("error in getaddrinfo\n");
		exit(-1);
	}

	int sockfd = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);
	if(sockfd <0)
	{
		printf("error in socket create\n");
		exit(-1);
	}
	
	char  echomsg[1024];
	snprintf(echomsg,sizeof(echomsg),echostring);
	
	ssize_t numBytes=0;
	numBytes = sendto(sockfd,echomsg,strlen(echomsg),0,servinfo->ai_addr,servinfo->ai_addrlen);
	if(numBytes<0)
	{
		printf("error in send the data\n");
	}
	printf("send the data %s\n",echomsg);

	//接收数据
	/*
		numBytes=0;
		memset(buf,'\0',sizeof(buf));
		numBytes = recvfrom(sockfd,buf,MAXBUF+1,0,(struct sockaddr *)&fromaddr,&fromaddrlen);
		char *bbuf=buf+1;
		if(buf[0]==2)
		{
			printf("recv msg from server %s\n",bbuf);
		}
		//printf("Received:%s\n",bbuf);
	*/

	//接收服务端信息
	/*
		struct sockaddr_storage fromaddr;
		socklen_t fromaddrlen = sizeof(fromaddr);
	
		char buf[MAXBUF+1];
		numBytes = recvfrom(sockfd,buf,MAXBUF+1,0,(struct sockaddr *)&fromaddr,&fromaddrlen);
		if(buf[0]==1)
		{
			printf("connected to the server\n");
		}
	*/

	freeaddrinfo(servinfo);
	close(sockfd);
	
	printfs("%s\t%d\t[DEBUG]:case_start 结束:pid:%u pthread_id:%u (0x%x)",__FILE__,__LINE__,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
}

void test_init()
{
    max_cases = 10;
}

void test_start()
{

    for(int i=0;i<max_cases;i++)
    {
 	printfs("%s\t%d\t[DEBUG]:test_case[%d]",__FILE__,__LINE__,i);
         //case_pthread 加入任务队列
         //case_start 加入任务队列
         if(thpool_add_work(&thpool,teset_case_pthread,NULL)<0)
         {
	         printfs("%s\t%d\t[DEBUG]:thpool full",__FILE__,__LINE__);
         }
         //sleep(1);
    }
    printfs("%s\t%d\t[DEBUG]:start_test finsh",__FILE__,__LINE__);
}

void usage()
{
     fprintf(stdout, "\t 40000000\n");
}

int main(int argc, const char *argv[])
{
teset_case_pthread(NULL);
/*
	if (argc < 2)
	{
	  usage();
	  exit(1);
	}

	test_init();//参数初始化

	max_cases = atoi(argv[1]);

	//线程池
	thpool_init(&thpool,75,5);
	usleep(50);

	//启动测试用例
	test_start();

	printf("start finsh\n");

	thpool_destroy(&thpool);
	printf("thpool_destroy\n");

	//等待其他线程执行完
	usleep(1000);

	printf("Main App exit\n");
	*/
	return 0;
}

