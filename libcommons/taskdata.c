/********************************** 
 * @author	leizhifesker@gmail.com
 * @date	2017/02/16
 * Last update:	2017/02/16
 * License:	LGPL
 * 
**********************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "fcntl.h"
#include "log.h"

#include "taskdata.h"

static task_t* task;

int task_init(const int max_tasks)
{
	if(task !=NULL) return 1;

	task = (task_t*) malloc(sizeof(task_t));
	//最大线程数
	if(!max_tasks || max_tasks < 1)
		task->max_tasks = 100;
	else
		task->max_tasks = max_tasks;
	task->head = NULL;
	task->last = NULL;

	return 0;
}

//任务列表刷新
int task_flush(const char *task_name)
{
	task_node_t *p = task->head;
	int busy = 0;
	int k=0;

	again:
	k=0;
	while(p!=NULL)
	{
		if(!strcmp(p->task_name,task_name))
		{
			//printfs("%s\t%d\t[DEBUG]:task exits p->task_name:%s",__FILE__,__LINE__,p->task_name);
			return 0;
		}
		k++;
		p=p->next;
		//printfs("%s\t%d\t[DEBUG]:k=%d lock:%d",__FILE__,__LINE__,k,!(p->lock));
	}

	//printfs("%s\t%d\t[DEBUG]:thpool_add_work for k=%d max_threads=%d",__FILE__,__LINE__,k,thpool->max_threads);
	if(k==task->max_tasks)
	{
		if(busy==10)
		{
			//printfs("%s\t%d\t[DEBUG]:thpool_add_work is full max_threads:%ld k=%d",__FILE__,__LINE__,thpool->max_threads,k);
			return -1;
		}

		busy++;
		p = task->head;
		sleep(1);
		goto again;
	}

	//队列入栈
	p = (task_node_t*) malloc(sizeof(task_node_t));
	p->next = NULL;
	p->prev = NULL;
	memset(p->task_name,'\0',sizeof(p->task_name));
	memcpy(p->task_name,task_name,strlen(task_name));

	//add to last node
	if(task->head == NULL)
		task->head = p;

	if(task->last == NULL)
		task->last = p;
	else
	{
		task->last->next = p;
		p->prev = task->last;
		task->last = p;
	}

	//usleep(1000);
	//printfs("%s\t%d\t[DEBUG]:thpool_add_work new:%ld",__FILE__,__LINE__,p->thread);
	return 1;
}

//任务结束
int task_remove(const char *task_name)
{
	int k=0;
	if(task==NULL || task->head==NULL || task_name==NULL || strlen(task_name)<1)
	return k;

	task_node_t *p = task->head;
	task_node_t *p1 = p;
	while(p!= NULL)
	{	
		k++;

		p1 = p->next;
		//找到节点
		if(!strcmp(p->task_name,task_name))
		{
			if(p==task->head)
			{
				if(p->next!=NULL)
				{
					task->head = p->next;
					p->next->prev = NULL;
				}
				else
				{
					task->head = NULL;
					task->last = NULL;
				}
			}
			else if(p==task->last)
			{
				if(p->prev!=NULL)
				{
					task->last = p->prev;
					p->prev->next = NULL;
				}
				else
				{
					task->head = NULL;
					task->last = NULL;
				}
			}
			else
			{
				p->prev->next = p->next;
				p->next->prev = p->prev;
			}

			free(p);
			p = NULL;
			return k;
		}
		p=p1;
	}

	return 0;
}

int task_exist(const char *task_name)
{
	int k=0;
	if(task==NULL || task->head==NULL || task_name==NULL || strlen(task_name)<1)
	return k;

	task_node_t *p = task->head;
	while(p!=NULL)
	{
		k++;
		if(!strcmp(p->task_name,task_name))
		{
			return k;
		}
		p = p->next;
	}

	return 0;
}

int task_size()
{
	int k=0;
	if(task==NULL || task->head==NULL)
	return k;

	task_node_t *p = task->head;
	while(p!=NULL)
	{
		k++;
		p = p->next;
	}

	return k;
}

void task_destroy()
{
	//printfs("%s\t%d\t[DEBUG]:task_destroy begin",__FILE__,__LINE__);
	task_node_t *p = task->head;
	if(p==NULL) return;

	task_node_t *p1 = NULL;
	do
	{
		//printfs("%s\t%d\t[DEBUG]:task_destroy",__FILE__,__LINE__);
		p1 = p->next;
		free(p);
	}while((p = p1)!= NULL);

	task->head = NULL;
	task->last = NULL;
	free(task);
	task = NULL;
}

//入栈
int task_push(task_node_t *task_node)
{
	return 0;
}
//出栈
int task_pop(task_node_t *task_node)
{
	return 0;
}


