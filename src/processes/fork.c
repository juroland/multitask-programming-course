#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

int x = 9;

int main(void)
{
    int y = 6;

    pid_t pid = fork();
    if (pid == 0) {
        printf("Child process (PID = %d, parent PID = %d)\n", getpid(),
                getppid()); 
        x *= 2;
        y *= 3;
        printf("x = %d, y = %d\n", x, y);
    } else if (pid != -1) {
        sleep(3);
        printf("Parent process (PID = %d, child PID = %d)\n", getpid(), pid);
        printf("x = %d, y = %d\n", x, y);
    } else {
        perror("Failed to fork");
        return 1;
    }
    return 0;
}
