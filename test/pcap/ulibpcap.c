//
//  main.c
//  pcaptest
//
//  Created by zc on 12-1-24.
//  Copyright 2012年 __MyCompanyName__. All rights reserved.
//
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/types.h"
#include "sys/socket.h"

#include "string.h"
#include "errno.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "ctype.h"

#include "pcap.h"

#include "log.h"
#include "sfline.h"
#include "fpcap.h"

#define PCAP_FILE "ping.pcap"
#define MAX_ETH_FRAME 1514
#define ERROR_FILE_OPEN_FAILED -1
#define ERROR_MEM_ALLOC_FAILED -2
#define ERROR_PCAP_PARSE_FAILED -3

//==========================

#define SNAP_LEN 1518	   // 以太网帧最大长度
#define SIZE_ETHERNET 14   // 以太网包头长度 mac 6*2, type: 2
#define ETHER_ADDR_LEN  6  // mac地址长度

#define PCAP_FILE "ping.pcap"
#define MAX_ETH_FRAME 1514
#define ERROR_FILE_OPEN_FAILED -1
#define ERROR_MEM_ALLOC_FAILED -2
#define ERROR_PCAP_PARSE_FAILED -3

struct packet_ethernet {
	u_char  ether_dhost[ETHER_ADDR_LEN];	/* destination host address */
	u_char  ether_shost[ETHER_ADDR_LEN];	/* source host address */
	u_short ether_type;					 /* IP? ARP? RARP? etc */
};
 
/* IP header */
struct packet_ip {
	u_char  ip_vhl;				 /* version << 4 | header length >> 2 */
	u_char  ip_tos;				 /* type of service */
	u_short ip_len;				 /* total length */
	u_short ip_id;				  /* identification */
	u_short ip_off;				 /* fragment offset field */
	#define IP_RF 0x8000			/* reserved fragment flag */
	#define IP_DF 0x4000			/* dont fragment flag */
	#define IP_MF 0x2000			/* more fragments flag */
	#define IP_OFFMASK 0x1fff	   /* mask for fragmenting bits */
	u_char  ip_ttl;				 /* time to live */
	u_char  ip_p;				   /* protocol */
	u_short ip_sum;				 /* checksum */
	struct  in_addr ip_src,ip_dst;  /* source and dest address */
	//struct in_addr ip_src;
	//struct in_addr ip_dst;			  /* source and dest address */
};
#define IP_HL(ip)			   (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)				(((ip)->ip_vhl) >> 4)
 
/* TCP header */
typedef u_int tcp_seq;
 
struct packet_tcp {
	u_short th_sport;			   /* source port */
	u_short th_dport;			   /* destination port */
	tcp_seq th_seq;				 /* sequence number */
	tcp_seq th_ack;				 /* acknowledgement number */
	u_char  th_offx2;			   /* data offset, rsvd */
	#define TH_OFF(th)	  (((th)->th_offx2 & 0xf0) >> 4)
	u_char  th_flags;
	#define TH_FIN  0x01
	#define TH_SYN  0x02
	#define TH_RST  0x04
	#define TH_PUSH 0x08
	#define TH_ACK  0x10
	#define TH_URG  0x20
	#define TH_ECE  0x40
	#define TH_CWR  0x80
	#define TH_FLAGS		(TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	u_short th_win;				 /* window */
	u_short th_sum;				 /* checksum */
	u_short th_urp;				 /* urgent pointer */
};
 
int m_write(const u_char *p,int len){
	FILE *fp;
	fp = fopen("pcap.bin","a+");
	fwrite(p,len,1,fp);
	fwrite("\n\n",4,1,fp);
	fclose(fp);
	
	int i=0;
	for(i=0;i<len;i++)
	{
		printf(" %02x",*(p++));
		if((i+1)%16==0){
			printf("\n");
		}
	}
	printf("\n");
}

