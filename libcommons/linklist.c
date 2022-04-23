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
#include "linklist.h"

int snode_cmp(snode_t *s,snode_t *d)
{
	int k=-1;

	if(s==NULL || d==NULL)
		return k;
	
	if(!(k=strcmp(s->value,d->value)))
	{
		//printfs("%s\t%d\t[DEBUG]:task exits p->task_name:%s",__FILE__,__LINE__,p->obj);
	}
	return k;
}

int linklist_init(link_list_t **plink)
{
	link_list_t *p = (link_list_t*) malloc(sizeof(link_list_t));
	p->head = NULL;
	p->last = NULL;

	*plink = p;//*plink 改变传地址的值 指针
	return 0;
}

//入栈
int linklist_push(link_list_t **plink,char *pv)
{
	link_list_t *linklist = *plink;
	link_node_t *p = linklist->head;
	int k=0;
	while(p!=NULL)
	{
		printfs("%s\t%d\t[DEBUG]:p->obj:%s pv:%s",__FILE__,__LINE__,p->pv,pv);
		if(!strcmp(p->pv,pv))
		{
			//printfs("%s\t%d\t[DEBUG]:task exits p->task_name:%s",__FILE__,__LINE__,p->obj);
			return 0;
		}
		k++;
		p=p->next;
		printfs("%s\t%d\t[DEBUG]:k=%d",__FILE__,__LINE__,k);
	}
	//printfs("%s\t%d\t[DEBUG]:",__FILE__,__LINE__);
	//队列入栈
	p = (link_node_t*) malloc(sizeof(link_node_t));
	p->next = NULL;
	p->prev = NULL;

	//printfs("%s\t%d\t[DEBUG]:sizeof:%d strlen:%d",__FILE__,__LINE__,sizeof(pv),strlen(pv));
	p->pv = malloc(strlen(pv));
	memset(p->pv,'\0',sizeof(p->pv));
	memcpy(p->pv,pv,sizeof(pv));

	p->msg = NULL;
	p->imgs = NULL;

	//add to last node
	if(linklist->head == NULL)
		linklist->head = p;

	if(linklist->last == NULL)
		linklist->last = p;
	else
	{
		linklist->last->next = p;
		p->prev = linklist->last;
		linklist->last = p;
	}

	return 1;
}

int linklist_push_msg(link_list_t **plink,char *pv,char *msg)
{

}

//出栈
int linklist_pop(link_list_t **plink,char *pv)
{
	int k=0;
	link_list_t *linklist = *plink;

	if(linklist==NULL || linklist->head==NULL || pv==NULL || strlen(pv)<1)
	return k;

	link_node_t *p = linklist->head;
	link_node_t *p1 = p;
	while(p!= NULL)
	{	
		k++;

		p1 = p->next;
		//找到节点
		if(!strcmp(p->pv,pv))
		{
			if(p==linklist->head)
			{
				if(p->next!=NULL)
				{
					linklist->head = p->next;
					p->next->prev = NULL;
				}
				else
				{
					linklist->head = NULL;
					linklist->last = NULL;
				}
			}
			else if(p==linklist->last)
			{
				if(p->prev!=NULL)
				{
					linklist->last = p->prev;
					p->prev->next = NULL;
				}
				else
				{
					linklist->head = NULL;
					linklist->last = NULL;
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

int linklist_exist(link_list_t **plink,char *pv)
{
	int k=0;
	link_list_t *linklist = *plink;
	
	if(linklist==NULL || linklist->head==NULL || pv==NULL || strlen(pv)<1)
	return k;

	link_node_t *p = linklist->head;
	while(p!=NULL)
	{
		k++;
		if(!strcmp(p->pv,pv))
		{
			return k;
		}
		p = p->next;
	}

	return 0;
}

int linklist_size(link_list_t **plink)
{
	int k=0;
	link_list_t *linklist = *plink;

	if(linklist==NULL || linklist->head==NULL)
	return k;

	link_node_t *p = linklist->head;
	while(p!=NULL)
	{
		k++;
		p = p->next;
	}

	return k;
}

void linklist_destroy(link_list_t **plink)
{
	//printfs("%s\t%d\t[DEBUG]:task_destroy begin",__FILE__,__LINE__);
	link_list_t *linklist = *plink;

	link_node_t *p = linklist->head;
	if(p==NULL) return;

	link_node_t *p1 = NULL;
	do
	{
		//printfs("%s\t%d\t[DEBUG]:task_destroy",__FILE__,__LINE__);
		p1 = p->next;
		free(p);
	}while((p = p1)!= NULL);

	linklist->head = NULL;
	linklist->last = NULL;
	free(linklist);
	linklist = NULL;
}
