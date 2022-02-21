#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#ifndef BUF_SIZE        /* Allow "gcc -D" to override definition, for example gcc -DBUF_SIZE=2048 */
#define BUF_SIZE 1024
#endif

void errExit(const char *format, ...)
{
    va_list argList;
    va_start(argList, format);
    vfprintf(stderr, format, argList);
    fprintf(stderr, " [ %s ] \n", strerror(errno));
    va_end(argList);

    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int inputFd, outputFd, openFlags;
    mode_t filePerms;
    char buf[BUF_SIZE];

    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        fprintf(stderr, "%s old-file new-file\n", argv[0]);

    /* Open input file */

    inputFd = open(argv[1], O_RDONLY);
    if (inputFd == -1)
        errExit("opening file %s", argv[1]);

    /* Open output file */
    openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
    S_IROTH | S_IWOTH;      /* rw-rw-rw- permission */

    outputFd = open(argv[2], openFlags, filePerms);
    if (outputFd == -1)
        errExit("opening file %s", argv[2]);
    /* ... */
    /* Transfer data */

    ssize_t numRead;
    while ((numRead=read(inputFd, buf, BUF_SIZE)) > 0)
        if(write(outputFd, buf, numRead) == -1)
            break;

    /* ... */
    exit(EXIT_SUCCESS);
}
