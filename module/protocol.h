#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

//server 文件发送
int send_file(const int sock_fd,const char *fname,const int websocket);

//server 协议返回
int send_pack(thclient_t *thclient);

//server 协议API
int recv_protocol(const char *req);

//server 协议握手
int recv_shake_hand(thclient_t *thclient);

void send_eof(int sockfd);

#endif
