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

    for (int i = 0 ; ; ++i) {
        printf("%d\n", i);
        sleep(3);
    }

    return 0;
}
