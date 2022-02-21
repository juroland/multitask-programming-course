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

        pause();
    } else {
        if (fork() == 0) {
            while (1) {
                printf("Child 2 : pid : %ld ; pgid : %ld\n", (long)getpid(), (long)getpgid(0));
                sleep(2);
            }
        } else {
            printf("Parent : pid : %ld ; pgid : %ld\n", (long)getpid(), (long)getpgid(0));

            sleep(2);

            if (setpgid(0, pid) < 0)
                perror("parent : setpgid");

            while (1) {
                printf("Parent : pid : %ld ; pgid : %ld\n", (long)getpid(), (long)getpgid(0));
                sleep(2);
            }
        }
    }

    return 0;
}
