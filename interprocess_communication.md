# Interprocess communication

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
> Explain the reason why a pipe can be used by the process that create it and its
> descendant (*i.e.*, by related processes).

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