void loop_callback(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
	static int count = 0;				   // 包计数器
	const struct packet_ethernet *ethernet;  /* The ethernet header [1] */
	const struct packet_ip *ip;			  /* The IP header */
	const struct packet_tcp *tcp;			/* The TCP header */
	const char *payload;					 /* Packet payload */
 
	int size_ip;
	int size_tcp;
	int size_payload;
 
	count++;
 
	/* 以太网头 */
	ethernet = (struct packet_ethernet*)(packet);
 
	/* IP头 */
	ip = (struct packet_ip*)(packet + SIZE_ETHERNET);
	size_ip = IP_HL(ip)*4;
	if (size_ip < 20) {
		printf("无效的IP头长度: %u bytes\n", size_ip);
		return;
	}
 
	if ( ip->ip_p != IPPROTO_TCP ){ // TCP,UDP,ICMP,IP
	return;
	}
 
	/* TCP头 */
	tcp = (struct packet_tcp*)(packet + SIZE_ETHERNET + size_ip);
	size_tcp = TH_OFF(tcp)*4;
	if (size_tcp < 20) {
		printf("无效的TCP头长度: %u bytes\n", size_tcp);
		return;
	}
 
	int sport =  ntohs(tcp->th_sport);
	int dport =  ntohs(tcp->th_dport);
 
	printf("%s:%d -> ", inet_ntoa(ip->ip_src), sport);
	printf("%s:%d ", inet_ntoa(ip->ip_dst), dport);
 
	//内容
	payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);
 
	//内容长度 
	size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);

	if (size_payload > 0) { 
		//printf("%d bytes:\n", size_payload, payload);
		printf("seq:%d ack:%d flags:%d bytes:%d\n",ntohs(tcp->th_seq),ntohs(tcp->th_ack),ntohs(tcp->th_flags),size_payload);
		m_write(payload,size_payload);
	} else {
		printf("seq:%d ack:%d syn:%d payload zero\n", ntohs(tcp->th_seq), ntohs(tcp->th_ack), ntohs(TH_SYN));
	}
}

/*void aloop_callback(u_char* argument, const struct pcap_pkthdr* header, const u_char* content){*/
/*	m_write(content, header->caplen);*/
/*}*/

int promiscuous(char *dev) {
	pcap_t *handle; /* 会话句柄 */
	//char *dev; /* 执行嗅探的设备 */
	char errbuf[PCAP_ERRBUF_SIZE]; /* 存储错误信息的字符串 */
	struct bpf_program filter; /* 已经编译好的过滤器 */
	char filter_app[] = "port 80"; /* 过滤表达式 */
	bpf_u_int32 mask; /* 所在网络的掩码 */
	bpf_u_int32 net; /* 主机的IP地址 */
	struct pcap_pkthdr header; /* 由pcap.h定义 */
	const u_char *packet; /* 实际的包 */
	/* Define the device */
	/* dev = pcap_lookupdev(errbuf); */
	//dev = "eth0";  /* 网卡名称 */
	pcap_lookupnet(dev, &net, &mask, errbuf); /* 探查设备属性 */
	handle = pcap_open_live(dev, 65536, 1, 0, errbuf); /* 以混杂模式打开会话 */
	pcap_compile(handle, &filter, filter_app, 0, net); /* 编译并应用过滤器 */
	pcap_setfilter(handle, &filter);
 
	pcap_loop(handle,-1,loop_callback,NULL);
	pcap_close(handle); /* 关闭会话 */
	return(0);
}

