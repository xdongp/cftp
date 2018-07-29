#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <fcntl.h>
#include <strings.h>
#include <pthread.h>
#include <signal.h>
#include "minilzo.h"
#include <sys/time.h>
#include "queue.h"


#define CLIENT_PORT ((20001+rand())%65536) // define the defualt client port as a random port
#define REUQEST_MESSAGE "welcome to connect the server.\n"
#define BLK_IMAGE     0xf0000000
#define BLK_IMAGE_OPP 0x0fffffff
#define BUFFER_SIZE 4096*2
#define BLK_SIZE 4096
#define REUQEST_MESSAGE "welcome to connect the server.\n"
#define EXIT_SIG 0
#define DEFAULT_PORT 20000
#define INTERVAL 1 //second

unsigned long int file_len = 0;
unsigned long int com_len = 0;
unsigned long int tmp_len = 0;


void usage()
{
       printf("usage: cftp-recv -i ipaddr -p port -f filename\n");
       exit(1);
}

inline void init_timer()
{
    struct itimerval value;
   
    /*设定执行任务的时间间隔为0秒100000微秒*/
    value.it_value.tv_sec = 1;
    value.it_value.tv_usec = 0;
    /*设定初始时间计数也为BUF_INTERVAL*/
    value.it_interval.tv_sec = INTERVAL;
    value.it_interval.tv_usec = 0;
    /*设置计时器ITIMER_REAL*/
    setitimer(ITIMER_REAL, &value, NULL);
}


static void timer_print(int sig)
{
   if(sig == SIGALRM)
    {
        
        printf("---------cftp send rate: %f MB---------------\n", (file_len -tmp_len)/1024.0/1024/INTERVAL);
        tmp_len = file_len;
    }
    return ;
}


static void* rate_show()
{
    printf("start transmission...\n");
    init_timer();
    signal(SIGALRM, timer_print);
    while(1)
    {
        sleep(INTERVAL);
    }
}





int accurate_recv(int sockfd, char *buf, unsigned len, int flag)
{
    unsigned int expect_recv_len = len;
    unsigned int real_recv_len = 0;
    char *dest = buf;
    unsigned int time=0;
    do
    {
        if((real_recv_len = recv(sockfd, dest, expect_recv_len, flag)) < 0)
        {
            printf("[Error]:recv error, expect_recv_len %d read_recv_len:%5d\n", expect_recv_len, real_recv_len);
            return -1;
        }
        expect_recv_len = expect_recv_len - real_recv_len;
        dest = (char *)(dest + real_recv_len);
        time++;
            
    }
    while(expect_recv_len != 0);

    /*if(time > 1)
    {    
        printf("[Warning]: expect_recv_len:%5d total recv_time:%5d\n", len, time);
    }*/

    return len;
}

int main(int argc, char **argv)
{     
    int fd, clifd, length = 0, com_flag = 1;
    int c, server_port;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t socklen = sizeof(servaddr);
    char buf[BUFFER_SIZE];
    char dst[BUFFER_SIZE];
    unsigned int send_len;
    unsigned int dst_len;
    unsigned int n;
    unsigned read_num = 0;
    char *filename = NULL, *ipaddr = NULL;
    struct timeval tvafter,tvpre;
    struct timezone tz;
    pthread_t timer; 

    /*检查参数的正确性*/
    /*默认端口为20000，如果没有指定端口，则使用默认端口*/
    server_port = DEFAULT_PORT;
 
    while (1) {
        c = getopt(argc, argv, "f:p:i:hv");
        if (c == -1)
            break;

        switch (c) {
        case 'f':
            //debug("filename", optarg);
            filename = optarg;
            break;

        case 'i':
            //debug("ipaddr", optarg);
             ipaddr = optarg;
            break;

        case 'p':
            //debug("port", optarg);
            server_port =  atoi(optarg);
            break;

        case 'v':
            printf("version 1.0.0.0");
            break;

        default:
            usage();
            break;
        }

    }

    if (filename == NULL || ipaddr == NULL)
    {
        error("Filename and ipaddr needed. ");
        usage();
    }
    
    printf("-------------------------------------------------------------------------------------\n");   
    printf("ip: %s, port: %d, filename: %s\n", ipaddr, server_port, filename);
    //printf("argv check ok!\n");


    
    if ((clifd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
         printf("create socket error!\n");
          exit(1);
    }

    fd = open(filename, O_WRONLY|O_CREAT, 0644);


    bzero(&cliaddr,sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(CLIENT_PORT);
    cliaddr.sin_addr.s_addr = htons(INADDR_ANY);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_aton(ipaddr, &servaddr.sin_addr);
    servaddr.sin_port = htons(server_port);

    if (bind(clifd,(struct sockaddr*)&cliaddr,sizeof(cliaddr))<0)
    {
        printf("bind to port %d failure!\n",CLIENT_PORT);
          exit(1);
    }

    if (connect(clifd,(struct sockaddr*)&servaddr, socklen) < 0)
    {
        printf("can't connect to %s!\n",ipaddr);
        exit(1);
    }

    gettimeofday (&tvpre , &tz);
    pthread_create(&timer, NULL, (void *)rate_show, NULL);  
    while(1)
    {
        
        if ((length = accurate_recv(clifd, (char *)&send_len, 4, 0)) != 4)
        {
            printf("[Error]:read length err\n");
            exit(1);
        }
        
        /*if(send_len > BUFFER_SIZE)
        {
            printf("lenght is too long,recv_len: %dsend_len:%d\n", length, send_len);
            exit(1);
        }*/
    
        if(send_len == EXIT_SIG)
        {
            printf("------------------- 传输完成 ----------------------\n");
            //close(clifd);
            break;
        }
        
        if(send_len & BLK_IMAGE)
        {    
            //printf("[uncompress]:1 send_len:%d\n", send_len);
            send_len = send_len & BLK_IMAGE_OPP;
            com_flag = 0;

            //printf("[uncompress]:2 send_len:%d\n", send_len);
        }
        else
        {
            com_flag = 1;
            //printf("[compress]:send_len:%d\n", send_len);
        }

            
        if ((length = accurate_recv(clifd, buf, send_len, 0)) != send_len)
        {
            printf("[Error]:read data err\n");
            break;
        }
        if( !com_flag )
        {
            if((n = write(fd, buf, length)) != length)
            {    
                printf("===write error:write len:%d====\n",n);
                exit(1);
            }
        }
        else
        {
            lzo1x_decompress( (lzo_bytep)buf,(lzo_uint)&send_len,
                          (lzo_bytep)dst,(lzo_intp)&dst_len, NULL);
            if((n = write(fd, dst, dst_len)) != dst_len)
            {    
                printf("===write error:write len:%d====\n",n);
                exit(1);
            }
        }
    
        //printf("num: %5d expect len: %5d  real recv len:%5d dst_len:%5d\n",read_num++, send_len, length, dst_len);
        file_len += dst_len;
        com_len += length;
        
    }
    if (length < 0)
    {
        printf("error comes when recieve data from server %s!",argv[1]);
          exit(1);
    }
    gettimeofday (&tvafter , &tz);

    printf("花费时间:%d\n", (tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000);

    printf("文件长度: %ld 压缩后长度:%ld  节省带宽:%f\%\n",file_len, com_len, 100 * (1 - (float)com_len / file_len));
    close(clifd);
    close(fd);
    printf("-------------------------------------------------------------------------------------\n");   
    return 0;
}
