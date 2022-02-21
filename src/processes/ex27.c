#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{

    pid_t pid = fork();
    if (pid == 0) {
        printf("Child process (PID = %d, parent PID = %d)\n", getpid(),
                getppid());
        execvp(argv[1], argv+1);
    } else if (pid != -1) {
        printf("Parent process (PID = %d, child PID = %d)\n", getpid(), pid);
    } else {
        perror("Failed to fork");
        return 1;
    }
    wait(NULL);
    return 0;
}
