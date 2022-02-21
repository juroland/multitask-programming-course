#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#define BUF_SIZE 4096

void cat(int fds[], int n)
{
    int nread = 0;
    char buf[BUF_SIZE];

    for (int i = 0; i < n; ++i)
    {
        while ((nread = read(fds[i], buf, BUF_SIZE)) > 0)
        {
            write(STDOUT_FILENO, buf, nread);
        }
    }
    exit(0);
}

void cat_stdin()
{
    int fd[] = {STDIN_FILENO};
    cat(fd, 1);
    exit(0);
}

void translate(char *set1, char *set2)
{
    execlp("tr", "tr", set1, set2, NULL);
}

void translate_squeeze(char *set1, char *set2)
{
    execlp("tr", "tr", "-s", set1, set2, NULL);
}

void remove_digits()
{
    translate_squeeze("[:digit:]", " ");
}

void remove_punct()
{
    translate_squeeze("[:punct:]", " ");
}

void replace_uppercase_by_lowercase()
{
    translate("[A-Z]", "[a-z]");
}

void replace_whitespace_by_newline()
{
    translate_squeeze("\\n\\f\\t\\r ", "\n");
}

void sort()
{
    execlp("sort", "sort", NULL);
}

void uniq_count()
{
    execlp("uniq", "uniq", "-c", NULL);
}

typedef void (*task_t)();

#define READ_END 0
#define WRITE_END 1

void run_pipeline(task_t *tasks, int ntask)
{
    if (ntask > 1)
    {
        int pipefd[2];
        pipe(pipefd);
        pid_t pid = fork();
        if (pid == 0)
        {
            close(pipefd[READ_END]);
            dup2(pipefd[WRITE_END], STDOUT_FILENO);
            tasks[0]();
        }
        else
        {
            close(pipefd[WRITE_END]);
            dup2(pipefd[READ_END], STDIN_FILENO);
            run_pipeline(++tasks, --ntask);
        }
    }

    tasks[0]();
}

void scatter(int fds[], int n)
{
    int nread = 0;
    char *line = NULL;
    size_t len = 0;
    FILE *stream = stdin;
    int i = 0;
    while ((nread = getline(&line, &len, stream)) > 0)
    {
        i = (i + 1) % n;
        write(fds[i], line, nread);
        len = 0;
        line = NULL;
    }
}

void create_pipes(int readfds[], int writefds[], int n)
{
    for (int i = 0; i < n; ++i)
    {
        int pipefd[2];
        pipe(pipefd);
        readfds[i] = pipefd[READ_END];
        writefds[i] = pipefd[WRITE_END];
    }
}

void closefds(int fds[], int n)
{
    for (int i = 0; i < n; ++i)
        close(fds[i]);
}

void run_parallel(task_t *tasks, int ntask, int input_fds[], int output_fds[], int nfds)
{
    for (int i = 0; i < nfds; ++i)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            dup2(input_fds[i], STDIN_FILENO);
            dup2(output_fds[i], STDOUT_FILENO);
            closefds(input_fds, i);
            closefds(&input_fds[i + 1], nfds - i - 1);
            closefds(output_fds, i);
            closefds(&output_fds[i + 1], nfds - i - 1);
            run_pipeline(tasks, ntask);
            exit(0);
        }
    }
}

void merge_sum(int fds[], int n)
{
    int pipefd[2];
    pipe(pipefd);

    pid_t pid = fork();
    if (pid == 0)
    {
        close(pipefd[READ_END]);
        dup2(pipefd[WRITE_END], STDOUT_FILENO);
        cat(fds, n);
    }
    closefds(fds, n);
    close(pipefd[WRITE_END]);
    dup2(pipefd[READ_END], STDIN_FILENO);
    execl("merge_sum", "merge_sum", NULL);
}

void run_parallel_pipeline(task_t *tasks, int ntask)
{
    const int n = 2;
    int input_rfds[n];
    int input_wfds[n];

    create_pipes(input_rfds, input_wfds, n);

    pid_t pid = fork();
    if (pid == 0)
    {
        closefds(input_rfds, n);
        scatter(input_wfds, n);
        return;
    }
    closefds(input_wfds, n);

    int output_rfds[n];
    int output_wfds[n];
    create_pipes(output_rfds, output_wfds, n);

    run_parallel(tasks, ntask, input_rfds, output_wfds, n);
    closefds(input_rfds, n);

    closefds(output_wfds, n);
    merge_sum(output_rfds, n);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage : %s filename\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Input file open: %s\n", strerror(errno));
        return -1;
    }

    dup2(fd, STDIN_FILENO);

    const int ntask = 7;
    task_t pipeline[] = {
        cat_stdin,
        remove_digits,
        remove_punct,
        replace_uppercase_by_lowercase,
        replace_whitespace_by_newline,
        sort,
        uniq_count};

    run_parallel_pipeline(pipeline, ntask);
    // run_pipeline(pipeline, ntask);

    return 0;
}