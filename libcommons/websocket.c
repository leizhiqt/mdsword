#include "stdio.h"
#include "fcntl.h"
#include "sys/socket.h"
#include "unistd.h"
#include "errno.h"

#include "base64.h"
#include "sha1.h"
#include "intLib.h"
#include "websocket.h"

#define WEB_SOCKET_KEY_LEN_MAX 256
#define RESPONSE_HEADER_LEN_MAX 1024
#define R_MAX 256

char * fetchSecKey(const char * buf)
{
  char *key;
  char *keyBegin;
  char *flag="Sec-WebSocket-Key: ";
  int i=0, bufLen=0;

  key=(char *)malloc(WEB_SOCKET_KEY_LEN_MAX);
  memset(key,0, WEB_SOCKET_KEY_LEN_MAX);
  if(!buf)
    {
      return NULL;
    }
 
  keyBegin=strstr(buf,flag);
  if(!keyBegin)
    {
      return NULL;
    }
  keyBegin+=strlen(flag);

  bufLen=strlen(buf);
  for(i=0;i<bufLen;i++)
    {
      if(keyBegin[i]==0x0A||keyBegin[i]==0x0D)
	{
	  break;
	}
      key[i]=keyBegin[i];
    }
  
  return key;
}

char * computeAcceptKey(const char * buf)
{
	char * clientKey;
	char * serverKey; 
	char * sha1DataTemp;
	char * sha1Data;
	short temp;
	int i,n;
	const char * GUID="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	if(!buf)
	{
		return NULL;
	}
	clientKey=(char *)malloc(R_MAX);
	memset(clientKey,0,R_MAX);
	clientKey=fetchSecKey(buf);

	if(!clientKey)
	{
		return NULL;
	}

	strcat(clientKey,GUID);

	sha1DataTemp=sha1_hash(clientKey);
	free(clientKey);

	n=strlen(sha1DataTemp);

	sha1Data=(char *)malloc(n/2+1);
	memset(sha1Data,0,n/2+1);

	for(i=0;i<n;i+=2)
	{      
		sha1Data[i/2]=htoi(sha1DataTemp,i,2);    
	} 

	serverKey = base64_encode(sha1Data, strlen(sha1Data)); 
	free(sha1Data);
	free(sha1DataTemp);
	return serverKey;
}

void shakeHand(const char *serverKey,char *responseHeader)
{
	if(!serverKey)
	{
		return;
	}

	memset(responseHeader,'\0',RESPONSE_HEADER_LEN_MAX);

	strcat(responseHeader, "HTTP/1.1 101 Switching Protocols\r\n");
	strcat(responseHeader, "Upgrade: websocket\r\n");
	strcat(responseHeader, "Connection: Upgrade\r\n");
	strcat(responseHeader, "Sec-WebSocket-Accept: ");
	strcat(responseHeader, serverKey);
	strcat(responseHeader, "\r\n\r\n");
	//printfs("%s\t%d\t[DEBUG]:Response Header:%s",__FILE__,__LINE__,responseHeader);
}

//协议解析
char * analyData(const char * buf,const int bufLen)
{
	char * data;
	char fin, maskFlag,masks[4];
	char * payloadData;
	char temp[8];
	unsigned long n, payloadLen=0;
	unsigned short usLen=0;
	int i=0; 

	if (bufLen < 2) 
	{
		return NULL;
	}

	fin = (buf[0] & 0x80) == 0x80; // 1bit，1表示最后一帧  
	if (!fin)
	{
		return NULL;// 超过一帧暂不处理 
	}

	maskFlag = (buf[1] & 0x80) == 0x80; // 是否包含掩码  
	if (!maskFlag)
	{
		return NULL;// 不包含掩码的暂不处理
	}

	payloadLen = buf[1] & 0x7F; // 数据长度
	//printfs("%s\t%d\t[DEBUG]: payloadLen:%d",__FILE__,__LINE__,payloadLen);
	if (payloadLen == 126)
	{      
		memcpy(masks,buf+4, 4);      
		payloadLen =(buf[2]&0xFF) << 8 | (buf[3]&0xFF);
		
		//printfs("%s\t%d\t[DEBUG]: payloadLen:%d",__FILE__,__LINE__,payloadLen);

		payloadData=(char *)malloc(payloadLen);
		
		memset(payloadData,0,payloadLen);
		memcpy(payloadData,buf+8,payloadLen);
	}
	else if (payloadLen == 127)
	{
		//printfs("%s\t%d\t[DEBUG]: payloadLen:%d",__FILE__,__LINE__,payloadLen);
	
		memcpy(masks,buf+10,4);
		for ( i = 0; i < 8; i++)
		{
			temp[i] = buf[8 - i];
		} 

		memcpy(&n,temp,8);
		payloadData=(char *)malloc(n); 
		memset(payloadData,0,n); 
		memcpy(payloadData,buf+14,n);//toggle error(core dumped) if data is too long.
		payloadLen=n;
	}
	else
	{   
		//printfs("%s\t%d\t[DEBUG]: payloadLen:%d",__FILE__,__LINE__,payloadLen);
		memcpy(masks,buf+2,4);    
		payloadData=(char *)malloc(payloadLen+1);
		memset(payloadData,0,payloadLen+1);
		memcpy(payloadData,buf+6,payloadLen); 
	}

	//printfs("%s\t%d\t[DEBUG]: payloadLen:%d",__FILE__,__LINE__,payloadLen);

	for (i = 0; i < payloadLen; i++)
	{
		payloadData[i] = (char)(payloadData[i] ^ masks[i % 4]);
	}

	printf("data(%d):%s \n",payloadLen,payloadData);
	return payloadData;
}
//pack text
//ascall
char * packData(const char * message,unsigned int *nsize,int frame)
 {
	char * data=NULL;
	unsigned int n = *nsize;
	
	//one 0x81
	//end:10000010
	//not end:00000010
	unsigned char headByte = 0x00;//发送帧
	if(frame==1)
	{
		headByte = 0x01;//起始帧
	}
	else if(frame==-1)
	{
		headByte = 0x80;//结束帧
	}
	else if(frame==0)
	{
		headByte = 0x81;//单帧
	}else if(frame==-2)
	{
		headByte = 0x88;//关闭帧
	}

	if (n < 126)
	{
		data=(char *)malloc(n+2);
		memset(data,0,n+2);	 
		data[0] = headByte;
		data[1] = n;
		memcpy(data+2,message,n);
		*nsize = n+2;
	}
	else if (n < 0xFFFF)
	{
		data=(char *)malloc(n+4);
		memset(data,0,n+4);
		data[0] = headByte;
		data[1] = 126;
		data[2] = (n>>8 & 0xFF);
		data[3] = (n & 0xFF);
		memcpy(data+4,message,n);
		*nsize = n+4;
	}
	//printf("packData(%d)\n",malloc_usable_size(data));
	return data;
}

