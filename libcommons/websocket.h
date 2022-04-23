/*
	WebSocket协议
*/

#ifndef _WEBSOCKET_H_
#define _WEBSOCKET_H_

//协议握手key
char * computeAcceptKey(const char * buf);

//协议握手
void shakeHand(const char *serverKey,char *responseHeader);

//解析数据
char * analyData(const char * buf,const int bufLen);

//字符串数据打包
char * packData(const char * message,unsigned int *nsize,int frame);

//二进制数据打包
char * packBinry(const char * message,unsigned int *nsize,int frame);

//websocket协议发送	关闭帧
int send_ws_close(int csockfd);

//websocket协议发	消息帧
int send_ws_data(const int csockfd,const char * message);

#endif
