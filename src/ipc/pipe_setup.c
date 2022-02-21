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

int main(void)
{
    int filedes[2];

    if (pipe(filedes) == -1)
        errExit("pipe");                     /* Create the pipe */

    switch (fork()) {
        case -1:
            errExit("fork");                 /* Create a child process */

        case 0: /* Child */
            if (close(filedes[1]) == -1)     /* Close unused write end */
                errExit("close");

            /* Child now reads from pipe */
            break;

        default: /* Parent */
            if (close(filedes[0]) == -1)     /* Close unused read end */
                errExit("close");

            /* Parent now writes to pipe */
            break;
    }

    return 0;
}

