//
//  pcap.c
//  pcaptest
//
//  Created by zc on 12-1-24.
//  Copyright 2012年 __MyCompanyName__. All rights reserved.
//
 
#include "stdio.h"
#include "stdlib.h"
#include "sys/socket.h"  
#include "netinet/in.h"  
#include "arpa/inet.h"  
#include "string.h"
#include "pcap.h"

#include "fpcap.h"
#include "log.h"
#include "sfline.h"

void prinfPcapFileHeader(fpcap_file_header *pfh){
	if (pfh==NULL) {
		return;
	}
	printf("=====================\n"
			"magic:0x%0x\n"
			"version_major:%u\n"
			"version_minor:%u\n"
			"thiszone:%d\n"
			"sigfigs:%u\n"
			"snaplen:%u\n"
			"linktype:%u\n"
			"=====================\n",
			pfh->magic,
			pfh->version_major,
			pfh->version_minor,
			pfh->thiszone,
			pfh->sigfigs,
			pfh->snaplen,
			pfh->linktype);
}

void printfPcapHeader(pcap_header *ph){
	if (ph==NULL) {
		return;
	}
	printfs("%s\t%d\t[DEBUG]:HEAD\tts.timestamp_s:%u\tts.timestamp_ms:%u\tcapture_len:%u\tlen:%d",__FILE__,__LINE__,
			ph->ts.timestamp_s,
			ph->ts.timestamp_ms,
			ph->capture_len,
			ph->len);
}

void printPcap(void * data,size_t size){

	if (data==NULL) {
		return;
	}

	int i=0;
	u_char buf[1024];

	//long l1 = 0;
	//long l2 = 0;
	
	u_char mac1[64];
	u_char mac2[64];
	snprintf(mac1,64,"%02X:%2X:%02X:%02X:%02X:%02X",*((u_char *)data),*((u_char *)data+1),*((u_char *)data+2),*((u_char *)data+3),*((u_char *)data+4),*((u_char *)data+5));
	snprintf(mac2,64,"%02X:%2X:%02X:%02X:%02X:%02X",*((u_char *)data+6),*((u_char *)data+7),*((u_char *)data+8),*((u_char *)data+9),*((u_char *)data+10),*((u_char *)data+11));

	printfs("%s\t%d\t[DEBUG]:MAC:%s\t-->%s",__FILE__,__LINE__, mac1, mac2);//注意：printf函数自右向左求值、覆盖 

	struct in_addr addr1, addr2;
	char bf1[64];
	char bf2[64];

	//RFC 894
	ushort * tp = (ushort *)((u_char *)data+12);

	printfs("%s\t%d\t[DEBUG]:tp->%04X",__FILE__,__LINE__,ntohs(*tp));

	ushort us;

	if(ntohs(*tp)==0x0800 || ntohs(*tp)==0x8006 || ntohs(*tp)==0x8035)
	{
		printfs("%s\t%d\t[DEBUG]:RFC 894>>",__FILE__,__LINE__);
		//save_append("fpcap.txt","RFC 894\n");
		tp++;
		u_char *ucp = (u_char *)tp;
		u_char uc = *ucp;
		uc = uc >> 4;
		printfs("%s\t%d\t[DEBUG]:version:\tipv%d",__FILE__,__LINE__,uc);

		snprintf(buf,sizeof(buf),"ipv%d\n",uc);
		save_txt("ipv.txt",buf);

		uc = *ucp;
		uc = uc & 0x0f;
		printfs("%s\t%d\t[DEBUG]:head size:\t%d (bytes)",__FILE__,__LINE__,uc*4);

		snprintf(buf,sizeof(buf),"%d (bytes)\n",uc*4);
		save_txt("heads.txt",buf);

		ucp++;
		uc = *ucp;
		printfs("%s\t%d\t[DEBUG]:OTS:\t%02X",__FILE__,__LINE__,uc);

		snprintf(buf,sizeof(buf),"%02X\n",uc);
		save_txt("ots.txt",buf);

		ucp++;
		tp = (ushort *)ucp;
		us = *tp;
		printfs("%s\t%d\t[DEBUG]:Packed size:\t%04X %d(bytes)",__FILE__,__LINE__,ntohs(us),ntohs(us));

		snprintf(buf,sizeof(buf),"%04X %d(bytes)\n",ntohs(us),ntohs(us));
		save_txt("pl.txt",buf);

		tp++;
		us = *tp;
		printfs("%s\t%d\t[DEBUG]:Packed identifier:\t%04X",__FILE__,__LINE__,ntohs(us));

		snprintf(buf,sizeof(buf),"%04X\n",ntohs(us));
		save_txt("pi.txt",buf);

		tp++;
		us = *tp;
		ntohs(us);
		us = us >> 13;
		printfs("%s\t%d\t[DEBUG]:Packed 3 bit:\t%02X",__FILE__,__LINE__,us);

		snprintf(buf,sizeof(buf),"%02X\n",us);
		save_txt("3bit.txt",buf);

		us = *tp;
		ntohs(us);
		us = us & 0x1fff;
		printfs("%s\t%d\t[DEBUG]:Packed 13 bit offset:\t%04X",__FILE__,__LINE__,us);
		snprintf(buf,sizeof(buf),"%04X\n",us);
		save_txt("13bit.txt",buf);

		tp++;
		ucp = (u_char *)tp;
		uc = *ucp;
		printfs("%s\t%d\t[DEBUG]:TTL:%d",__FILE__,__LINE__,uc);
		snprintf(buf,sizeof(buf),"%d\n",uc);
		save_txt("ttl.txt",buf);

		ucp++;
		uc = *ucp;
		printfs("%s\t%d\t[DEBUG]:network protocol type:\t%2X",__FILE__,__LINE__,uc);
		snprintf(buf,sizeof(buf),"%2X\n",uc);
		save_txt("npt.txt",buf);

		ucp++;
		tp = (ushort *)ucp;
		us = *tp;
		printfs("%s\t%d\t[DEBUG]:16bit CRC:%04X",__FILE__,__LINE__,ntohs(us));

		snprintf(buf,sizeof(buf),"%04X\n",ntohs(us));
		save_txt("16crc.txt",buf);

		//SIP->DIP
		tp++;
		ucp = (u_char *)tp;
		memcpy(&addr1, (char *)ucp, 4);//复制4个字节大小  
		memcpy(&addr2, (char *)ucp+4, 4);

		snprintf(bf1,64,"%s",inet_ntoa(addr1));
		snprintf(bf2,64,"%s",inet_ntoa(addr2));

		printfs("%s\t%d\t[DEBUG]:IP:\t%s-->%s",__FILE__,__LINE__,bf1, bf2);
		snprintf(buf,sizeof(buf),"%s\t%s\n",bf1, bf2);
		save_txt("sdip.txt",buf);


		printfs("%s\t%d\t[DEBUG]:<<RFC 894",__FILE__,__LINE__);
		//
	}
	else//802.3
	{
		printfs("%s\t%d\t[DEBUG]:802.3",__FILE__,__LINE__);
		//save_append("fpcap.txt","802.3\n");
	}
	
	

	for (i=0; i<size;i++) {
		unsigned char ch = *((unsigned char *)data+i);
		if(i%6==0) printf("\n");
		printf("%02x",ch);
	}
	printf("\n");
}
