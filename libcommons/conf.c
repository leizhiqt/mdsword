#include "stdio.h"
#include "string.h"
#include "log.h"
#include "conf.h"

char *cnf_sign = "=";

/*返回str2第一次出现在str1中的位置(下表索引),不存在返回-1*/  
int indexOf(char *str1,char *str2)  
{  
    char *p=str1;  
    int i=0;  
    p=strstr(str1,str2);  
    if(p==NULL)  
        return -1;  
    else{  
        while(str1!=p)  
        {  
            str1++;  
            i++;  
        }  
    }  
    return i;  
}  
/*返回str1中最后一次出现str2的位置(下标),不存在返回-1*/  
int lastIndexOf(char *str1,char *str2)  
{  
    char *p=str1;  
    int i=0,len=strlen(str2);  
    p=strstr(str1,str2);  
    if(p==NULL)return -1;  
    while(p!=NULL)  
    {  
        for(;str1!=p;str1++)i++;  
        p=p+len;  
        p=strstr(p,str2);  
    }  
    return i;  
}

/*截取src字符串中,从下标为start开始到end-1(end前面)的字符串保存在dest中(下标从0开始)*/  
void substring(char *dest,char *src,int start,int end)  
{  
    int i=start;  
    if(start>strlen(src))return;  
    if(end>strlen(src))  
        end=strlen(src);  
    while(i<end)  
    {     
        dest[i-start]=src[i];  
        i++;  
    }  
    dest[i-start]='\0';  
    return;  
}  

/*
 *从配置文件中读取相应的值
 *输入参数：1，配置文件路径 2，匹配标记 3，输出存储空间
 *并且排除了空行，“=”前后无内容，无“=”的情况
 */
int conf_read(char *conf_path,char *conf_name,char *cnf_val)
{
    char cnf_buf[LINE_SIZE];
    memset(cnf_buf,'\0',sizeof(cnf_buf));
        
    char linebuf[LINE_SIZE];
    
     char lkey[LINE_SIZE];
     char lval[LINE_SIZE];
     
    FILE *f;
    f = fopen(conf_path,"r+");
    if(f == NULL)
    {
        printf("OPEN CONFIG FALID\n");
        return 0;
    }

     int on_line = 0;
     
    while(fgets(linebuf,LINE_SIZE,f) != NULL)
    {   
          //printf("config_linebuf:%s\n",linebuf);
        int i = indexOf(linebuf,cnf_sign);
        substring(lkey,linebuf,0,i);
        substring(lval,linebuf,i+1,strlen(linebuf));
        
        printfs("%s\t%d\t[DEBUG]:key:%s\tvalue:%s",__FILE__,__LINE__,lkey,lval);
    }
     fclose(f);
}

int conf_load(char *conf_path)
{
     FILE *f;

     f = fopen(conf_path,"r+");
     if(f == NULL)
     {
          printf("OPEN CONFIG FALID\n");
          return 0;
     }

     kc = 0;
     char linebuf[LINE_SIZE];
     
     while(fgets(linebuf,LINE_SIZE,f) != NULL)
     {   
          //printf("config_linebuf:%s\n",linebuf);
          if(linebuf[0]=='#') continue;
          
         int i = indexOf(linebuf,cnf_sign);
         substring(kv[kc][0],linebuf,0,i);
         substring(kv[kc][1],linebuf,i+1,strlen(linebuf)-1);

         //printf("key:%s\tvalue:%s\n",kv[kc][0],kv[kc][1]);
         kc++;
     }
     //printf("kc=%d\n",kc);
     fclose(f);
}

/*
 *添加修改文件（当配置文件中存在标记字段，则进行修改，若不存在则进行添加）
 *
 *输入参数：1，配置文件路径 2，匹配标记 3，替换或添加的内容
 *
 */
