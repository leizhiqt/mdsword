/********************************** 
 * @author	leizhifesker@icloud.com
 * @date	2017/03/07
 * Last update:	2017/03/07
 * License:	LGPL
 * 
 **********************************/
#include "stdio.h"
#include "stdlib.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/stat.h"
#include "string.h"
#include "dirent.h"
#include "regex.h"

#include "sfline.h"
#include "log.h"

int exist_line(const char *file_name,const char *buf)
{
	FILE * fstream;
	char * line = NULL;  
	size_t len = 0;  
	ssize_t read;
	
	if((fstream = fopen(file_name, "r"))==NULL)
		return 0;

	int RET = 0;

	while ((read = getline(&line, &len, fstream)) != -1) {
	
		if(!(read>1)) continue;

		//*(line+read-1) = '\0';
		//printf("Retrieved line of length:%zu length:%d len:%d line:%s buf:%s\n",read,read,len,line,buf);

		if(strcmp(line,buf)==0)
		{
			RET = 1;
			break;
		}
	}

	if(line) free(line);
	fclose(fstream);

	return RET;
}

int save_txt(const char *file_name,const char *buf){

	int RET = -1;

	if(exist_line(file_name,buf))
	{
		printfs("%s\t%d\t[DEBUG]: exist_line:true",__FILE__,__LINE__);
		return -1;
	}

	int fd = open(file_name,O_WRONLY|O_CREAT|O_APPEND,0644);
	if(fd ==-1)
	{
		printfs("%s\t%d\t[DEBUG]: open:%s RET:%d",__FILE__,__LINE__,file_name,RET);
		return -2;
	}

	RET = write(fd, buf, strlen(buf));
	printfs("%s\t%d\t[DEBUG]: write:%s RET:%d",__FILE__,__LINE__,file_name,RET);

	//release:
	RET=close(fd);
	printfs("%s\t%d\t[DEBUG]: close:%s RET:%d",__FILE__,__LINE__,file_name,RET);

	return 0;
}

int save_append(const char *file_name,const char *buf){

	int RET = -1;

	int fd = open(file_name,O_WRONLY|O_CREAT|O_APPEND,0644);
	if(fd ==-1)
	{
		printfs("%s\t%d\t[DEBUG]: open:%s RET:%d",__FILE__,__LINE__,file_name,RET);
		return -2;
	}

	RET = write(fd, buf, strlen(buf));
	printfs("%s\t%d\t[DEBUG]: write:%s RET:%d",__FILE__,__LINE__,file_name,RET);

	//release:
	RET=close(fd);
	printfs("%s\t%d\t[DEBUG]: close:%s RET:%d",__FILE__,__LINE__,file_name,RET);

	return 0;
}

void brline(const char *ch,const int k)
{
	char *p = (char *)ch+k-1;

	if(*p=='\r' || *p=='\n')
		*p='\0';

	p--;

	if(*p=='\r' || *p=='\n')
		*p='\0';
}

void clsline(const char *ch)
{
	char *p = (char *)ch;
	while('\0' != *p)
	{
		//printf("%c %02X\n",*p,*p);
		if(*p=='\r') *p='\0';
		p++;
	}
	//printf("c:%c 0x%02X\n",*p,*p);
	*p = '\0';
	//printf("c:%c 0x%02X\n",*p,*p);
}

//tar cmd
void tarz(const char *infile, const char *outfile)
{
	char ecmd[512];
	memset(ecmd,'\0',sizeof(ecmd));
	
	snprintf(ecmd,sizeof(ecmd),"tar -zcf %s %s",outfile,infile);
	printfs("%s\t%d\t[DEBUG]:ecmd:%s",__FILE__,__LINE__,ecmd);
	system(ecmd);
}

/*str_tohex
 *二进制字节数组 转换为 16进制的字符串
 *char* hexs		16进制的字符串
 *const int hexsmax	16进制的字符串长度
 *const char* s		二进制字节数组
 *const int smax	二进制字节数组长度 	
 *return int 0:转换成功 非0:转换失败
*/
int str_tohex(char* hexs,const int hexsmax, const char* s, const int smax)
{
	const char* chs_hex = "0123456789ABCDEF";
	int k = 0;
	int i=0,j=0;

	for(i=0,j=0;i<smax;i++,j+=2)
	{
		if(j>=hexsmax) return 1;

		k = (s[i] & 0xf);
		hexs[i*2+1] = chs_hex[k];

		k = ((s[i]>>4) & 0xf);
		hexs[i*2] = chs_hex[k];
		
		//printfs("%s\t%d\t[DEBUG]: %c %c",__FILE__,__LINE__,hexs[i*2],hexs[i*2+1]);
	}
	return 0;
}

