#ifndef __DONGLE_H__
#define __DONGLE_H__

#include "pthread.h"
#include "Dongle_API.h"

//缓冲区大小
#define BUFFER_SIZE 4096

typedef int32_t int32;

//
uint get_limit();

void get_bufxml(uchar *buf);
void get_kdate(uchar *buf);
void get_ver(uchar *buf);
void get_kid(uchar *buf);

//key数量
int dongle_count;

//Key对象
int *dongles;

DONGLE_INFO * pKEYList;

//==========
//加密Key 方法
int DongleEnum(int yes);

void dongle_init();

void dongle_release();

/*write key data
 *文件内容写入到KEY的数据区
 *const char *fullpath 文件路径
 *k		KEY序号
 *return int 0成功 非0失败
*/
int write_data(const char *fullpath,int k);

/*read key data
 *	读取KEY数据区&保存到缓冲区bufxml
 *k		KEY序号
*/
void read_data(int n);

/*read buf limit
 *	读取KEY缓冲区 limit
 *k		KEY序号
*/
void read_limit(int n);

/*	write private data
 *	写入KEY-DATA区
 *k		KEY序号
 *s0	写入数据
 *ofset 偏移字节
 *len	字节长度
*/
int wpd(int k,char *s0,int ofset,int len);

/*	read private data
 *	读取KEY-DATA区
 *k		KEY序号
 *buf	读入缓存
 *ofset 偏移字节 33
 *len	字节长度 9
*/
int rpd(int k,unsigned char* buf,int ofset,int len);

/*
 *	读取授权时间
 *k		KEY序号
 *buf	读取到缓存 需要调用者释放
 *return 返回读取字节数 读取到缓冲区
*/
int rdate(int k);

/*
 *	写入授权时间
 *k		KEY序号
 *udates	写入日期字符串
 *return int 0成功
*/
int wdate(int k,char *udates);

/*
 *	写入授权次数
 *int k	key序号
 *uint c 授权次数
 *return int 0成功
*/
int wlimit(int k,uint c);

/*
 *读取授权次数
 *int k	key序号
 *return uint 授权次数
*/
uint rlimit(int k);

/*
 *	读取授权版本号
 *k		KEY序号
 *return 返回读取字节数 读取到缓冲区
*/
int rver(int k);

/*
 *	写入授权版本号
 *k		KEY序号
 *wbuf	授权版本号
 *return int 0成功
*/
int wver(int k,char *wbuf);

//RSA by KEY
/*
 *	RSA字符串加密
 *buf	原字符串
 *uchar **pbuf	加密后的字符串
 *return 
*/

int str_encode(const uchar *buf,uchar **pbuf);
/*
 *	RSA字符串解密
 *buf	原字符串
 *uchar **pbuf	解密后的字符串
 *return 
*/
int str_decode(const uchar *buf,uchar **pbuf);

//测试方法=================
int RSATest();

void RSAList();

char* hexdump();
int RSA_init();
void RSA_clean();

void print_hid(int n);
#endif
