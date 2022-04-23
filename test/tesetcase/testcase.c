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

#include "sfline.h"
#include "dtype.h"
#include "thpool.h"
#include "log.h"

#define MAXDATASIZE 2048 /*每次最大数据传输量 */

//开辟线程池
static	thpool_t thpool;

//测试包
//static char case_buf[MAXDATASIZE];

static int max_cases;

//static pthread_mutex_t lock_socket;

//客户端
typedef struct _client client_t;
typedef struct _client{
	int sockfd;
	int port;
	struct hostent *host;
	struct sockaddr_in serv_addr;
	pthread_mutex_t lock;
	int exit_socket;
	int sn;//send Number of times
	
	uchar buf[MAXDATASIZE];
	int buf_size;
}client_t;
 
void *console()
{
	char sendBuf[MAXDATASIZE]="Hello, you are connected!";

	while(1){
		printf("client:");
		fgets(sendBuf, MAXDATASIZE, stdin);
		if(strcmp(sendBuf,"close")==0) break;
		sleep(2);
	}
	sleep(2);

	pthread_exit(NULL);
}

//发送线程
void *send_thread(void *v)
{
	//**(int**)&v 二维数组
	client_t* client = (client_t *)v;

	printf(">>send_thread to:%s port:%d sockfd:%d buf:%s\n",inet_ntoa(client->serv_addr.sin_addr),
			ntohs(client->serv_addr.sin_port),client->sockfd,client->buf);

	int k=0;
	while(1)
	{
	 	if(strlen(client->buf)>0)
		{
			if(strcmp(client->buf,"close")==0)
				break;
			
			pthread_mutex_lock(&client->lock);
			
			if(client->exit_socket) break;
			
			k=0;
			if((k=send(client->sockfd,client->buf, client->buf_size, 0)) == -1)
			{
				perror("send error");
				client->exit_socket = 1;
				break;
			}

			pthread_mutex_unlock(&client->lock);

			*(client->buf+k) = '\0';
			printf("send k:%d sockfd:%d for:%s\n",k,client->sockfd,client->buf);
			
			sleep(1);
		}
	}

	sleep(1);
	printf("exit send_thread\n");
	//pthread_exit(NULL);
}

//接收线程
void *retrieve_thread(void *v)
{
	client_t* client = (client_t *)v;
	printf("<<retrieve_thread sockfd:%d\n",client->sockfd);
	
	int k=0;
	char buf[MAXDATASIZE];
	
	while(1)
	{
		pthread_mutex_lock(&client->lock);
		
		if(client->exit_socket) break;
		//printf("while retrieve_thread sockfd:%d\n",client->sockfd);
		k=0;
		if ((k=recv(client->sockfd, buf, MAXDATASIZE, 0)) ==-1)
		{
			perror("recv error");
			client->exit_socket = 1;
			break;
		}
		pthread_mutex_unlock(&client->lock);

		buf[k] = '\0';
		printf("retrieve k:%d sockfd:%d for:%s\n",k,client->sockfd,buf);
		
		sleep(1);
	}

	sleep(1);

	printfs("%s\t%d\t[DEBUG]:exit retrieve_thread:%d",__FILE__,__LINE__,client->sockfd);
	//pthread_exit(NULL);
}

