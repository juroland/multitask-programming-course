---
title: 'UNIX Processes : part 2'
---

# Process user IDs

Each process has two user IDs associated to it. The **real user ID** (UID) and
the **effective user ID** (EUID). The UID identifies the user who created it
(*i.e.*, equal to the UID of the parent process). The EUID determines the
*permissions* of the process (*i.e.*, what resources and files the process is
allowed to access). In the same way, the process has two group IDs associated to
it. The **real group ID** (GID) and the **effective group ID** (EGID). 

By default, real and effective IDs have the same value. These values are
different if the *set uid* mode, and the *set gid* mode are enabled. If the
**set uid** mode is enabled (`chmod u+s`), then the file will run under the id
of the owner of the file. If the **set gid** mode is enabled (`chmod g+s`), then
the file will run under the id of the group owning the file. In other words, if
the *set uid* (*set gid*) mode is enabled, then the effective user (group) ID is
equal to the owner of the executable file.

> Exercise #
>
> Modify the previous source code so that the `uid` and `euid` are
> displayed for both processes with system calls `getuid()` and `geteuid()`.

> Exercise #
>
> Change the binary executable owner to `root` with the command
> `chown root` followed by the file name. What is the difference when the *set
> uid bit* is enabled ?

Two other IDs are also associated with each process: the *saved set-user-ID* and
the *saved set-group-ID*. These values are copied from the effective IDs when
the program is executed. In practice, this allows programs with the set uid mode
enabled to temporarily increase or lower privileges by switching the value of
EUID between the values of the user ID and the saved set-user-ID. By this means,
we are able to apply the *least privilege* principle that states : "Every
program and every user of the system should operate using the least set of
privileges necessary to complete the job." (Saltzer, J.H.; Schroeder, M.D., "The
protection of information in computer systems", in Proceedings of the IEEE ,
vol.63, no.9, pp.1278-1308, Sept. 1975).

A common secure programming practice, is to drop privileges on program startup
and to require them as needed. In this way, we limit the damages that would
occur from accidents, errors, and attacks. As explained in a later section, it
is also important to drop privileges permanently when executing another program.
For example, we can temporarily drop and then require privileges as follows.

```C
uid_t orig_euid;

orig_euid = geteuid();
if (seteuid(getuid()) == -1) /* drop privileges */
    /* error */

/* do unprivileged work */

if (seteuid(orig_euid) == -1) /* reacquire privileges */
    /* error */

/* do privileged workd */
```

This code uses two calls to `seteuid()` that sets the effective user ID of the
calling process. This function takes as parameter either the real user ID, the
effective user ID or the saved set-user-ID of the calling process. Otherwise,
the function returns `-1` and `errno` is set to `EPERM`. Only a privileged
process is able to set its effective user ID to another value.

> Exercise #
>
> Write a program that illustrates this programming practice. For example, a
> set uid program with owner equal to root is able to open of a protected file
> before a call to `seteuid(getuid())` and then unable to open the same file.

When a child process is created, it receives a copy of all the parent's file
descriptors. Therefore, they point to the same entry in the open file table
that contains, among other things, the current file offset. Consequently, if
this offset is modified by the child process, then it is also modified for the
parent process.

> Exercise #
>
> Write a program that illustrates that file descriptors are shared by child and
> parent processes after a `fork()`. For example, the program open a file in
> write only (without the `O_APPEND` flag enabled), then it calls `fork()`. The
> child process write a finite sequence of digits in the file and the parent
> process write a finite sequence of letters. Use `sleep()` in order to
> interlace the execution of the two processes. What happens if the file is
> opened after the call to `fork()` ?


The `wait()` system call suspends the execution of the calling process until one
of its children terminates. This function has an output parameter (`int
*status`) that stores the termination status of that child once the call is
completed. This function also returns the `PID` of the terminated child, or `-1`
on error. For example the following loop is used to wait for all children of the
calling process to terminate.

```C
#include <sys/wait.h>
/* pid_t wait(int *status); */

int status;
pid_t childPid;
while ((childPid = wait(&status)) != -1)
    printf("Child process : %ld\n", (long) childPid);

if (errno != ECHILD) /* ECHILD : there is no children */
    printf("An unexpected error...interrupt, or invalid argument\n");
```

> Exercise #
>
> With the help of the `wait()` system call, transform the program of the
> previous exercise so that all the digits appear before all the letters in
> the output file. Do not use the `O_APPEND` flag when opening the output file.

