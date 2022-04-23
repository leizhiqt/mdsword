/********************************** 
 * @author	leizhifesker@gmail.com
 * @date	2017/02/16
 * Last update:	2017/02/16
 * License:	LGPL
 * 
**********************************/

#ifndef _THPOOL_
#define _THPOOL_

/*
*开辟一个线程池thpool
*每一个线程对应一个任务队列
*一个任务队列对应多个任务  任务调度 根据算法处理
*/

/*
*线程池thpool
*线程池加入任务队列
*/

/**
	线程节点
**/
typedef struct _th_node th_node_t;
typedef struct _th_node{
	pthread_t thread;		//线程ID
	unsigned int lock;		//是否可用
	pthread_mutex_t mutex;
	void* (*function)(void* arg);	//函数指针
	void*	arg;			//函数参数
	th_node_t* prev;		//指向上一个节点
	th_node_t* next;		//指向下一个节点
}th_node_t;

/**
	线程池
**/
typedef struct _thpool{
	int 		max_threads;	//线程数
	int 		min_threads;	//线程数
	th_node_t*	head;		//头指针
	th_node_t*	last;		//末尾指针
	pthread_mutex_t mutex;		//访问锁
}thpool_t;

int thpool_init(thpool_t* thpool,const int max_threads,const int min_threads);
pthread_t thpool_add_work(thpool_t* thpool, void* (*function)(void*), void* arg);
void thpool_destroy(thpool_t* thpool);

 #endif
