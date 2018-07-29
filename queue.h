/*
 *  FILE   : queue.h
 *  AUTHOR : Jeffrey Hunter
 *  WEB    : http://www.iDevelopment.info
 *  NOTES  : Define queue record structure and
 *           all forward declarations.
 */


#ifndef _queue_h
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#define BLK_SIZE      4096
#define BLK_IMAGE     0xf0000000
#define BLK_IMAGE_OPP 0x0fffffff

#define QUEUE_THRESHOLD 5
#define MIN_QUEUE_SIZE (5)
#define QUEUE_LENGTH (500)
//#define BLK_SIZE 8192
//#define EXIT_SIG 0xFFFF0000
#define EXIT_SIG 0
#define error(str)        fatal_error(str)
#define fatal_error(str)   fprintf(stderr, "%s\n", str)
#define debug(str, data)   do {fprintf(stdout,"[Debug]:info:%s, value:%s\n", str, data); }while(0)

pthread_mutex_t tlock;

typedef struct node{
unsigned int  offset;
unsigned int len;
char data[BLK_SIZE*2];
char buf[BLK_SIZE*2];
pthread_mutex_t lock;
struct node* next;
}blk;


typedef blk queue_elem;
typedef struct {
    sem_t empty;  
    sem_t full;  
    unsigned int capacity;
    unsigned int elem_size;
    unsigned int size;
    queue_elem *head;
    queue_elem *front;
    queue_elem *rear;
}asyn_queue, queue_record, *queue;



//typedef char   uint8_t;
//typedef short  uint16_t;
//typedef int    uint32_t;


int         queue_empty(queue q);
int         queue_full(queue q);
queue       create_queue(int max_element);
void        make_empty(queue q);
int         queue_put(queue q, queue_elem *elem);
queue_elem* queue_get(queue q, queue_elem *elem);


#endif  /* _Queue_h */

