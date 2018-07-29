/*
*文件名：queue.c
*功能：实现异步队列
*作者：Xiaodong Pan
*/
#include <stdlib.h>
#include <string.h>
#include "queue.h"


/*判断队列是否为空*/
inline int queue_empty(queue q) 
{
    return q->size == 0;
}


/*判断队列是否为慢*/
inline int queue_full(queue q) 
{
    return q->size == q->capacity;
}


/*置空队列*/
inline void make_empty(queue q)
{
    q->size = 0;
    q->front = q->head->next;
    q->rear = q->head->next;

}


/*判断队列中元素是否不多，为压缩传输的策略准备*/
inline int queue_near_rear(queue q)
{
    if(q->size < QUEUE_THRESHOLD)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*创建指定长度的队列*/
queue create_queue(int max_element) 
{
    int i;
    char *data;
    queue q;
    queue_elem *elem, *p;

    if(max_element < MIN_QUEUE_SIZE)
    {
        error("Createqueue Error: queue size is too small.");
    }

    q = (queue_record *)malloc(sizeof(queue_record));
    if (q == NULL)
    {
        fatal_error("Createqueue Error: Unable to allocate more memory.");
    }
    
    sem_init(&(q->empty), 0, max_element);
    sem_init(&(q->full), 0, 0);
    
	/*创建队列*/
    q->head =(queue_elem *)malloc(sizeof(queue_elem));
    if (q->head == NULL)
    {
        fatal_error("Createqueue Error: Unable to allocate more memory.");
    }
    else
    {
        q->head->next = NULL;
    }

    p = q->head;
    for(i = 0; i < max_element; i++)
    {
        elem = (queue_elem *)malloc(sizeof(queue_elem));
        if(NULL == elem)
        {
            fatal_error("Cannot malloc memory for queue node!\n");
        } 
        else
        {
            elem->offset = 0;
            elem->len = 0;
			/*每个队列元素存在一把锁*/
			pthread_mutex_init(&elem->lock, NULL);
            p->next = elem;
            p = p->next;
        }
    }
    
    p->next = q->head->next;
    q->capacity = max_element;
	/*由于要对q->size进行操作，暂时采用tlock对其进行锁定*/
    pthread_mutex_init(&tlock, NULL);
    make_empty(q);

    return q;
}


/*拷贝队列元素*/
int copy_elem(queue_elem *src, queue_elem *dest)
{		
	int real_len = src->len & BLK_IMAGE_OPP;
	dest->offset = src->offset;
	dest->len = src->len;
	memcpy(dest->data, src->data, real_len);
}


/*增加队列长度*/
inline void queue_size_add(queue qu)
{
   	pthread_mutex_lock(&tlock);
    qu->size++;
 	pthread_mutex_unlock(&tlock);
}


/*减少队列长度*/
inline void queue_size_sub(queue qu)
{
   	pthread_mutex_lock(&tlock);
    qu->size--;
 	pthread_mutex_unlock(&tlock);
}


/*将元素插入队列*/
int queue_put(queue q, queue_elem *elem)
{
	int all_elem_size;
    unsigned int offset;
	queue_elem *src;
	queue_elem *dest;

	/*获取队列空的信号量，如果没有获取到，则阻塞*/
    sem_wait(&(q->empty));	
	/*锁定队列元素*/
	pthread_mutex_lock(&q->rear->lock);

	src = elem;
	dest = q->rear;
	copy_elem(src, dest);	

    q->rear = q->rear->next;
    queue_size_add(q);
	
	/*将队列满的信号量加一*/
    sem_post(&(q->full));
			
	return 0;
}


/*从队列中取出元素，在这里实现了一个零拷贝的获取操作，但是一定要记得解锁*/
queue_elem* queue_get(queue q, queue_elem *elem)
{
	int all_elem_size;
	queue_elem *front;

	/*等待队列满的信号量，如果队列为空，则阻塞*/
    sem_wait(&(q->full));

	front = q->front;
    q->front = q->front->next;
	elem = front;

    queue_size_sub(q);
	/*将队列为空的信号量加一*/
    sem_post(&(q->empty));

	return front;
}
