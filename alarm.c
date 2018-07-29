#include <stdio.h>
#include <signal.h>
static void timer(int sig) {
    if(sig == SIGALRM) {
        printf("timer\n");
    }
    return;
}

int main(int argc, char* argv[]) {
    signal(SIGALRM, timer);
    while(1)
    {
    alarm(1);
    //sleep(1);
    }
    getchar();
    return 0;
}