/*str_tostr
 *16进制的字符串转换为二进制字节数组
 *char* s 			二进制字节数组
 *const char* hexs	16进制的字符串	
 *return int 0:转换成功 非0:转换失败
*/
int str_tostr(char* s, const char* hexs)
{
	int RET = -1;

	int i=0;
	int maxs = strlen(hexs);

	if(maxs%2 != 0 ) return RET;
	//unsigned int ascii = 65535;
	for (i = 0; i < maxs/2 ; i++)
	{
		RET = sscanf(hexs+i*2, "%02X", (unsigned int*)(s+i));
		//if(*(unsigned int*)(s+i) > 255) break;
		//printfs("sscanf %s\t%d\t[DEBUG]:RET=%d %c %c %c",__FILE__,__LINE__,RET,*(hexs+i),*(hexs+i+1),*(s+i));
	}
	//printfs("sscanf %s\t%d\t[DEBUG]:i=%d",__FILE__,__LINE__,i);
	*(s+i)= '\0';
	return 0;
}

/*a2x
 *16进制的字符转换为二进制字符
 *char ch			16进制的字符
 *return char 二进制字符
*/
char a2x(char ch)
{
	switch(ch)
	{
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'A':
		case 'a':
			return 10;
		case 'B':
		case 'b':
			return 11;
		case 'C':
		case 'c':
			return 12;
		case 'D':
		case 'd':
			return 13;
		case 'E':
		case 'e':
			return 14;
		case 'F':
		case 'f':
			return 15;
		default:
			break;
	}
	return 0;
}

//
int cdev(const char * cmd)
{
	int k = 0;

	FILE *fstream=NULL;    
	char buff[1024];  
	memset(buff,0,sizeof(buff));

	if(NULL==(fstream=popen("ls -l /dev/video*","r")))    
	{   
		//fprintf(stderr,"execute command failed: %s",strerror(errno));
		return -1;    
	}   

	while(NULL!=fgets(buff, sizeof(buff), fstream))   
	{   
		printf("%s",buff);
		k++;
	} 

	pclose(fstream);  
	return k;
}

int rm_task(const char * tpath)
{
	int k = 0;

	char ecmd[512];
	memset(ecmd,'\0',sizeof(ecmd));
	
	snprintf(ecmd,sizeof(ecmd),"rm -rf %s",tpath);
	printfs("%s\t%d\t[DEBUG]:ecmd:%s",__FILE__,__LINE__,ecmd);
	system(ecmd);

	return k;
}

//创建路径
int mkdir_p(const char *fullpath)
{
	int k=-1;

     if (access(fullpath,F_OK|R_OK|W_OK|X_OK) > -1)  
	{
		return 1;
	}
     
	k=strlen(fullpath);
	k += 64;
	char buf[k+1];
	char cbuf[k+1];

	strncpy(buf,fullpath,k);
	memset(cbuf,'\0',sizeof(cbuf));

	if(buf[0]=='/') strcat(cbuf,"/");

	char *token=strtok(buf,"/");
	k=0;
	while(token!=NULL){
		strcat(cbuf,token);
		strcat(cbuf,"/");

		//printfs("%s\t%d\t[DEBUG]: mkdir_p k:%d cbuf:%s",__FILE__,__LINE__,k,cbuf);
		if (access(cbuf,F_OK|R_OK|W_OK|X_OK) == -1)  
		{
			k=mkdir(cbuf,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
		token=strtok(NULL,"/");
		k++;
	}
	return k;
}

/*
*复制函数
*复制文件
*/
int cp_file(char *destination_path,char* source_path)
{  
    char buffer[1024];  
    FILE *in,*out;//定义两个文件流，分别用于文件的读取和写入int len;  
    if((in=fopen(source_path,"r"))==NULL){//打开源文件的文件流  
        printfs("%s\t%d\t[DEBUG]:源文件打开失败:%s",__FILE__,__LINE__,source_path);
        return -1; 
    }  
    if((out=fopen(destination_path,"w"))==NULL){//打开目标文件的文件流  
        printfs("%s\t%d\t[DEBUG]:目标文件创建失败！:%s",__FILE__,__LINE__,destination_path); 
        return -2;  
    }  
    int len;//len为fread读到的字节长  
    while((len=fread(buffer,1,1024,in))>0){//从源文件中读取数据并放到缓冲区中，第二个参数1也可以写成sizeof(char)  
        fwrite(buffer,1,len,out);//将缓冲区的数据写到目标文件中  
    }  
    fclose(out);  
    fclose(in);
    
    return 0;
}

/*
*sendmail发送邮件
*const char *buf         邮件内容
*const char *subject     邮件标题
*const char *attach      附件全路径
*const char *to_addr     收件人邮箱地址
*/
void send_email(const char *buf,const char *subject,const char *attach,const char *to_addr)
{
     //echo "this is my test mail" | mail  -s 'mail test' -a a.txt   1920849305@qq.com
     ////sendmail 发送格式
     char send_email[1024];
     snprintf(send_email,sizeof(send_email),"echo \"%s\" | mail  -s '%s' -a %s %s",buf,subject,attach,to_addr);
     printfs("%s\t%d\t[DEBUG]:send_email:%s",__FILE__,__LINE__,send_email); 
     system(send_email);
}
