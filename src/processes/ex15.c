#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(void)
{
    uid_t orig_euid;

    printf("uid = %ld, euid = %ld\n", (long)getuid(), (long)geteuid());

    int fd = open("test.txt", O_RDWR);
    if (fd == -1)
        perror("open 1");

    orig_euid = geteuid();
    if (seteuid(getuid()) == -1)
        perror("drop");

    printf("uid = %ld, euid = %ld\n", (long)getuid(), (long)geteuid());

    int fd_b = open("test.txt", O_RDWR);
    if (fd_b == -1)
        perror("open 2");

    if (seteuid(orig_euid) == -1)
        perror("reacquire");

    printf("uid = %ld, euid = %ld\n", (long)getuid(), (long)geteuid());

    return 0;
}
