#include <unistd.h>

int main()
{
    char c;
    read(STDIN_FILENO, &c, 1);
    return 0;
}