//客户端线程
void *case_pthread(void *v)
{
	client_t client = *(client_t *)v;
	client.exit_socket = 0;
	
	//fcntl( sockfd, F_SETEL, O_NONBLOCK );
	if ((client.sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket error");
		pthread_exit(NULL);
	}
	
	client.serv_addr.sin_family=AF_INET;
	client.serv_addr.sin_port=htons(client.port);
	client.serv_addr.sin_addr = *((struct in_addr *)client.host->h_addr);
 
	bzero(&(client.serv_addr.sin_zero),8);
 
	if (connect(client.sockfd, (struct sockaddr *)&client.serv_addr,sizeof(struct sockaddr)) == -1) {
		perror("connect error");
		goto release;
	}
 
	 printf("connect to:%s port:%d\n",inet_ntoa(client.serv_addr.sin_addr),ntohs(client.serv_addr.sin_port));

	pthread_mutex_init(&client.lock,NULL);//初始化互斥锁
	 
	//send_thread 加入任务队列
	 pthread_t s_thread,r_thread;
	 if((s_thread=thpool_add_work(&thpool,send_thread, (void*)&client))<0)
	 {
		 printfs("%s\t%d\t[DEBUG]:thpool full",__FILE__,__LINE__);
		 goto release;
	 }
	 
	 //retrieve_thread 加入任务队列
	 if((r_thread=thpool_add_work(&thpool,retrieve_thread, (void*)&client))<0)
	 {
		 printfs("%s\t%d\t[DEBUG]:thpool full",__FILE__,__LINE__);
		 goto release;
	 }
	 
	pthread_join(s_thread,NULL);
	pthread_join(r_thread,NULL);
	 
	//pthread_mutex_destroy(&lock_socket);
	 release:
		close(client.sockfd);
	 //   free(client);
}

//顺序流程测试
void *teset_case_pthread(void *v)
{
	client_t client = *(client_t *)v;

	pthread_t pid = getpid();
	pthread_t tid = pthread_self();

	printfs("%s\t%d\t[DEBUG]:case_start 开始:pid:%u pthread_id:%u (0x%x)",__FILE__,__LINE__,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
	
	while(1)
	{
		//fcntl( sockfd, F_SETEL, O_NONBLOCK );
		if((client.sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
				printf("sockfd:%d to:%s port:%d  Failed\n",client.sockfd,inet_ntoa(client.serv_addr.sin_addr),
				ntohs(client.serv_addr.sin_port));
				goto try;
		}
		
		//printfs("%s\t%d\t[DEBUG]:case_start 运行:csockfd:%d pid:%u pthread_id:%u (0x%x)",__FILE__,__LINE__,client.sockfd,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
		
		client.serv_addr.sin_family=AF_INET;
		client.serv_addr.sin_port=htons(client.port);
		client.serv_addr.sin_addr = *((struct in_addr *)client.host->h_addr);
	 
		bzero(&(client.serv_addr.sin_zero),8);
	 
		if (connect(client.sockfd, (struct sockaddr *)&client.serv_addr,sizeof(struct sockaddr)) == -1) {
			perror("connect error");
			goto release;
		}

		printfs("%s\t%d\t[DEBUG]:connect to:%s port:%d",__FILE__,__LINE__,inet_ntoa(client.serv_addr.sin_addr),ntohs(client.serv_addr.sin_port));
		
		printf(">>send_thread to:%s port:%d sockfd:%d buf:%s\n",inet_ntoa(client.serv_addr.sin_addr),
				ntohs(client.serv_addr.sin_port),client.sockfd,client.buf);

		int k=0;
	 
		if((k=send(client.sockfd,client.buf,client.buf_size,0)) == -1)
		{
			perror("send error");
			goto release;
		}
		printf("send\tk:%d sockfd:%d for:%s\n",k,client.sockfd,client.buf);
		
		if((k=recv(client.sockfd, client.buf, MAXDATASIZE, 0)) ==-1)
		{
			perror("recv error");
			goto release;
		}
		
		printf("retrieve\tk:%d sockfd:%d for:%s\n",k,client.sockfd,client.buf);
			
		release:
			close(client.sockfd);
		try:
			if(client.sn>0 && --client.sn==0) break;
			sleep(10);
	}
	
	printf("exit send_thread sockfd:%d\n",client.sockfd);
	printfs("%s\t%d\t[DEBUG]:case_start 结束:pid:%u pthread_id:%u (0x%x)",__FILE__,__LINE__,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
}

void test_start(client_t* client)
{
	 /*
	 //console 加入任务队列
	 if(thpool_add_work(&thpool,console, NULL)<0)
	 {
		 printfs("%s\t%d\t[DEBUG]:thpool full",__FILE__,__LINE__);
	 }*/

	for(int i=0;i<max_cases;i++)
	{
		 printfs("%s\t%d\t[DEBUG]:test_case[%d]",__FILE__,__LINE__,i);

		 //case_pthread 加入任务队列
		 //case_start 加入任务队列
		 if(thpool_add_work(&thpool,teset_case_pthread,client)<0)
		 {
			 printfs("%s\t%d\t[DEBUG]:thpool full",__FILE__,__LINE__);
		 }
		 sleep(1);
	}
	printfs("%s\t%d\t[DEBUG]:start_test finsh",__FILE__,__LINE__);
}

void test_init()
{
	max_cases = 10;
}

void api_send()
{
	client_t client;
	client.host=gethostbyname("192.168.1.93");
	client.port=10088;
	//01 05 00 10 ff 00 8d ff	开
	//01 05 00 10 00 00 cc 0f	关
	char open_plc[] = "\x01\x05\x00\x10\xff\x00\x8d\xff\0";
	char close_plc[] = "\x01\x05\x00\x10\x00\x00\xcc\x0f\0";
	
	snprintf(client.buf,sizeof(client.buf),"%s\n",open_plc);

	teset_case_pthread((void *)&client);
}
	
void usage()
{
	 fprintf(stdout, "\ttestcase -h192.168.1.93 -p10088 -sn0 [-ms|-mx]01:120:02  -n10\n");
	 fprintf(stdout, "\ttestcase -h192.168.1.93 -p10088 -sn1 -mx01050010ff008dff -n10\n");
	 fprintf(stdout, "\ttestcase -h192.168.1.93 -p10088 -sn1 -mx010500100000cc0f -n10\n");
}

int main(int argc, char** argv, char *envp[]) {

	if (argc < 3)
	{
		usage();
		exit(1);
	}

	client_t client;

	char buf[64] = {0};
	int k = 0;
	while (argc > k) {
		//printf("%s\n",argv[k]);

		if (strstr(argv[k],"-h") == argv[k]){
			printfs("%s\t%d\t[DEBUG]:-h:%s",__FILE__,__LINE__,argv[k]+2);
			if((client.host=gethostbyname(argv[k]+2))==NULL)
			{
				herror("gethostbyname error");
				return -1;
			}
		}
		
		if (strstr(argv[k],"-p") == argv[k]){
			printfs("%s\t%d\t[DEBUG]:-p:%s",__FILE__,__LINE__,argv[k]+2);
			if((client.port=atoi(argv[k]+2))<0)
			{
				herror("gethostbyname error");
				return -2;
			}
		}
		
		if (strstr(argv[k],"-sn") == argv[k]){
			printfs("%s\t%d\t[DEBUG]:-ms:%s",__FILE__,__LINE__,argv[k]+3);
			client.sn = atoi(argv[k]+3);
		}
		
		if (strstr(argv[k],"-mx") == argv[k]){
			printfs("%s\t%d\t[DEBUG]:-mx %s",__FILE__,__LINE__,argv[k]+3);

			memset(client.buf,'\0',sizeof(client.buf));
			str_tostr(client.buf,argv[k]+3);
			client.buf_size = strlen(argv[k]+3)/2;
			
			/*char buf[1024];
			str_tohex(buf,client.buf_size*2,client.buf,client.buf_size);
			printfs("%s\t%d\t[DEBUG]:-ms:%s",__FILE__,__LINE__,buf);*/
			
			teset_case_pthread((void *)&client);
			return 1;
		}

		if (strstr(argv[k],"-ms") == argv[k]){
			printfs("%s\t%d\t[DEBUG]:-ms:%s",__FILE__,__LINE__,argv[k]+3);

			memset(client.buf,'\0',sizeof(client.buf));
			snprintf(client.buf,sizeof(client.buf),"%s\n",argv[k]+3);
			client.buf_size = strlen(argv[k]+3);

			teset_case_pthread((void *)&client);
			return 2;
		}

		if (strstr(argv[k],"-n") == argv[k]){
			printfs("%s\t%d\t[DEBUG]:-ms:%s",__FILE__,__LINE__,argv[k]+2);
			max_cases = atoi(argv[k]+2);
		}

		k++;
	}
	printf("\n");
	
	test_init();//参数初始化

	//线程池
	thpool_init(&thpool,75,5);
	usleep(50);

	//启动测试用例
	test_start(&client);

	printf("start finsh\n");
	
	thpool_destroy(&thpool);
	printf("thpool_destroy\n");
	
	//等待其他线程执行完
	usleep(1000);

	printf("Main App exit\n");
	
	return 0;
}
