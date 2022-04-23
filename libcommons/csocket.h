/*
	Client API
*/

#ifndef _CSOCKET_H_
#define _CSOCKET_H_

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/socket.h"

#include "netdb.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "log.h"
#include "sfline.h"

#define MAXDATASIZE 2048 /*每次最大数据传输量 */

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
	
	char buf[MAXDATASIZE];
	int buf_size;
	char flags;
}client_t;

int client_connect(client_t* client);

int client_send(client_t* client);

int client_recv(client_t* client);

void client_close(client_t* client);

//
int short_send(client_t* client);
#endif