The value of `status` can be inspected with macros such as `WIFEXITED(status)`
which returns true if the child process terminated normally. Other macros can be
found in man pages (`man 2 wait`).

The `waitpid()` system call allows to wait for a particular child whose process
`ID` is equal to a given `pid`:

```C
pid_t waitpid(pid_t pid, int *status, int options);
```

Moreover, instead of waiting for a process to terminate, this system calls
suspends execution of the calling process until the child has change state.
That is to say, the child terminated; the child was stopped by a signal; or the
child was resumed by a signal.

The value of `options` can include zero or more of the following options:

* `WNOHANG` : Return immediately if no child specified by `pid` has yet changed 
state
* `WUNTRACED` : Also return when a child process is `stopped` by a signal
* `WCONTINUED` : Also return when a child has been resumed by delivery of a 
`SIGCONT` signal

> Exercise #
>
> Write a program that illustrates the use of `WUNTRACED` and
> `WCONTINUED`. Use the command `kill -SIGSTOP` to stop the child process and
> `kill -SIGCONT` to continue the execution of the child process.

The following program creates a fan of `n` processes (a parent process and `n-1` child processes),
where `n` is passed as a command-line argument.

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

pid_t r_wait(int *stat_loc) {
   pid_t retval;
   while (((retval = wait(stat_loc)) == -1) && (errno == EINTR)) ;
   return retval;
}

int main(int argc, char *argv[]) {
   pid_t childpid;
   int i, n;

   if (argc != 2) {
      fprintf(stderr, "Usage: %s n\n", argv[0]);
      return 1;
   }
   n = atoi(argv[1]);
   for (i = 1; i < n; i++)
      if ((childpid = fork()) <= 0)
         break;

   while(r_wait(NULL) > 0) ;    /* wait for all of your children */ 
   fprintf(stderr, "i:%d  process ID:%ld  parent ID:%ld  child ID:%ld\n",
           i, (long)getpid(), (long)getppid(), (long)childpid);
   return 0;
}
```

> Exercise #
>
> Explain what happens when you replace the test `(childpid = fork()) <= 0` of
> the previous program with `(childpid = fork()) > 0`. 

> Exercise #
>
> Explain what happens when you replace the test `(childpid = fork()) <= 0` of
> the previous program with `(childpid = fork()) == -1`.

> Exercise #
>
> How can you modify the program so that you can use `pstree` (for example,
> `pstree 4` shows the running processes rooted at the process with PID equal
> to 4) to see the processes that are created ?

> Exercise #
>
> What happens if you interchange the `while` loop and `fprintf` statements ?

> Exercise #
>
> What happens if you replace the `while` loop with the statement `wait(NULL);`
> ?

## Summary

Among others, the following attributes are **inherited** after a call to `fork()`:

* Stack segment
* Data segment
* Heap segment
* Open file descriptors
* Process group ID (see later)
* Session ID
* Read IDs
* Effective and saved set IDs
* The calling thread (see later)
* Signal dispositions
* Mutexes (see later)

The following attributes are **shared** after a call to `fork()`:

* Text segment
* File offsets
* Open file status flags
* Named semaphores (see later)

# Process termination
A process can terminate either *normally* using the `_exit()` system call or
*abnormally* when terminated by a signal.

```C
#include <unistd.h>
void _exit(int status);
```
The `status` argument defines the termination status of the process. This value
is obtained by the parent process when it calls `wait()`. Although this
variable is defined as `int`, only the least significant 8 bits are made
available to the parent process. By convention, a termination status of 0 indicates
a successful termination.

It's recommended to use the `exit()` library function (defined as part of the
C standard library), instead of calling `_exit()` directly.

```C
#include <stdlib.h>
void exit(int status);
```

Indeed, further actions are performed by `exit()`:

* Exit handlers are called (see `atexit()` and `on_exit()` for registering exit handlers)
* The `stdio` stream buffers are flushed
* The `_exit()` system call is invoked with the corresponding `status` value

The `return n` statement at the end of the `main()` function is equivalent to
calling `exit(n)` (unless variables local to `main()` are accessed during
exit which results in undefined behavior). Starting from the C99 standard, if
the main function does not contain a `return statement`, then by default it
returns `0`.

> Exercise #
>
> If a child process makes the call `exit(-1)`, what exit status will be seen
> by the parent ?

The I/O functions of the C standard library such as `printf()`, `fread()`, or
`fwrite()`, use a *buffer cache* for improving performance by reducing the
number of system calls that are costly operations. For example, `fprintf(fp,
"abcd")` writes the string `"abcd"` to a file described by the file pointer
`fp`. This string is not directly written on disk, but instead in a buffer.
After subsequent call to `fprintf()`, if the buffer is full, then its content is
eventually written to the file with a call to `write()` with the corresponding
file descriptor. We are able to force a write of the *buffer cache* for a given
output stream with `fflush()`. For example, `fflush(stdio)` to force a write to
the standard output. 

When system calls such as `read()` and `write()` are invoked, the kernel also
use a buffer cache in order to reduce the number of disk transfer. Therefore,
these system calls are not synchronized with the disk operation.

It is worth noting that terminal I/O are line buffered. On output, the call to
`write()` occurs either when the buffer is full, or when a new line symbol is
encountered.

> Exercise #
>
> What output would be generated by the following program ?

```C
        #include <stdio.h>
        #include <unistd.h>

        int main(void) {
            printf("This is my output.");
            fork();
            return 0;
        }