//读取pcap文件
int rpcap(const char *fpcap)
{
 
	printf("sizeof:int %lu,unsigned int %lu,char %lu,unsigned char %lu,short:%lu,unsigned short:%lu\n",
	sizeof(int),sizeof(unsigned int),sizeof(char),sizeof(unsigned char),sizeof(short),sizeof(unsigned short));

	logsk(1024);
	
	fpcap_file_header  pfh;
	pcap_header  ph;
	int count=0;
	void * buff = NULL;
	int readSize=0;
	int ret = 0;

	FILE *fp = fopen(fpcap, "rw");
 
	if (fp==NULL) {
		fprintf(stderr, "Open file %s error.",PCAP_FILE);
		ret = ERROR_FILE_OPEN_FAILED;
		goto ERROR;
	}
 
	fread(&pfh, sizeof(fpcap_file_header), 1, fp);	
	prinfPcapFileHeader(&pfh);
	//fseek(fp, 0, sizeof(pcap_file_header));
 
	buff = (void *)malloc(MAX_ETH_FRAME);
	for (count=1; ; count++) {
		memset(buff,0,MAX_ETH_FRAME);
		//read pcap header to get a packet
		//get only a pcap head count .
		readSize=fread(&ph, sizeof(pcap_header), 1, fp);
		if (readSize<=0) {
			break;
		}
		printfPcapHeader(&ph);
 
 
		if (buff==NULL) {
			fprintf(stderr, "malloc memory failed.\n");
			ret = ERROR_MEM_ALLOC_FAILED;
			goto ERROR;
		}
 
		//get a packet contents.
		//read ph.capture_len bytes.
		readSize=fread(buff,1,ph.capture_len, fp);
		if (readSize != ph.capture_len) {
			free(buff);
			fprintf(stderr, "pcap file parse error.\n");
			ret = ERROR_PCAP_PARSE_FAILED;
			goto ERROR;
		}

		char packets[ph.capture_len+2];
		memset(packets,'\0',ph.capture_len+2);
		int k=0;
		for(int i=0;i<ph.capture_len;i++){
			u_char uc = *((u_char *)buff+i);

			if(isascii(uc) && !iscntrl(uc)){
				packets[k] = uc;
				k++;
				//printf("%c",uc);
				//snprintf(packets,sizeof(packets),"%s%c",packets,uc);
			}
		}
		packets[k] = '\n';

		//snprintf(packets,sizeof(packets),"%s\n",packets);
		char *fstr = NULL;
		fstr = strstr(packets,"Test-group");
		int tk = -1;
		if(fstr != NULL){
			tk = fstr-packets;
		}

		printfs("%s\t%d\t[DEBUG]:Test-group tk:%d",__FILE__,__LINE__,tk);
		if(tk>0){
			char bufpacks[strlen(packets)+1];
			memset(bufpacks,'\0',sizeof(bufpacks));
			memcpy(bufpacks,packets+tk,strlen(packets)-tk+1);

			//save_txt("ippacket.txt",bufpacks);

			fstr= strstr(bufpacks,";");
			tk = -1;
			if(fstr != NULL){
				tk = fstr-bufpacks;
			}

			if(tk>0){
				memset(packets,'\0',sizeof(packets));
				memcpy(packets,bufpacks,tk+1);
				packets[tk+1]='\n';

				save_txt("ippacket.txt",packets);
			}
		}


		printfs("%s\t%d\t[DEBUG]:frame[%d],readSize:%d packload:%s",__FILE__,__LINE__,count,readSize,packets);

		//printPcap(buff, ph.capture_len);
		//printfs("%s\t%d\t[DEBUG]:frame[%d],readSize:%d buff:%s",__FILE__,__LINE__,count,readSize,buff);

		if (feof(fp) || readSize <=0 ) { 
			break;
		}
	}
 
ERROR:
	//free
	if (buff) {
		free(buff);
		buff=NULL;
	} 
	if (fp) {
		fclose(fp);
		fp=NULL;
	}	
 
	return ret;
}

