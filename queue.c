/*
*�ļ�����queue.c
*���ܣ�ʵ���첽����
*���ߣ�Xiaodong Pan
*/
#include <stdlib.h>
#include <string.h>
#include "queue.h"


/*�ж϶����Ƿ�Ϊ��*/
inline int queue_empty(queue q) 
{
    return q->size == 0;
}


/*�ж϶����Ƿ�Ϊ��*/
inline int queue_full(queue q) 
{
    return q->size == q->capacity;
}


/*�ÿն���*/
inline void make_empty(queue q)
{
    q->size = 0;
    q->front = q->head->next;
    q->rear = q->head->next;

}


/*�ж϶�����Ԫ���Ƿ񲻶࣬Ϊѹ������Ĳ���׼��*/
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


/*����ָ�����ȵĶ���*/
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
    
	/*��������*/
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
			/*ÿ������Ԫ�ش���һ����*/
			pthread_mutex_init(&elem->lock, NULL);
            p->next = elem;
            p = p->next;
        }
    }
    
    p->next = q->head->next;
    q->capacity = max_element;
	/*����Ҫ��q->size���в�������ʱ����tlock�����������*/
    pthread_mutex_init(&tlock, NULL);
    make_empty(q);

    return q;
}


/*��������Ԫ��*/
int copy_elem(queue_elem *src, queue_elem *dest)
{		
	int real_len = src->len & BLK_IMAGE_OPP;
	dest->offset = src->offset;
	dest->len = src->len;
	memcpy(dest->data, src->data, real_len);
}


/*���Ӷ��г���*/
inline void queue_size_add(queue qu)
{
   	pthread_mutex_lock(&tlock);
    qu->size++;
 	pthread_mutex_unlock(&tlock);
}


/*���ٶ��г���*/
inline void queue_size_sub(queue qu)
{
   	pthread_mutex_lock(&tlock);
    qu->size--;
 	pthread_mutex_unlock(&tlock);
}


/*��Ԫ�ز������*/
int queue_put(queue q, queue_elem *elem)
{
	int all_elem_size;
    unsigned int offset;
	queue_elem *src;
	queue_elem *dest;

	/*��ȡ���пյ��ź��������û�л�ȡ����������*/
    sem_wait(&(q->empty));	
	/*��������Ԫ��*/
	pthread_mutex_lock(&q->rear->lock);

	src = elem;
	dest = q->rear;
	copy_elem(src, dest);	

    q->rear = q->rear->next;
    queue_size_add(q);
	
	/*�����������ź�����һ*/
    sem_post(&(q->full));
			
	return 0;
}


/*�Ӷ�����ȡ��Ԫ�أ�������ʵ����һ���㿽���Ļ�ȡ����������һ��Ҫ�ǵý���*/
queue_elem* queue_get(queue q, queue_elem *elem)
{
	int all_elem_size;
	queue_elem *front;

	/*�ȴ����������ź������������Ϊ�գ�������*/
    sem_wait(&(q->full));

	front = q->front;
    q->front = q->front->next;
	elem = front;

    queue_size_sub(q);
	/*������Ϊ�յ��ź�����һ*/
    sem_post(&(q->empty));

	return front;
}
