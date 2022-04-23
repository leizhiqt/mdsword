/********************************** 
 * @author	leizhifesker@gmail.com
 * @date	2017/02/16
 * Last update:	2017/02/16
 * License:	LGPL
 * 
**********************************/

#ifndef _LINKLIST_
#define _LINKLIST_

#define byte unsigned char

typedef struct _snode snode_t;
typedef struct _snode{
	char *value;
	//int elemSize;
	//int logicLen;
	//int allocLen;
}snode_t;

/**
	LinkNode
**/
typedef struct _link_node link_node_t;
typedef struct _link_list link_list_t;

typedef struct _link_node{
	unsigned int id;	//ID
	char *pv;		//保存字符串:任务号/异常信息/保存图像

	link_list_t *msg;	//保存信息
	link_list_t *imgs;	//保存图像

	link_node_t* prev;	//指向上一个节点
	link_node_t* next;	//指向下一个节点
}link_node_t;

/**
	LinkList
**/
typedef struct _link_list{
	link_node_t*	head;		//头指针
	link_node_t*	last;		//末尾指针
}link_list_t;

int linklist_init(link_list_t **plink);

int snode_cmp(snode_t *s,snode_t *d);

//入栈
int linklist_push(link_list_t **plink,char *pv);
int linklist_push_msg(link_list_t **plink,char *pv,char *msg);
int linklist_push_img(link_list_t **plink,char *pv,char *img);

//出栈
int linklist_pop(link_list_t **plink,char *pv);
int linklist_pop_msg(link_list_t **plink,char *pv,char *msg);
int linklist_pop_img(link_list_t **plink,char *pv,char *img);

int linklist_exist(link_list_t **plink,char *pv);

int linklist_size(link_list_t **plink);

void linklist_destroy(link_list_t **plink);

 #endif