//读取pcap文件
int wpcap(const char *fpcap)
{
	char *err;
	char *dev;
	struct pcap_pkthdr *header;
	struct bpf_program guize;
	char guize_string[]="";
	u_char *packet;
	bpf_u_int32 netip;
	bpf_u_int32 netmask;
	dev=pcap_lookupdev(err);
	pcap_t *pcap_handle;
	pcap_lookupnet(dev,&netip,&netmask,err);
	pcap_handle=pcap_open_live(dev,BUFSIZ,1,200,err);
	pcap_compile(pcap_handle,&guize,guize_string,0,netip);
	pcap_setfilter(pcap_handle,&guize);

	////////捕获数据包 并写入文件/////////////////////////////////
	pcap_dumper_t *p;
	p=pcap_dump_open(pcap_handle,"/home/yang/桌面/11/112");
	pcap_next_ex(pcap_handle,&header,(const u_char**)&packet);
	for(int k=0;k<header->len;k++)
	printf("%c",*(packet+k));
	printf("\n");
	pcap_dump((unsigned char*)p,header,packet); //写入
	pcap_dump_close(p); //关闭

	/////////读取数据报文件
	pcap_t *ohandle;
	ohandle=pcap_open_offline("/home/yang/桌面/11/112",err);
	//p=pcap_dump_open(ohandle,"/home/yang/桌面/11/112");
	struct pcap_pkthdr * header2;
	const u_char *packet2;
	pcap_next_ex(ohandle,&header2,&packet2);
	for(int k=0;k<header2->len;k++)
	printf("%c",*(packet2+k));
	printf("\n");
}

//一直读取文件更新
//char *path="/home/zhf/zhf/c_prj/c_test.txt";
void read_file(const char *path)
{
	int fd,n;
	int bk=0;
	char buf[1024];
	fd=open(path,O_RDONLY);

	printfs("%s\t%d\t[DEBUG]:fd:%d",__FILE__,__LINE__,fd);

	while(1)
	{
		n=read(fd,buf,sizeof(buf));
		//printfs("%s\t%d\t[DEBUG]:%d",__FILE__,__LINE__,n);
		if(n>0)
		{
			buf[n]='\0';
			printfs("%s\t%d\t[DEBUG]:%s",__FILE__,__LINE__,buf);
		}
		else
		{
			bk++;
		}
		if(bk==100) usleep(10000);
	}
	close(fd);
}

int ipaddr()  
{  
	char ip1[] = "192.0.1.130";  
	char ip2[] = "192.0.1.57";  
	struct in_addr addr1, addr2;  
	long l1, l2;  
	l1 = inet_addr(ip1);   //IP字符串——》网络字节  
	l2 = inet_addr(ip2);  
	printf("IP1: %s \t IP2: %s\n", ip1, ip2);  
	printf("l1: %ld \t l2: %ld\n", l1, l2);  

	printf("Addr1: %08X \t Addr2: %08X\n", l1, l2);

	memcpy(&addr1, &l1, 4); //复制4个字节大小  
	memcpy(&addr2, &l2, 4);  
	//printf("%s <--> %s\n", inet_ntoa(addr1), inet_ntoa(addr2)); //注意：printf函数自右向左求值、覆盖
	char bf1[64];
	char bf2[64];
	snprintf(bf1,64,"%s",inet_ntoa(addr1));
	snprintf(bf2,64,"%s",inet_ntoa(addr2));

	printf("%s \t %s\n",bf1,bf2); //网络字节 ——》IP字符串
	//printf("%s\n", inet_ntoa(addr2));  
	return 0;
}

//goose pack
unsigned char *payload="010ccd01000140618672b81f8100800088b803e800c1000000006181b6802973696d706c65494f47656e65726963494f2f4c4c4e3024474f24676362416e616c6f6756616c756573810100822373696d706c65494f47656e65726963494f2f4c4c4e3024416e616c6f6756616c756573832973696d706c65494f47656e65726963494f2f4c4c4e3024474f24676362416e616c6f6756616c756573840859dc880df3f7ce0a8501018601008701008801018901008a0104ab1a850204d28c060000000000008502162e8a086161616161616161";

