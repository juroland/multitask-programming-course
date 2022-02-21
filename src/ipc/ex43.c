#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

void errExit(const char *format, ...)
{
    va_list argList;
    va_start(argList, format);
    vfprintf(stderr, format, argList);
    fprintf(stderr, " [ %s ] \n", strerror(errno));
    va_end(argList);

    exit(EXIT_FAILURE);
}

void child(int readFd, int writeFd)
{
    int nb[2] = {0};
    read(readFd, nb, sizeof(nb));

    int result = nb[0] + nb[1];
    write(writeFd, &result, sizeof(int));
}


void parent(int readFd, int writeFd)
{
    int nb[2] = {0};
    scanf("%d %d", &nb[0], &nb[1]);

    write(writeFd, nb, sizeof(nb));

    int result = 0;
    read(readFd, &result, sizeof(int));

    printf("%d + %d = %d\n", nb[0], nb[1], result);
}

int main(void)
{
    int dataFd[2] = {0};
    int resultFd[2] = {0};

    if (pipe(dataFd) == -1 || pipe(resultFd) == -1)
        errExit("pipe");

    switch (fork()) {
        case -1:
            errExit("fork"); 

        case 0: /* Child */
            if (close(dataFd[1]) == -1 || close(resultFd[0]) == -1)
                errExit("child : unused pipe close");

            child(dataFd[0], resultFd[1]);

            if (close(dataFd[0]) == -1 || close(resultFd[1]) == -1)
                errExit("child : pipe close");

            break;

        default: /* Parent */
            if (close(dataFd[0]) == -1 || close(resultFd[1]) == -1)
                errExit("parent : unused pipe close");

            parent(resultFd[0], dataFd[1]);

            if (close(dataFd[1]) == -1 || close(resultFd[0]) == -1)
                errExit("parent : pipe close");

            break;
    }

    return 0;
}
