#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(void)
{
    printf("pid : %ld\n", (long)getpid());

    pid_t pid;
    if ((pid = fork()) == 0) {
        printf("Child : pid : %ld ; pgid : %ld\n", (long)getpid(), (long)getpgid(0));

        if (setpgid(0, 0) < 0)
            perror("child 1 : setpgid");

        while (1) {
            printf("Child : pid : %ld ; pgid : %ld\n", (long)getpid(), (long)getpgid(0));
            sleep(2);
        }

        pause();
    } else {
        while (1) {
            printf("parent : pid : %ld ; pgid : %ld\n", (long)getpid(), (long)getpgid(0));
            sleep(2);
        }
    }

    return 0;
}