int conf_update(char *conf_path,char *conf_name,char *conf_value)
{
    char cnf_buf[LINE_SIZE];
    memset(cnf_buf,'\0',sizeof(cnf_buf));
        
    char linebuf[LINE_SIZE];
    
     char lkey[LINE_SIZE];
     char lval[LINE_SIZE];
     char lread[LINE_SIZE];
     
    FILE *f;
    f = fopen(conf_path,"r+");
    if(f == NULL)
    {
        printf("OPEN CONFIG FALID\n");
        return 0;
    }

     int on_line = 0;
     
    while(fgets(linebuf,LINE_SIZE,f) != NULL)
    {   
          //printf("config_linebuf:%s\n",linebuf);
          snprintf(lread,strlen(linebuf),"%s",linebuf);
           /*         
        	char *token=strtok(linebuf,cnf_sign);
        	int k=0;
	     while(token!=NULL){
		     //printfs("%s\t%d\t[DEBUG]: recv_cmd ERROR k:%d token:%s",__FILE__,__LINE__,k,token);
		     switch(k)
		     {
			     case 0://key
			          printf("%s=",token);
			          snprintf(lkey,strlen(token)+1,"%s",token);
				     break;
			     case 1://value
			          printf("%s\n",token);
			          snprintf(lval,strlen(token),"%s",token);
				     break;
			     default:
				     break;
		     }

		     token=strtok(NULL,cnf_sign);
		     k++;
	     }
	     */
	     //printf("%s||%s\n",lkey,conf_name);

	     if(!strcmp(lkey,conf_name)){
	          strcat(cnf_buf,conf_name);
               strcat(cnf_buf,"=");
               strcat(cnf_buf,conf_value);
               strcat(cnf_buf,"\n");
               
               on_line=1;
	          //break;
	     }else{
	          strcat(cnf_buf,lread);
	          strcat(cnf_buf,"\n");
	     }
    }
     remove(conf_path);
     fclose(f);

     if(!on_line){
          strcat(cnf_buf,conf_name);
          strcat(cnf_buf,"=");
          strcat(cnf_buf,conf_value);
          strcat(cnf_buf,"\n");
          
          on_line=1;
     }
     
     FILE *fp;
     fp = fopen(conf_path,"w+");
     if(fp == NULL)
     {
          printf("OPEN CONFIG FALID\n");
          return 2;
     }
     //fseek(fp,0,SEEK_SET);
     
     //printf("cnf_buf>%s\n",cnf_buf);
     fputs(cnf_buf,fp);
     fclose(fp);
}
/*
 *删除配置文件内容（
 *
 *输入参数：1，配置文件路径 2，匹配标记 
 *
 */
int conf_delete(char *conf_path,char *conf_name)
{
    char cnf_buf[LINE_SIZE];
    memset(cnf_buf,'\0',sizeof(cnf_buf));
        
    char linebuf[LINE_SIZE];
    
     char lkey[LINE_SIZE];
     char lval[LINE_SIZE];
     char lread[LINE_SIZE];
     
    FILE *f;
    f = fopen(conf_path,"r+");
    if(f == NULL)
    {
        printf("OPEN CONFIG FALID\n");
        return 0;
    }

     int on_line = 0;
     
    while(fgets(linebuf,LINE_SIZE,f) != NULL)
    {   
          //printf("config_linebuf:%s\n",linebuf);
          snprintf(lread,strlen(linebuf),"%s",linebuf);
          /*
        	char *token=strtok(linebuf,cnf_sign);
        	int k=0;
	     while(token!=NULL){
		     //printfs("%s\t%d\t[DEBUG]: recv_cmd ERROR k:%d token:%s",__FILE__,__LINE__,k,token);
		     switch(k)
		     {
			     case 0://key
			          printf("%s=",token);
			          snprintf(lkey,strlen(token),"%s",token);
				     break;
			     case 1://value
			          printf("%s\n",token);
			          snprintf(lval,strlen(token),"%s",token);
				     break;
			     default:
				     break;
		     }

		     token=strtok(NULL,cnf_sign);
		     k++;
	     }
	     
	     if(strcmp(lkey,conf_name) && k==2){
	          strcat(cnf_buf,lread);
	          strcat(cnf_buf,"\n");
	     }*/
    }
     remove(conf_path);
     fclose(f);

     FILE *fp;
     fp = fopen(conf_path,"w+");
     if(fp == NULL)
     {
          printf("OPEN CONFIG FALID\n");
          return 2;
     }
     //fseek(fp,0,SEEK_SET);
     
     //printf("cnf_buf>%s\n",cnf_buf);
     fputs(cnf_buf,fp);
     fclose(fp);
}


 /*
int conf_append(char *conf_path,char *conf_name,char *conf_value)
{
     char cnf_buf[LINE_SIZE];
     memset(cnf_buf,'\0',sizeof(cnf_buf));

     char *cnf_sign = "=";

     FILE *fp;
     fp = fopen(conf_path,"a+");
     if(fp == NULL)
     {
          printf("OPEN CONFIG FALID\n");
          return 0;
     }

     strcat(cnf_buf,conf_name);
     strcat(cnf_buf,cnf_sign);
     strcat(cnf_buf,conf_value);
     strcat(cnf_buf,"\n");

     fputs(cnf_buf,fp);
     fclose(fp);
}
*/