//pack Binry
char * packBinry(const char * message,unsigned int *nsize,int frame)
 {
	char * data=NULL;
	unsigned int n = *nsize;

	//end:10000010
	//not end:00000010
	//one 0x82
	unsigned char headByte = 0x00;//发送帧
	if(frame==1)
	{
		headByte = 0x02;//起始帧
	}
	else if(frame==-1)
	{
		headByte = 0x80;//结束帧
	}
	else if(frame==0)
	{
		headByte = 0x82;//单帧
	}

	//
	if (n < 126)
	{
		data=(char *)malloc(n+2);
		memset(data,0,n+2);	 
		data[0] = headByte;
		data[1] = n;
		memcpy(data+2,message,n);
		*nsize = n+2;
	}
	else if (n < 0xFFFF)
	{
		data=(char *)malloc(n+4);
		memset(data,0,n+4);
		data[0] = headByte;
		data[1] = 126;
		data[2] = (n>>8 & 0xFF);
		data[3] = (n & 0xFF);
		memcpy(data+4,message,n);
		*nsize = n+4;
	}
	//printf("packData(%d)\n",malloc_usable_size(data));
	return data;
 }

//websocket协议关闭帧命令
int send_ws_close(int csockfd)
{
	int RET = -1;

	char cbuf[4]={0x08,0x08,0x00,0x00};

	if((RET=send(csockfd,cbuf,4,0))== -1)
	{
		perror("send error");
	}
	return RET;
}

//协议发送
int send_ws_data(int csockfd,const char * message)
{
	int RET = -1;

	char *data = NULL;
	int n =0;

	n = strlen(message);
	data = packData(message,&n,0);

	if((RET=send(csockfd,data,n,0))== -1)
	{
		perror("send error");
	}
	free(data);

	return RET;
}
//发送文件
int send_file(const int sock_fd,const char *fname)
{
	int fd = open(fname,O_RDONLY,0644);
	if(fd ==-1)
	{
		printf("open error\n");
		return -1;
	}
	char rbuf[4];

	int BUFFER_SIZE = 1024;

	char buf[BUFFER_SIZE+1];
	bzero(buf, BUFFER_SIZE+1);

	int n = 0;
	char *data = NULL;
	int RET = -1;

	// 每读取一段数据，便将其发送给客户端，循环直到文件读完为止
	//int wfd = open("bbbb.jpg",O_WRONLY|O_CREAT,0644);
	long fsize = lseek(fd,0L,SEEK_END);
	int count = 0;

	lseek(fd,0L,SEEK_SET);
	int f=0;
	while((n = read(fd,buf,BUFFER_SIZE)) > 0) 
	{
		//write(wfd, buf, n);
		//data = packBinry(buf,&n,1);
		//printf("Send count:%d fsize:%d\n",count,fsize);
		count +=n;

		if(f==0)
		{
			data = packBinry(buf,&n,1);
			//printf("Send frame 1:%d\n",f);
		}

		//printf("Send count:%d fsize:%d\n",count,fsize);
		if(count==fsize)
		{
			data = packBinry(buf,&n,-1);
			//printf("Send frame -1:%d\n",f);
		}
		else
		{
			data = packBinry(buf,&n,f+1);
			//printf("Send frame 2...:%d\n",f);
		}

		RET= send(sock_fd,data,n,0);
		//清理堆数据
		free(data);
		bzero(buf,BUFFER_SIZE);
		//发送
		if(RET<0)
		{
			//printfs("%s\t%d\t[DEBUG]:sock_fd:%d Send File:%s Failed.",__FILE__,__LINE__,sock_fd,fname);
			break;
		}

		//n=-1;
		//usleep(500);
		f++;
	}
	//printf("Send ======= n:%d RET:%d\n",n,RET);
	//close(wfd);

	close(fd);
	return 0;
}
