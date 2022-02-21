#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

void errExit(const char *format, ...)
{
    va_list argList;
    va_start(argList, format);
    vfprintf(stderr, format, argList);
    fprintf(stderr, " [ %s ] \n", strerror(errno));
    va_end(argList);

    exit(EXIT_FAILURE);
}

void child_ls(int pipeFd[2])
{
    if (close(pipeFd[0]) == -1)
        perror("child ls : close unused pipe");

    dup2(pipeFd[1], STDOUT_FILENO);

    execlp("ls", "ls", (char*) NULL);

    errExit("execlp ls");
}


void child_wc(int pipeFd[2])
{
    if (close(pipeFd[1]) == -1)
        perror("child ls : close unused pipe");

    dup2(pipeFd[0], STDIN_FILENO);

    execlp("wc", "wc", "-l", (char*) NULL);

    errExit("execlp wc");
}

void create_child(void (*child_function)(int pipeFd[2]), int pipeFd[2])
{
    switch (fork()) {
        case -1:
            errExit("fork"); 

        case 0: /* Child */
            child_function(pipeFd);
            break;

        default: /* Parent */
            break;
    }
}

int main(void)
{
    int pipeFd[2] = {0};

    if (pipe(pipeFd) == -1)
        errExit("pipe");

    create_child(child_ls, pipeFd);
    create_child(child_wc, pipeFd);

    if (close(pipeFd[0]) == -1 || close(pipeFd[1]) == -1)
        perror("parent: close pipe");

    wait(NULL);
    wait(NULL);

    return 0;
}
