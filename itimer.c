#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

static char msg[] = "time is running out\n";
static int len;

// 向标准错误输出信息，告诉用户时间到了
void prompt_info(int signo)
{
    write(STDERR_FILENO, msg, len);   
}

// 建立信号处理机制
void init_sigaction(void)
{
    struct sigaction tact;
   
    /*信号到了要执行的任务处理函数为prompt_info*/
    tact.sa_handler = prompt_info;
    tact.sa_flags = 0;
    /*初始化信号集*/
    sigemptyset(&tact.sa_mask);
    /*建立信号处理机制*/
    sigaction(SIGALRM, &tact, NULL);
}

void init_time()
{
    struct itimerval value;
   
    /*设定执行任务的时间间隔为2秒0微秒*/
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 1000000;
    /*设定初始时间计数也为2秒0微秒*/
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 300000;
    /*设置计时器ITIMER_REAL*/
    setitimer(ITIMER_REAL, &value, NULL);
}

int main()
{
    len = strlen(msg);
    init_sigaction();
    init_time();
    while ( 1 )sleep(3);
    exit(0);
}
