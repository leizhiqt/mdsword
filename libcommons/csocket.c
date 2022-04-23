#include "csocket.h"

int client_connect(client_t* client)
{
	//fcntl( sockfd, F_SETEL, O_NONBLOCK );
	try:
	if((client->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
			printf("sockfd:%d to:%s port:%d  Failed\n",client->sockfd,inet_ntoa(client->serv_addr.sin_addr),
			ntohs(client->serv_addr.sin_port));
			
			if(client->sn>0 && --client->sn==0) return -1;
			
			sleep(10);
			goto try;
	}

	client->serv_addr.sin_family=AF_INET;
	client->serv_addr.sin_port=htons(client->port);
	client->serv_addr.sin_addr = *((struct in_addr *)client->host->h_addr);
 
	bzero(&(client->serv_addr.sin_zero),8);
 
	if (connect(client->sockfd, (struct sockaddr *)&client->serv_addr,sizeof(struct sockaddr)) == -1) {
		perror("connect error");
		return -1;
	}

	printfs("%s\t%d\t[DEBUG]:connect to:%s port:%d",__FILE__,__LINE__,inet_ntoa(client->serv_addr.sin_addr),ntohs(client->serv_addr.sin_port));

	return 0;
}

int client_send(client_t* client)
{
	int k=0;
	if((k=send(client->sockfd,client->buf,client->buf_size,0)) == -1)
	{
		perror("send error");
		return -1;
	}
	char buf[MAXDATASIZE];
	str_tohex(buf,client->buf_size*2,client->buf,client->buf_size);
	buf[client->buf_size*2]='\0';

	printfs("%s\t%d\t[DEBUG]:send\tk:%d sockfd:%d for:%s",__FILE__,__LINE__,k,client->sockfd,buf);

	return 0;
}


int client_recv(client_t* client)
{
	int k=0;
	if((k=recv(client->sockfd, client->buf, MAXDATASIZE, 0)) ==-1)
	{
		perror("recv error");
		return -1;
	}
	
	char buf[MAXDATASIZE];
	str_tohex(buf,client->buf_size*2,client->buf,client->buf_size);
	buf[client->buf_size*2]='\0';
	
	printfs("%s\t%d\t[DEBUG]:recv\tk:%d sockfd:%d for:%s",__FILE__,__LINE__,k,client->sockfd,buf);

	return 0;
}

void client_close(client_t* client)
{
	close(client->sockfd);
}


/////////////////////
int short_send(client_t* client)
{
	char buf[MAXDATASIZE];
	str_tohex(buf,client->buf_size*2,client->buf,client->buf_size);
	buf[client->buf_size*2]='\0';

	//fcntl( sockfd, F_SETEL, O_NONBLOCK );
	try:
	if((client->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
			printf("sockfd:%d to:%s port:%d  Failed\n",client->sockfd,inet_ntoa(client->serv_addr.sin_addr),
			ntohs(client->serv_addr.sin_port));
			
			if(client->sn>0 && --client->sn==0) return -1;
			
			sleep(10);
			goto try;
	}

	client->serv_addr.sin_family=AF_INET;
	client->serv_addr.sin_port=htons(client->port);
	client->serv_addr.sin_addr = *((struct in_addr *)client->host->h_addr);
 
	bzero(&(client->serv_addr.sin_zero),8);
 
	if (connect(client->sockfd, (struct sockaddr *)&client->serv_addr,sizeof(struct sockaddr)) == -1) {
		perror("connect error");
		goto release;
	}

	printfs("%s\t%d\t[DEBUG]:connect to:%s port:%d",__FILE__,__LINE__,inet_ntoa(client->serv_addr.sin_addr),ntohs(client->serv_addr.sin_port));

	int k=0;
	if((k=send(client->sockfd,client->buf,client->buf_size,0)) == -1)
	{
		perror("send error");
		goto release;
	}
	printfs("%s\t%d\t[DEBUG]:send\tk:%d sockfd:%d for:%s",__FILE__,__LINE__,k,client->sockfd,buf);

	if((k=recv(client->sockfd, client->buf, MAXDATASIZE, 0)) ==-1)
	{
		perror("recv error");
		goto release;
	}

	if(k>0)
	{
		str_tohex(buf,k*2,client->buf,k);
		buf[k*2]='\0';

		printfs("%s\t%d\t[DEBUG]:retrieve\tk:%d sockfd:%d for:%s",__FILE__,__LINE__,k,client->sockfd,buf);
	}

	release:
		close(client->sockfd);

	return 0;
}
