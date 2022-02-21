#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void handler(int sig) 
{
    printf("I received a signal : %d\n", sig);
}

int main(void)
{
    signal(SIGINT, &handler);

    for ( ; ; ) {
        pause();
    }

    return 0;
}
