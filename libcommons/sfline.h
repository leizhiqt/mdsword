#ifndef _SFLINE_H_
#define _SFLINE_H_

//去掉回车换行字符串
void brline(const char *ch,const int k);

//字符串是否文件已存在
int exist_line(const char *file_name,const char *buf);

int save_txt(const char *file_name,const char *buf);

int save_append(const char *file_name,const char *buf);

int save_binary(const char *file_name,const char *buf);

//压缩打包
void tarz(const char *infile, const char *outfile);

/*str_tohex
 *二进制字节数组 转换为 16进制的字符串
 *char* hexs		16进制的字符串
 *const int hexsmax	16进制的字符串长度
 *const char* s		二进制字节数组
 *const int smax	二进制字节数组长度 	
 *return int 0:转换成功 非0:转换失败
*/
int str_tohex(char* hexs,const int hexsmax, const char* s, const int smax);

/*str_tostr
 *16进制的字符串转换为二进制字节数组
 *char* s 			二进制字节数组
 *const char* hexs	16进制的字符串	
 *return int 0:转换成功 非0:转换失败
*/
int str_tostr(char* s, const char* hexs);

//列表设备数目
int cdev(const char * cmd);

//删除目录
int rm_task(const char * tpath);

int mkdir_p(const char *file_name);

/*
*复制函数
*复制文件
*/
int cp_file(char *destination_path,char* source_path);

/*
*sendmail发送邮件
*const char *buf         邮件内容
*const char *subject     邮件标题
*const char *attach      附件全路径
*const char *to_addr     收件人邮箱地址
*/
void send_email(const char *buf,const char *subject,const char *attach,const char *to_addr);

#endif
