#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

void handler(int s)
{
    printf("signal %d\n", s);
}

int main(void) {
    signal(SIGINT, &handler);
    printf("pid : %ld\n", (long)getpid());

    sleep(2);

    char buffer[100];
    int nb_read;
    //if((nb_read = read(STDIN_FILENO, buffer, 100)) < 0)
    //{
    //    perror("try to read from stdin");
    //}
    
    while(write(STDOUT_FILENO, "123\n", 4));
    printf("Back\n");

    return 0;
}
