#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

static char msg[] = "time is running out\n";
static int len;

// ���׼���������Ϣ�������û�ʱ�䵽��
void prompt_info(int signo)
{
    write(STDERR_FILENO, msg, len);   
}

// �����źŴ������
void init_sigaction(void)
{
    struct sigaction tact;
   
    /*�źŵ���Ҫִ�е���������Ϊprompt_info*/
    tact.sa_handler = prompt_info;
    tact.sa_flags = 0;
    /*��ʼ���źż�*/
    sigemptyset(&tact.sa_mask);
    /*�����źŴ������*/
    sigaction(SIGALRM, &tact, NULL);
}

void init_time()
{
    struct itimerval value;
   
    /*�趨ִ�������ʱ����Ϊ2��0΢��*/
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 1000000;
    /*�趨��ʼʱ�����ҲΪ2��0΢��*/
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 300000;
    /*���ü�ʱ��ITIMER_REAL*/
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
