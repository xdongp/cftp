/*
 *  FILE   : testQueue.c
 *  AUTHOR : Jeffrey Hunter
 *  WEB    : http://www.iDevelopment.info
 *  NOTES  : Example program that tests the
 *           Queue implementation.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "queue.h"

queue qu;
int i;
char queue_str[20] = "123456789abcdef9876";
 

void myprintf(char *str, unsigned int size)
{
    unsigned int i;
    for(i = 0; i < size; i++)
    {
        printf("%c",*(str+i));
    }
    printf("\n");
}

void* work(void *arg)
{
    int num; 
    while(1)
        {
        i = random()%10;
        num = 2;
        queue_elem *elem;
        elem =(queue_elem *)malloc(sizeof(queue_elem));
        elem->offset = i+num;
        elem->len = i+num;
        memcpy(elem->data, &queue_str, i+num);
        queue_put(qu, elem);
        printf("---------------------------work sleep %d sec\n",num);
        sleep(num);
        }
}

void* consume(void *arg)
{
    int num; 
    queue_elem *elem;
    while(1)
    {
        elem = queue_get(qu, elem);
        num = 1;
        printf("offset:%d  len:%d  ", elem->offset, elem->len);
        myprintf(elem->data,elem->len);
        printf("consumer sleep %d sec\n",num);
        sleep(num);
    }
}

main() {

    pthread_t writer;
    pthread_t reader;
    printf("\n");
    printf("Create Queue(10)...\n\n");
    qu = create_queue(10);
  
 
    pthread_create(&writer, NULL, (void *)work, NULL);
    pthread_create(&reader, NULL, (void *)consume, NULL);
   
    while(1)
    {
    printf("==================================queue size:%d    =============\n",qu->size);
    sleep(1);
    }
   
     
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);
    return 0;

}
