/********************************** 
 * @author	leizhifesker@gmail.com
 * @date	2017/02/16
 * Last update:	2017/02/16
 * License:	LGPL
 * 
**********************************/

#ifndef _TASKDATA_
#define _TASKDATA_

/**
	节点
**/
typedef struct _task_node task_node_t;

typedef struct _task_node{
	unsigned int task_id;		//ID
	char task_name[128];		//任务名称
	
	task_node_t* prev;		//指向上一个节点
	task_node_t* next;		//指向下一个节点
}task_node_t;

/**
	任务
**/
typedef struct _task{
	int 		max_tasks;	//线程数
	task_node_t*	head;		//头指针
	task_node_t*	last;		//末尾指针
}task_t;

int task_init(const int max_tasks);

int task_flush(const char *task_name);

int task_remove(const char *task_name);

int task_exist(const char *task_name);

int task_size();

void task_destroy();

int task_push(task_node_t *task_node);

int task_pop(task_node_t *task_node);
 #endif