void sendpack(unsigned char* hexs)
{
	//char *FinalPacket = "\x00\xe0\x4c\x68\x00\x19\x3c\x97\x0e\x3b\xee\x23\x08\x00\x45\x00\x00\x32\x00\x76\x00\x00\x40\x01\xf6\x6a\xc0\xa8\x01\x53\xc0\xa8\x01\x47\x08\x00\xdb\x19\xc1\x49\xc5\x6c\x56\x65\x72\x73\x69\x6f\x6e\x3b\x20\x54\x65\x73\x74\x2d\x63\x61\x73\x65\x3a\x20\x33\x30";
	int k = strlen(hexs)/2;
	
	unsigned char FinalPacket[k+1];
	memset(FinalPacket,'\0',sizeof(FinalPacket));
	
	str_tostr(FinalPacket,hexs);

/*	char *FinalPacket = "\x44\x8a\x5b\xc9\x42\x24\x3c\x97\x0e\x3b\xee\x23\x08\x00\x40\x00\x00\x01\x00\x12\x00\x00\x40\x01\xfb\xe1\xc0\xa8\x01\x53\xc0\xa8\x01\x65\x08\x00\x18\xe1\xc0\xe5\xc5\x08\x54\x65\x73\x74\x2d\x67\x72\x6f\x75\x70\x3a\x20\x49\x50\x76\x34\x2e\x49\x50\x76\x34\x2e\x69\x70\x76\x34\x2d\x6d\x65\x73\x73\x61\x67\x65\x2e\x45\x74\x68\x65\x72\x6e\x65\x74\x2d\x50\x61\x79\x6c\x6f\x61\x64\x2e\x49\x50\x76\x34\x2d\x50\x61\x63\x6b\x65\x74\x2e\x49\x50\x76\x34\x2d\x48\x65\x61\x64\x65\x72\x2e\x54\x6f\x74\x61\x6c\x2d\x4c\x65\x6e\x67\x74\x68\x3b\x20\x54\x65\x73\x74\x2d\x63\x61\x73\x65\x3a\x20\x31";
*/
//这是一个完整的以太网帧。最后四个字节 8b 6b f5 13是其FCS字段，用于与后面生成的CRC32对照  
/*alt_u8  tx_data[] = {  
        0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0x00,   0x1f,   //8  
        0x29,   0x00,   0xb5,   0xfa,   0x08,   0x06,   0x00,   0x01,   //15  
        0x08,   0x00,   0x06,   0x04,   0x00,   0x01,   0x00,   0x1f,   //24  
        0x29,   0x00,   0xb5,   0xfa,   0xac,   0x15,   0x0e,   0xd9,   //32  
        0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0xac,   0x15,   //40  
        0x0e,   0x8e,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   //48  
        0x00,   0x00 ,  0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   //56  
        0x00,   0x00,   0x00,   0x00,   0x8b,   0x6b,   0xf5,   0x13    //64  
};*/
	char *dev; /* Network Device(s) driver */
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle; /* Session(s) description */
	int i = 0;
	 
	if (getuid())
	{
		printf("Error! Must be root ... exiting\n");
		return;
	}
	 
	dev = pcap_lookupdev(errbuf);
	if (dev == NULL)
	{
		fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
		return;
	}
	printf("Device: %s\n", dev);
	 
	/* Set the pcap description */
	handle = pcap_open_live(dev, BUFSIZ, 1, 0, errbuf);
	if (handle == NULL)
	{
		printf("pcap_open_live(): %s\n", errbuf);
		return;
	}
	/* Here we loop the injector */
	int flag_retorno = 1;
	/*if (vflag)*/
	//flag_retorno = pcap_sendpacket(handle,FinalPacket, 55);
	flag_retorno = pcap_sendpacket(handle,FinalPacket, k);
	if(flag_retorno == 0)
	{
		printf("Injecting packet on %s: 1 second delay\n",dev);
	}

	/* 关闭会话 */
	pcap_close(handle);
}

int main(int argc,char *argv[]){
	promiscuous("eth0");
	return(0);
}
