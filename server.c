#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "readfile.h"

#define LENGTH_OF_LISTEN_QUEUE 1 //length of listen queue in server
#define BUFFER_SIZE 255
#define WELCOME_MESSAGE "welcome to connect the server. "
#define DEFAULT_PORT 20000
#define INTERVAL 1 //sec
#define BUF_INTERVAL 100000 //usec
#define BUF_SEND_TIME (INTERVAL * 1000000 / BUF_INTERVAL)

typedef struct{
unsigned int batch_send_byte;
unsigned int batch_time;
unsigned int send_byte;
}send_record; 


/*缓存数据的队列*/
queue qu;

/*开始限速的标准*/
int open_limit = 0;

/*速度限速的速率,默认为10M*/
int limit_rate = 1024 * 1024 * 10;

/*限速采用的信号量*/
sem_t send_buffer_empty;

blk *fblk;
int i, fd, servfd, clifd;

/*限速用来记录速率的结构*/
send_record srd;
 
static int send_rate_limit()
{
        /*等待缓冲可以发送*/
    int buf_send_byte;
    sem_wait(&send_buffer_empty);
    
    buf_send_byte = limit_rate / BUF_SEND_TIME;
    if(srd.batch_send_byte <  buf_send_byte)
    {
        sem_post(&send_buffer_empty);
    }
    return 0;    
}


inline void init_timer()
{
    struct itimerval value;
   
    /*设定执行任务的时间间隔为0秒100000微秒*/
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 100000;
    /*设定初始时间计数也为BUF_INTERVAL*/
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = BUF_INTERVAL;
    /*设置计时器ITIMER_REAL*/
    setitimer(ITIMER_REAL, &value, NULL);
}


static void timer_clean(int sig)
{
   if(sig == SIGALRM)
    {
        srd.batch_time++;
        if(srd.batch_time == BUF_SEND_TIME)
        {
            printf("---------cftp send rate: %f M---------------\n", srd.send_byte/1024.0/1024);
            srd.batch_time = 0;
            srd.send_byte = 0;
        }
        srd.send_byte += srd.batch_send_byte;
        srd.batch_send_byte = 0;
        sem_post(&send_buffer_empty);
    }
    return ;
}


static void* rate_control()
{
    printf("start transmission...\n");
    init_timer();
    signal(SIGALRM, timer_clean);
    while(1)
    {
        sleep(INTERVAL);
    }
}


void* work(void *arg)
{
    while(1)
	{
        if(queue_near_rear(qu))
        {
            if( read_blk(fblk) == NULL)
			    break;
        }
        else
        {
	 	    if( read_and_compress(fblk) == NULL)
			    break;
        }
        
        queue_put(qu, fblk);
    }
	/*临时的通信方法,告诉传输端已经结束*/
	fblk->len = EXIT_SIG;
	queue_put(qu, fblk);
	
	//printf("==================read finish=====================\n");
}

void* consume(void *arg)
{
    int n;
    queue_elem *elem;
    int real_len;
    while(1)
    {
        /*开启限速共功能*/
        if(open_limit)
        {
            send_rate_limit();
        }

        elem = queue_get(qu, elem);

		if(elem->len == EXIT_SIG)
		{
			if ((n = send(clifd, &elem->len, sizeof(unsigned int), 0)) != sizeof(unsigned int))
			{
				printf("[ERROR]:write len error,expect write:%5d real write len:%5d\n",  sizeof(unsigned int), n);
				exit(1);
			}
			break;
		}

        if ((n = send(clifd, &elem->len, sizeof(unsigned int), 0)) != sizeof(unsigned int))
		{
			printf("[ERROR]:write len error,expect write:%5d real write len:%5d\n",  sizeof(unsigned int), n);
			exit(1);
		}

    	if( elem->len & BLK_IMAGE)
	    {
		    real_len = elem->len & BLK_IMAGE_OPP;
	    }
	    else
	    {
		    real_len = elem->len;
	    }
 
        if ((n = send(clifd, elem->data, real_len, 0)) != real_len)
		{
			printf("[ERROR]:write len error,expect write:%5d real write len:%5d\n", real_len, n);
			exit(1);
		}
		else
		{
            srd.batch_send_byte += BLK_SIZE;//real_len;
		}
		pthread_mutex_unlock(&elem->lock);
    }

	printf("==================Finished transmission!======================\n");
}

int version()
{
    char *version = "CFTP SERVER 1.0.0.0";
    printf("%s\n", version);
    exit(0);
}

int usage()
{
    printf("%s", ("===========================================================\n\
CFTP is transmission tools,which transmission speed may be greater than the bandwidth!\n\
Usage:  cftp-serv -f filename -p port -l [limit-rate(MB)] [-v]\n\
Author: Xiaodong Pan\n\
Date:   2010-02-22\n"));
    printf("===========================================================\n");
    exit(0);
}

int main(int argc, char **argv)
{
    struct sockaddr_in servaddr,cliaddr;
    pthread_t writer;
    pthread_t reader;
    pthread_t timer;
    int c, server_port = 0;
    char *filename = NULL;

/*检查参数的正确性*/
    /*默认端口为20000，如果没有指定端口，则使用默认端口*/
    server_port = DEFAULT_PORT;
    while (1) {
        c = getopt(argc, argv, "f:p:l:v");
        if (c == -1)
            break;

        switch (c) {
        case 'f':
            //debug("filename", optarg);
            filename = optarg;
            break;

        case 'p':
            //debug("port", optarg);
            server_port = atoi(optarg);
            break;

        case 'l':
            //debug("limit_rate", optarg);
            limit_rate = atoi(optarg) * 1024 * 1024;
            open_limit = 1;
            break;

        case 'v':
            version();
            break;

        default:
            break;
        }

    }

    if (filename == NULL)
    {
        usage();
    }
    
    printf("filename: %s, port: %d...\n", filename, server_port);
    //printf("argv check ok!\n");


	if ((servfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
	  	printf("create socket error!\n");
	 	exit(1);
	}

    int bReuseaddr=1;
    setsockopt(servfd,SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(int));    

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(server_port);
	servaddr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(servfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
	{
	  	printf("bind to port %d failure!\n",server_port);
	  	exit(1);
	}

	if (listen(servfd,LENGTH_OF_LISTEN_QUEUE) < 0)
	{
	  	printf("call listen failure!\n");
	  	exit(1);
	}

	socklen_t length = sizeof(cliaddr);
	clifd = accept(servfd,(struct sockaddr*)&cliaddr,&length);
	if (clifd < 0)
	{
		printf("error comes when call accept!\n");
		exit(1);
	}
  
      sem_init(&send_buffer_empty, 0, 1);

/*开始读文件*/
  	fblk = init_read(filename);
    qu = create_queue(QUEUE_LENGTH);
    //printf("Create Queue...\n\n");
 /*速度显示*/
    pthread_create(&timer, NULL, (void *)rate_control, NULL);  
    pthread_create(&writer, NULL, (void *)work, NULL);
    pthread_create(&reader, NULL, (void *)consume, NULL);
     
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

	//send(clifd,buf,BUFFER_SIZE,0);
    close(fd);
	close(clifd);         
	close(servfd);
	return 0;

}