```

> Exercise #
>
> What output would be generated by the following program ?

```C
        #include <stdio.h>
        #include <unistd.h>

        int main(void) {
            printf("This is my output.\n");
            fork();
            return 0;
        }
```

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
* Effective and saved set IDs (see previous section about `set uid bit`)
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

For security purpose, a set-user-ID program should reset its user IDs to the
same value as the real user ID. This result is achieved by calling
`setuid(getuid())` before a call to an `exec()` function. Moreover, as explained
previously, the saved set-user-ID is replaced by the value of the effective user
ID after a successful call to an `exec()` function.

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

# Process state
A process belongs to one of the following states:

* `TASK_RUNING` : The process is either executing on the CPU or waiting to be
executed.
* `TASK_INTERRUPTIBLE` : The process is suspended (sleeping) until some event
occur. If a signal is sent to a process in this state, then the process is woken
up.
* `TASK_UNINTERRUPTIBLE` : Similar to `TASK_INTERRUPTIBLE`, except that the
process in this state cannot be interrupted by a signal. For example, when a
process is waiting for the completion of a disk IO.
* `TASK_STOPPED` : The process is forbidden to run. A process is stopped with the signal `SIGSTOP` or `SIGTSTP` and restarted with `SIGCONT`.
* `TASK_TRACED` : The process execution has been stopped by a debugger.
* `EXIT_ZOMBIE` : The process has finished its execution. All the resources held
by the process are released back to the system, but the kernel keeps
information, for the parent process, about the process termination. This
*zombie* is finally removed when the parent process is informed of this
information, or when the parent terminates.
* `EXIT_DEAD` : The final state after `EXIT_ZOMBIE`.

> Exercise #
>
> Write a program that creates a zombie process. Display its
> state stored in `/proc/PID/status`.

Long lived processes, such as network servers and shells create a large number of
child processes during their lifetime. This may lead to an accumulation of
`zombie` that will eventually fill the process table preventing the creation of
new processes. In this case, the parent process can call the `wait()` or
`waitpid()` system calls. However, this approach is often inconvenient, because
the call to `wait()` is blocking and a call to a non-blocking version of
`waitpid()` leads to waste CPU time and adds complexity. A solution is to handle
the `SIGCHLD` signal. This signal is sent to a parent process whenever one of
its child process terminates. By default, this signal is ignored.

Hence, a solution could be to call `wait()`, inside the signal handler, to reap
the zombie processes. However, signals are not queued and are temporarily blocked
when the signal handler is called. Consequently, the handler might fail to reap
some zombie children. 

The proposed solution is to loop inside the `SIGCHLD` handler, until there are
no more dead child to be reaped as follows:

```C
while (waitpid(-1, NULL, WNOHANG) > 0)
    continue;
```

> Exercise #
>
> Write a program that generates 100 zombie processes and then wait for some
> input before returning. Display these zombie processes with `pstree`. Add a
> `SIGCHLD` handler to the parent process so that all the zombie processes are
> removed.  Check that all these zombie processes are removed before the end of
> the parent process execution with the `pstree` command. You have to add
> several calls to `sleep()` or `pause()` when necessary.
