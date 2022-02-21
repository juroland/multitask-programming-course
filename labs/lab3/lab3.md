---
title: 'Lab 3 : Program Execution, Inter-Process Communication'
---

# Program execution

A process can replace the program that it is running by a new program with one
of the `exec()` family of functions (`man 3 exec`). For example, the `execv()`
system call is declared as follows.

```C
#include <unistd.h>
int execv(const char *pathname, char * const argv[]);
```

This function never returns on success and returns `-1` on error. The first
argument, `pathname`, is the pathname of the program to be loaded in this
process's memory. The second argument, `argv`, is a `NULL` terminated array
that contains the command-line arguments to be passed to this program. This
argument corresponds to the `argv` argument of the C `main()` function.

For example, the following program creates a child process to run `/bin/ls` with `argv[0]` equal to
`ls` and `argv[1]` equal to `-l`. This list of arguments ends with `NULL`, because `argv` is defined
as a `NULL` terminated array.

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {

   pid_t childpid;

   childpid = fork();
   if (childpid == -1)  {
       perror("Failed to fork");
       return 1; 
   }

   if (childpid == 0) {    /* child code */
       execl("/bin/ls", "ls", "-l", NULL);
       perror("Child failed to exec ls");
       return 1; 
   }

   if (childpid != wait(NULL)) {    /* parent code */
       perror("Parent failed to wait due to signal or error");
       return 1;
   }

   return 0; 
}
```

> Exercise #
>
> Write a program that executes a command passed as parameter, wait
> the return value, and print the returned value. For example, `./launch ps -f`
> executes the command `ps -f` and display its termination status. For this
> purpose, you can use the system call `execvp()` for executing the command with
> its arguments, and `wait()` for suspending the executing of the calling process
> until its children terminates.


Among others, the following attributes are **preserved** after a call to an `exec()` function:

* Process IDs (PID, PPID, PGID, Session ID, Read IDs)
* Open file descriptors (unless the close-on-exec flag `FD_CLOEXEC` is enabled)
* File offsets and status
* Working directory
* Default or ignored signal dispositions

The following attribute are **never** preserved after a call to an `exec()` function:

* Text segment
* Stack segment
* Data segment
* Heap segment
* Shared memory (see later)
* Threads (see later)
* Mutexes and condition variables (see later)
* Semaphores (see later)
* Signal handlers (because the text of the existing process is replaced)

> Exercise #
>
> The shell uses these properties in order to redirect the output of a command in
> a file with commands such as `ls /tmp > dir.txt`. Write a program that executes
> a command passed as parameter and redirects the output in a file also passed as
> parameter. A message on standard output has to be displayed on completion of
> this operation. For this purpose, use the system call `int dup2(int oldfd, int
> newfd)` that allows to duplicate a file descriptor. It closes the entry `newfd`
> in the per-process file descriptor table if it was open. Then, it creates
> a copy of the descriptor referred by `oldfd` to the file descriptor specified
> by `newfd`. After this call, the `oldfd` and `newfd` refer to the same open
> file descriptor. For example, `dup2(fd, STDOUT_FILENO)` implies that the
> standard output refers to the same entry than `fd`.

\newpage
> Exercise #
>
> The following program uses `fcntl()` to enable the *close-on-exec* flag on the
> file descriptor referred by `STDOUT_FILENO` (if an argument is provided).
> Explain the result.

```C
        #include <fcntl.h>
        
        void errExit(const char *format, ...)
        {
            va_list argList;
            va_start(argList, format);
            vfprintf(stderr, format, argList);
            fprintf(stderr, " [ %s ] \n", strerror(errno));
            va_end(argList);
        
            exit(EXIT_FAILURE);
        }
        
        int
        main(int argc, char *argv[])
        {
            int flags;
        
            if (argc > 1) {
                flags = fcntl(STDOUT_FILENO, F_GETFD);       /* Fetch flags */
                if (flags == -1)
                    errExit("fcntl - F_GETFD");
        
                flags |= FD_CLOEXEC;                  /* Turn on FD_CLOEXEC */
        
                if (fcntl(STDOUT_FILENO, F_SETFD, flags) == -1) /* Update flags */
                    errExit("fcntl - F_SETFD");
            }
        
            execlp("ls", "ls", "-l", argv[0], (char *) NULL);
            errExit("execlp");
        }
```

<!-- For security purpose, a set-user-ID program should reset its user IDs to the
same value as the real user ID. This result is achieved by calling
`setuid(getuid())` before a call to an `exec()` function. Moreover, as explained
previously, the saved set-user-ID is replaced by the value of the effective user
ID after a successful call to an `exec()` function. -->

<!--
The UNIX operating system is a **multi-tasking** operating system. Multiple
processes can simultaneously reside in memory and receive use of the CPU(s).
UNIX is also said to be a *preemptive* operating system. This means that a
scheduler determines which process receives use of the CPU and for how long. For
this purpose, the kernel is able to suspend the execution a given process and to
resume the execution of another process. This operation, called *context
switch*, is mainly composed of the following two tasks.

    * Switching the page tables to install a new address space.
    * Save running process hardware context (CPU registers) and replace it with
      the one of the next running process.
-->

\newpage
# Pipe

A **pipe** is an unidirectional data channel that is used for communication
between related processes. This buffer is maintained in kernel memory and is
managed in a first-in-first-out manner.  It is created with the system call
`pipe()`.

```C
/*                              Pipe
 *                    ________________________
 *                   |       byte stream      |
 *     pipefd[1] --> | unidirectional channel | --> pipefd[0]
 *    (write end)    |________________________|    (read end)
 */

