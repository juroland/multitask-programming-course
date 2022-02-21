#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{

    pid_t pid = fork();
    if (pid == 0) {
        printf("Child process (PID = %d, parent PID = %d)\n", getpid(),
                getppid()); 
        sleep(10);
    } else if (pid != -1) {
        printf("Parent process (PID = %d, child PID = %d)\n", getpid(), pid);
        int status;
        while(waitpid(pid, &status, WUNTRACED | WCONTINUED))
        {
            if (WIFSTOPPED(status))
                printf("stopped\n");
            if (WIFCONTINUED(status))
                printf("continued\n");
        }
    } else {
        perror("Failed to fork");
        return 1;
    }
    return 0;
}
