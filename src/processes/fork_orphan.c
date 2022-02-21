#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

int main(void)
{
    pid_t pid = fork();
    if (pid == 0) {
        printf("Child process (PID = %d, parent PID = %d)\n", getpid(),
                getppid()); 
        sleep(15);
        printf("Child process (PID = %d, parent PID = %d)\n", getpid(),
                getppid()); 
    } else if (pid != -1) {
        printf("Parent process (PID = %d, child PID = %d)\n", getpid(), pid);
    } else {
        perror("Failed to fork");
        return 1;
    }
    return 0;
}