#include <unistd.h>
int pipe(int pipefd[2]);
```

The two ends of the pipe are referred by `pipefd[0]` (the read end) and
`pipefd[1]` (the write end). The usual system calls `read()` and `write()` are
used to perform `I/O` on the pipe. This channel is *byte stream*. This means
that there is no concept of message boundaries. Therefore, we are able to read
an arbitrary number of bytes from the pipe.

> Exercise #
>
> Explain the reason why a pipe can be used by the process that creates it and its
> descendants (*i.e.*, by related processes).

A `read()`, with the file descriptor `pipefd[0]`, has the following properties:

* A read is destructive.
* If the pipe is *not* empty, then it returns immediately.
    * It obtains the minimum between the number of bytes requested and the
        number of bytes currently available in the pipe.
* If the pipe is empty and there is at least a process that has the pipe open
    for writing, then it blocks until something is written on the pipe.
* If the pipe is empty and no process has the pipe open for writing, then it
    returns 0 (end-of-file).

In other words, reading `n` bytes from a pipe containing `p` bytes has the
following semantics:

* **`p = 0`, write end open** : block.
* **`p = 0`, write end closed** : return 0 (EOF).
* **`p < n`** : read `p` bytes.

As explained previously, when a pipeline such as `ls | wc -l` is entered on the
command line, the shell creates two processes executing `ls` and `wc`
respectively.  The communication between these two programs is handled by a
pipe.

```C
/*                                Pipe
 *   ____________       ________________________       ___________
 *  |            |     |       byte stream      |     |           |
 *  | ls (stdout)| --> | unidirectional channel | --> |(stdin) wc |
 *  |____________|     |________________________|     |___________|
 */
```

A write to a pipe is guaranteed to be atomic as long as no more than `PIPE_BUF`
bytes are written at a time.^[For example, on Linux, the value of `PIPE_BUF` is
equal to 4096 bytes. The pipe capacity is 65536 bytes and can be changed by a
call to `fcntl()`with a size between the system page size and the value in
`/proc/sys/fs/pipe-max-size`.] This means that data written by multiple process
on the same pipe won't be intermingled if they write no more than `PIPE_BUF`
bytes.

A `write()` of `n` bytes with the file descriptor `pipefd[1]` has the following
properties:

* If there is no sufficient space available in the pipe, then the call blocks
    until the `n` bytes are written on the pipe.
    * If `n <= PIPE_BUF`, then the `n` bytes are written atomically on the pipe.
    * If the call is interrupted by a signal handler, then the call unblocks and
        returns a count of the number of bytes successfully transferred.
* If every process has closed the read file descriptor `pipefd[2]`, then the 
    kernel sends the `SIGPIPE` signal. If the process catch, or ignore this
    signal, then the call to `write()` fails and set `errno` to `EPIPE`.

In other words, writing `n` bytes to a pipe has the following semantics:

* **`n <= PIPE_BUF`, read end open** : atomically write `n` bytes; if there is
    no sufficient space, then block until sufficient space is available on the
    pipe.
* **`n >= PIPE_BUF`, read end open** : write `n` bytes; if there is
    no sufficient space, then block until sufficient space is available on the
    pipe; the kernel may transfer the data in multiple smaller pieces, and
    therefore the data may be interleaved.
* **read end closed** : `SIGPIPE` + `EPIPE`.

In most cases, a single process is allowed to write to the pipe, and another
related process is allowed to read from the pipe. The following listing presents
the main steps for creating a pipe to transfer data from a parent process to a
child process. This listing is available in `pipe_setup.c`.

```C
int filedes[2];

if (pipe(filedes) == -1)
    errExit("pipe");                 /* Create the pipe */

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
```

> Exercise #
>
> Write a program that creates a child process. The parent process ask the user
> for two integers. Then it communicates these two integers via a pipe to the
> child process. The child process computes the sum, and write the result in a
> second pipe. Finally, the parent process displays the result.

\newpage

> Exercise #
>
> Write a program that creates a pipe and two child processes. The first child binds
> its standard output to the write end of the pipe and then executes `ls`. The
> second child binds its standard input to the read en of the pipe and then
> executes `wc -l`. This procedure is similar to the one performed by the shell to
> execute `ls | wc -l`. For this purpose, use the `dup2()` system call.

It is important to close unused pipe file descriptors for the following reasons:

* To ensure that a process doesn't exhaust its limited set of file descriptors.
* So that the reading process is able to see an end-of-file when the writing
    process has completed its output.
    * For this purpose, the reading process also has to close its write end of
        the pipe.
* So that the writing process is notified if it tries to write to a process for
    which no process has open the read file descriptor.
    * For this purpose, the writing process has to close the read end.
      Otherwise, the process might block indefinitely when it calls `write()`
      on a full pipe.

> Exercise #
>
> Write a program that creates a *synchronized* process fan composed of $n$ processes.
> In other words, a parent process and $n-1$ child processes. Processes have to wait
> until all have eben created before echoing their messages to standard output. Use a
> single pipe to perform this synchronization.
> The use of `r_read` and `r_write` is required. Why ?

```C
#include <errno.h>
#include <unistd.h>

ssize_t r_write(int fd, void *buf, size_t size) {
   char *bufp;
   size_t bytestowrite;
   ssize_t byteswritten;
   size_t totalbytes;

   for (bufp = buf, bytestowrite = size, totalbytes = 0;
        bytestowrite > 0;
        bufp += byteswritten, bytestowrite -= byteswritten) {
      byteswritten = write(fd, bufp, bytestowrite);
      if ((byteswritten == -1) && (errno != EINTR))
         return -1;
      if (byteswritten == -1)
         byteswritten = 0;
      totalbytes += byteswritten;
   }
   return totalbytes;
}

ssize_t r_read(int fd, void *buf, size_t size) {
   ssize_t retval;

   while (retval = read(fd, buf, size), retval == -1 && errno == EINTR) ;
   return retval;
}    
```