
/* tell destination host with ip 'dstip' that the host with request ip 'srcip' is with mac address srcmac
 * author: white cpf  2003.5.15.
 * compile: gcc arp.c -lnet -lpcap -o arp
 */
#include "stdio.h"
#include "pthread.h"
#include "libnet.h"

#include <pcap.h>
#include "thpool.h"

#include "netinet/if_ether.h"
#include "netinet/ip.h"
#include "netinet/udp.h"
#include "netinet/tcp.h"
#include "netinet/ip_icmp.h"

#include "sfline.h"

char *device = "eth0";

u_long src_ip, dst_ip;
u_short src_prt, dst_prt;
char *payload;
u_short payload_s;

libnet_t *l;

u_char enet_src[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
u_char enet_dst[6] = {0x21, 0x22, 0x23, 0x24, 0x25, 0x26};

char mac_s[18]="40:61:86:72:b8:1f\0";
char mac_d[18]="08:00:27:67:de:76\0";

u_long seq=0x01010101;
u_long ack=0x02020202;

static	thpool_t thpool;

void usage(char *name)
{
	fprintf(stderr,
		"usage: %s -s source_ip.source_port -d destination_ip.destination_port"
		" [-p payload]\n",
	name);
	
	fprintf(stderr,
		"usage: %s -s 192.168.1.83.1111 -d 192.168.1.96.22\n",
	name);
	
	fprintf(stderr,
		"usage: %s -s 192.168.1.83.1111 -d 192.168.1.96.22 -p testcase\n",
	name);
}

//int send_tcp_syn(libnet_t *l,u_long src_ip,u_long dst_ip,u_short src_prt,u_long dst_prt,
//					char *payload,u_short payload_s);

int send_tcp_syn();

void mac(char *buf,u_char *uchar,int s)
{
	int k=0;
	u_char ubuf[strlen(buf)];
	strcpy(ubuf,buf);
	
	u_char uc=0;

	char *token=strtok(ubuf,":");
	while(token!=NULL){

		sscanf(token, "%2x", &uc);
		*(uchar+k) = uc;

		//printf("token:%s %d %2x %2x\n",token,k,*(uchar+k),uc);
		
		token=strtok(NULL,":");
		if(k++==s) break;
	}
	//printf("uchar\t%x:%x:%x:%x:%x:%x p:%p p:%p\n", *(uchar+0), *(uchar+1), *(uchar+2), *(uchar+3), *(uchar+4), *(uchar+5),uchar,enet_src);
}

//命令行参数处理函数
int ip_par(int argc,char *argv[]){
	int c;
	char *cp;

	src_ip  = 0;
	dst_ip  = 0;
	src_prt = 0;
	dst_prt = 0;
	payload = NULL;
	payload_s = 0;

    while ((c = getopt(argc, argv, "d:s:p:")) != EOF)
    {
        switch (c)
        {
            /*
             *  We expect the input to be of the form `ip.ip.ip.ip.port`.  We
             *  point cp to the last dot of the IP address/port string and
             *  then seperate them with a NULL byte.  The optarg now points to
             *  just the IP address, and cp points to the port.
             */
            case 'd':
                if (!(cp = strrchr(optarg, '.')))
                {
                    usage(argv[0]);
                }
                *cp++ = 0;
                dst_prt = (u_short)atoi(cp);
                if ((dst_ip = libnet_name2addr4(l, optarg, LIBNET_RESOLVE)) == -1)
                {
                    fprintf(stderr, "Bad destination IP address: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                printf("dst_ip:%s\n", libnet_addr2name4(dst_ip, LIBNET_DONT_RESOLVE));
                break;
            case 's':
                if (!(cp = strrchr(optarg, '.')))
                {
                    usage(argv[0]);
                }
                *cp++ = 0;
                src_prt = (u_short)atoi(cp);
                if ((src_ip = libnet_name2addr4(l, optarg, LIBNET_RESOLVE)) == -1)
                {
                    fprintf(stderr, "Bad source IP address: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }

                printf("src_ip:%s\n", libnet_addr2name4(src_ip, LIBNET_DONT_RESOLVE));
                break;
            case 'p':
                payload = optarg;
                payload_s = strlen(payload);
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (!src_ip || !src_prt || !dst_ip || !dst_prt)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

	mac(mac_s,enet_src,6);
	mac(mac_d,enet_dst,6);

	printf("enet_src\t%02x:%02x:%02x:%02x:%02x:%02x\n", enet_src[0], enet_src[1], enet_src[2], enet_src[3], enet_src[4], enet_src[5]);
	printf("enet_dst\t%02x:%02x:%02x:%02x:%02x:%02x\n", enet_dst[0], enet_dst[1], enet_dst[2], enet_dst[3], enet_dst[4], enet_dst[5]);

}

void call(u_char *argument,const struct pcap_pkthdr* pack,const u_char *content)
{
	int m=0,n;
	const u_char *buf,*iphead;
	u_char *p;
	struct ether_header *ethernet;
	struct iphdr *ip;
	struct tcphdr *tcp;
	struct udphdr *udp;
	struct icmphdr *icmp;
	buf=content;
	printf("==================================================\n");
	printf("The Frame is \n");
	while(m< (pack->len))
	{
		printf("%02x",buf[m]);
		m=m+1;
		if(m%16==0)
			printf("\n");
		else
			printf(":");
	}
	printf("\n");
	printf("Grabbed packet of length %d\n",pack->len);
	printf("Recieved at ..... %s",ctime((const time_t*)&(pack->ts.tv_sec))); 
//	printf("Ethernet address length is %d\n",ETHER_HDR_LEN);

	ethernet=(struct ether_header *)content;
	p=ethernet->ether_dhost;
	n=ETHER_ADDR_LEN;
	printf("Dest MAC is:");
	do{
		printf("%02x:",*p++);
	}while(--n>0);
	printf("\n");
	p=ethernet->ether_shost;
	n=ETHER_ADDR_LEN;
	printf("Source MAC is:");
	do{
		printf("%02x:",*p++);
	}while(--n>0);
	printf("\n");
	
	printf("ether_type %x %x\n",ntohs(ethernet->ether_type),ETHERTYPE_IP);
	
	if(ntohs(ethernet->ether_type)==ETHERTYPE_IP)
	{
		printf("It's a IP packet\n");
		ip=(struct iphdr*)(content+14);
		printf("IP Version:%d\n",ip->version);
		printf("TTL:%d\n",ip->ttl);
		printf("Source address:%s\n",inet_ntoa(*((struct in_addr *)&(ip->saddr))));
		printf("Destination address:%s\n",inet_ntoa(*((struct in_addr *)&(ip->daddr))));
		printf("Protocol:%d\n",ip->protocol);
		switch(ip->protocol)
		{
			case 6:
				printf("The Transport Layer Protocol is TCP\n");
				tcp=(struct tcphdr*)(content+14+20);
				printf("Source Port:%d\n",ntohs(tcp->source));
				printf("Destination Port:%d\n",ntohs(tcp->dest));
				printf("Sequence Number:%u\n",ntohl(tcp->seq));
				printf("ack:%u\n",ntohl(tcp->ack_seq));

				//ack = *((u_long *)&(tcp->ack_seq));
				//ack = 0x03030303;
				//send_tcp_syn();

				//seq++;
				//send_tcp_syn();
				break;
			case 17:
				printf("The Transport Layer Protocol is UDP\n");
				udp=(struct udphdr*)(content+14+20);
				printf("Source port:%d\n",ntohs(udp->source));
				printf("Destination port:%d\n",ntohs(udp->dest));
				break;
			case 1:
				printf("The Transport Layer Protocol is ICMP\n");
				icmp=(struct icmphdr*)(content+14+20);
				printf("ICMP Type:%d\n", icmp->type);
				switch(icmp->type)
				{
					case 8:
						printf("ICMP Echo Request Protocol\n");
						break;
					case 0:
						printf("ICMP Echo Reply Protocol\n");
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
		
/*		if(*iphead==0x45)
		{
			printf("Source ip :%d.%d.%d.%d\n",iphead[12],iphead[13],iphead[14],iphead[15]);
			printf("Dest ip :%d.%d.%d.%d\n",iphead[16],iphead[17],iphead[18],iphead[19]);
			
		}*/
//		tcp= (struct tcp_header*)(iphead);
//		source_port = ntohs(tcp->tcp_source_port);
//		dest_port = ntohs(tcp->tcp_destination_port);

	}
	else if(ntohs (ethernet->ether_type) == ETHERTYPE_ARP)
	{
		printf("This is ARP packet.\n");
		iphead=buf+14;
		if (*(iphead+2)==0x08)
		{
			printf("Source ip:\t %d.%d.%d.%d\n",iphead[14],iphead[15],iphead[16],iphead[17]);
			printf("Dest ip:\t %d.%d.%d.%d\n",iphead[24],iphead[25],iphead[26],iphead[27]);
			printf("ARP TYPE: %d (0:request;1:respond)\n",iphead[6]);

		}
	}
}

int send_tcp_syn(){

	int c;
	libnet_ptag_t t;
	char errbuf[LIBNET_ERRBUF_SIZE];

    printf("libnet 1.1 packet shaping: TCP + options[link]\n");

    /*
     *  Initialize the library.  Root priviledges are required.
     */
    l = libnet_init(
            LIBNET_LINK,                            /* injection type */
            device,                                   /* network interface */
            errbuf);                                /* error buffer */

    if (l == NULL)
    {
        fprintf(stderr, "libnet_init() failed: %s", errbuf);
        exit(EXIT_FAILURE); 
    }
	printf("libnet 1.1 init: [Success]%d\n");

    t = libnet_build_tcp_options(
        (uint8_t*)"\003\003\012\001\002\004\001\011\010\012\077\077\077\077\000\000\000\000\000\000",
        20,
        l,
        0);
    if (t == -1)
    {
        fprintf(stderr, "Can't build TCP options: %s\n", libnet_geterror(l));
        goto bad;
    }
	printf("libnet 1.1 libnet_build_tcp_options: [Success]\n");
    t = libnet_build_tcp(
        src_prt,                                    /* source port */
        dst_prt,                                    /* destination port */
        seq,                                 /* sequence number */
        ack,                                 /* acknowledgement num */
        TH_SYN,                                     /* control flags */
        32767,                                      /* window size */
        0,                                          /* checksum */
        10,                                          /* urgent pointer */
        LIBNET_TCP_H + 20 + payload_s,              /* TCP packet size */
	(uint8_t*)payload,                         /* payload */
        payload_s,                                  /* payload size */
        l,                                          /* libnet handle */
        0);                                         /* libnet id */
    if (t == -1)
    {
        fprintf(stderr, "Can't build TCP header: %s\n", libnet_geterror(l));
        goto bad;
    }
	printf("libnet 1.1 libnet_build_tcp: [Success]\n");
    t = libnet_build_ipv4(
        LIBNET_IPV4_H + LIBNET_TCP_H + 20 + payload_s,/* length */
      	0,                                          /* TOS */
        242,                                        /* IP ID */
        0,                                          /* IP Frag */
        64,                                         /* TTL */
        IPPROTO_TCP,                                /* protocol */
        0,                                          /* checksum */
        src_ip,                                     /* source IP */
        dst_ip,                                     /* destination IP */
        NULL,                                       /* payload */
        0,                                          /* payload size */
        l,                                          /* libnet handle */
        0);                                         /* libnet id */
    if (t == -1)
    {
        fprintf(stderr, "Can't build IP header: %s\n", libnet_geterror(l));
        goto bad;
    }
  printf("libnet 1.1 libnet_build_ethernet: [Success]\n");
    t = libnet_build_ethernet(
        enet_dst,                                   /* ethernet destination */
        enet_src,                                   /* ethernet source */
        ETHERTYPE_IP,                               /* protocol type */
        NULL,                                       /* payload */
        0,                                          /* payload size */
        l,                                          /* libnet handle */
        0);                                         /* libnet id */
    if (t == -1)
    {
        fprintf(stderr, "Can't build ethernet header: %s\n", libnet_geterror(l));
        goto bad;
    }

    /*
     *  Write it to the wire.
     */
	c = libnet_write(l);
	printf("libnet 1.1 libnet_write:%d\n",c);

	if (c == -1)
	{
	    fprintf(stderr, "Write error: %s\n", libnet_geterror(l));
	    goto bad;
   }
   else
   {
	   fprintf(stderr, "Wrote %d byte TCP packet; check the wire.\n", c);
   }
	printf("libnet 1.1 libnet_write: [Success]\n");

	bad:
	libnet_destroy(l);
	return (EXIT_FAILURE);
}

int pcap_tcp_syn()
{
	int ret;

	char *dev;
	char errbuf[LIBNET_ERRBUF_SIZE];
	bpf_u_int32 netp;           /* ip                        */
	bpf_u_int32 maskp;          /* subnet mask               */
	pcap_t* handle;
	int promisc=1;	             /* set to promisc mode?		*/
	int pcap_time_out=1000*3;
	char filter_str[100]="";
	struct bpf_program fp;      /* hold compiled program     */
	const u_char *packet;
	struct pcap_pkthdr hdr;     /* pcap.h    */
	u_char pkt_data[1024];

	dev = pcap_lookupdev(errbuf);
	if(dev == NULL){ 
		fprintf(stderr,"%s\n",errbuf);
		return -1;
	}

	ret=pcap_lookupnet(dev,&netp,&maskp,errbuf);
	if(ret==-1){
		fprintf(stderr,"%s\n",errbuf);
		return -1;
	}

	handle = pcap_open_live(dev,BUFSIZ,promisc,pcap_time_out,errbuf);
	if(handle == NULL){
		printf("pcap_open_live(): %s\n",errbuf);
		return -1; 
	}

	//pcap_setnonblock(handle,0,errbuf);

	//sprintf(filter_str,"(dst %s)",libnet_addr2name4(dst_ip, LIBNET_DONT_RESOLVE));
	char s1[128];
	char s2[128];
	snprintf(s1,sizeof(s1),"%02x:%02x:%02x:%02x:%02x:%02x",enet_src[0], enet_src[1], enet_src[2], enet_src[3], enet_src[4], enet_src[5]);
	snprintf(s2,sizeof(s2),"%02x:%02x:%02x:%02x:%02x:%02x",enet_dst[0], enet_dst[1], enet_dst[2], enet_dst[3], enet_dst[4], enet_dst[5]);
	
	printf("enet_src\t%s\n",s1);
	printf("enet_dst\t%s\n",s2);
	
	sprintf(filter_str,"ether src %s and ether dst %s",s2,s1);
	//sprintf(filter_str,"ip dst 192.168.1.83");
	if(pcap_compile(handle,&fp,filter_str,0,netp) == -1){
		printf("Error calling pcap_compile\n"); 
		return -1;
	}

	if(pcap_setfilter(handle,&fp) == -1){ 
		printf("Error setting filter\n"); 
		return -1;
	}

	printf("wait packet:filter:%s\n",filter_str);
	//pcap_loop(handle,1,call,NULL);
	
	pcap_dispatch(handle,-1,call,NULL);
/*
	struct pcap_pkthdr *pkt_header;  
    const u_char *p_data;
	int r = 0;
	while((r=pcap_next_ex(handle, &pkt_header,&p_data))>=0)
	{
		printf("pcap packs:%d\n",r);
		if(r==0) continue; 
		
		int k=pkt_header->len;
		
		printf("pcap packs:%d\n",k);
		
		char buf[2*k+1];
		memset(buf,'\0',sizeof(buf));

		str_tohex(buf,sizeof(buf),(char *)p_data,k);
	
		printf("pack[%s]",buf);
		break;
	}
*/
}

int main(int argc, char *argv[]){

	//初始化
	ip_par(argc,argv);

	//发送syn
	//send_tcp_syn();
	//抓包
	//pcap_tcp_syn();

	//线程池
	thpool_init(&thpool,25,5);
	usleep(50);


	//pcap_tcp_syn 加入任务队列
	thpool_add_work(&thpool,(void*)pcap_tcp_syn, NULL);

	sleep(3);
	//send_tcp_syn 加入任务队列
	thpool_add_work(&thpool,(void*)send_tcp_syn, NULL);

	//等待其他线程执行完
	sleep(1);

	printf("Wait thpool_destroy\n");
	thpool_destroy(&thpool);

	//等待其他线程执行完
	sleep(1);
	
	printf("Main Exit\n");
	return 0;
}


