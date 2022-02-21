---
title: '**Multi-Task Programming**'
subtitle: '*in the GNU/Linux Environment*'
author:
- name: Julien Roland
  email: juroland@gmai.com
date: '2015 -- 2016'
---

# Processes
A **process** is an instance of an executing program. It is characterised by one
or more sequences of instructions executed in the same *private address space*.
This means that one process is not able to read or modify the memory of another
process. However, as explained later in more detail, several processes can share
memory by executing the same program or with shared memory techniques.

The execution of a process depends on the value of the instruction pointer. This
value is incremented by the CPU to point to the next instruction and modified by
instructions such as `jmp`. This sequence of instruction is called a
*thread of execution*.


## Credentials

Each process has a process ID (`pid`), a positive integer that uniquely
identifies the process on the system. The `getpid()` system call returns the
process ID of a calling process.

```C
#include <sys/types.h>
#include <unistd.h>

pid_t getpid(void);
```

The return type is `pid_t`, defined an unsigned integer type. Its a common
practice to cast this value to `long` for printing, because there is no guarantee
that a `pid_t` will fit in a `int`.

> Exercise #
>
> Write a C program that displays the process ID. See `man getpid` for more
> details about this system call.

The kernel provides information about each process on the system in the `/proc`
file system. For each process id PID, there exists a directory named `/proc/PID`
that contains various files containing information about that process (see, `man
proc` for more details).

> Exercise #
>
> Create a program that displays its `pid` and then waits for some input before
> returning. Execute the program with three arguments. For example, `./a.out one
> two three`. In a second terminal, display the content of `/proc/PID/cmdline`
> (with `od -c /proc/PID/cmdline`), and list the file attributes of
> `/proc/PID/exe`, and `/proc/PID/cwd` with `ls -l`. Explain the result.

## Memory layout

The memory allocated to each process is divided into the following parts, known
as segments:

* *Text* : the instructions of the program (read-only)
* *Initialized data* : the static (and global) variables that are explicitly
  initialized
* *Uninitialized data* : the static (and global) variables that are not
  explicitly initialized (also known as the `bss` segment)
* *Stack* : a piece of memory that grows and shrinks as functions are called. A
  new stack frame is pushed on the stack when a function is called and removed
  from the stack when the function returns. A stack frame contains local
  variables (known as automatic variables), arguments, return values, and CPU
  registers such as the program counter to be restored when the function
  returns. It is worth noting that automatic variables and function arguments
  are not always stored on the stack when optimized by the compiler.  Instead,
  they are often stored in CPU registers. This piece of memory is also called
  the *user stack*, a second per-process stack called the *kernel stack* is also
  used when executing system calls.
* *Heap* : an area used by programs to dynamically allocate extra memory

The command `size` displays the size of these segments for a given object file.

> Exercise #
>
> Compile the following two source files as object files (`gcc -c`) and compare,
> and explain, the output of the `size` command.

```C
        int main(int argc, char **argv) {
            return 0;
        }
        
        int main(int argc, char **argv) {
            char x;
            char y = '7';
            static char z = '8'; 
            return 0;
        }
```

> Exercise #
>
> Use `ls -l` to compare the size of the executable modules for the following
> two programs. Explain the result.

```C
        /* largearray.c */
        int myarray[50000];
        int main(int argc, char **argv) {
            myarray[0] = 3;
            return 0;
        }
        
        /* largearrayinit.c */
        int myarray[50000] = {1, 2, 3, 4};
        int main(int argc, char **argv) {
            myarray[0] = 3;
            return 0;
        }
```

The memory layout of a given process is displayed with the command `cat
/proc/PID/maps` where PID is the process id of the considered process. Each line
describes an address space with the corresponding permissions (r = read, w =
write, x = execute, s = shared) and pathname.

> Exercise #
>
> Create a program that displays its `pid`, allocates an array in the heap,
> declares and initialized a global array, declares a global array (not
> initialized), and then waits for some input before returning. Display its
> memory layout and identify its text, data, stack, and heap segments. Further
> segments are also displayed. A read-only segment that contains data such as
> constant strings. Several segments refer to shared libraries if the source
> code is not compiled with the `-static` option. Other segments such as `vvar`,
> `vdso` and `vsyscall` are used on Linux for improving the performance of
> frequent system calls (see `man vdso` for more details).

### A review of key concepts of virtual memory management

In the virtual memory scheme, the memory used by each program is divided into
equally-sized pieces, called pages, and the RAM is divided into page frames of
the same size. The mapping between pages and page frames is defined in a page
table. A different page table is maintained by the kernel for each process. This
table is then used by a paging hardware to translate virtual addresses into
physical addresses. 

Only a subset of page reside in the physical memory. This set of pages is called
the *resident set*. Remaining pages are stored in a reserved area of the disk
space, called the *swap space*.  If the process refers to virtual address in a
page that is not member of the resident set, then the paging hardware raises an
interrupt called a *page fault*. This interruption is handled by the kernel in
order to load the corresponding page in memory.

## System calls

A process is able to send requests to the kernel to perform some actions. For
example, to write some data in a file or to create a new process. These requests
called *system calls* changes the processor state from *user mode* to *kernel
mode* and give the control to the kernel. Once this action is completed, the
kernel gives back the control to the calling process.

Each system call is wrapped by a C library function. This function copies all
arguments to appropriate locations, stores the system call number in a CPU
register, and raises a particular software interrupt to give control to the
kernel. In response, the kernel invokes the appropriate system call service
routine that is identified by the number stored in the CPU register.

If there is an error during the system call, then the routine sets the global
integer variable `errno` to a positive value that identifies what went wrong. A
list of error names defined on Linux is available in `man errno`. If the call is
successful, then the corresponding C library function returns 0.  Otherwise, it
returns -1. The routine `void perror(const char *s)`, defined in `errno.h`,
produces a message on the standard error output the string `s` followed by a
message corresponding to the value of `errno`. In what follows, we sill make no
difference between a system call and its corresponding C library function.

Almost every system call indicates whether the call succeeded or failed by
returning a status value. Usually, a system call returns `-1` in order to
indicate that an error occurred and sets the global integer variable `errno` to
a positive value that specifies the type of error. An error message can be
obtained, based on this value, by calling the functions `perror()` or
`strerror()`.

## File I/O

A process refers to an open file with a file descriptor, *i.e.*, a non-negative
integer. This value refers to a table of file descriptors maintained for each
process by the kernel. Each entry in this table contains the following
information:

* A set of flags (in practice there is only the *close on exec* flag, described
  in a later section)
* A reference to the open file descriptor in a system wide open file descriptor
  table called the *open file table* that contains further details on the open
  files (file offset, status flag, i-node pointer)

When a process is started by the shell it starts with three standards
file descriptors :

* 0 : standard input (`STDIN_FILENO` as defined in `unistd.h`)
* 1 : standard output (`STDOUT_FILENO`)
* 2 : standard error (`STDERR_FILENO`)

You can use various system calls such as `read()` and `write()` to perform I/O
operations. Let us consider the following function that returns the next file
read from a file.

```C
#include <errno.h>
#include <unistd.h>

int readline(int fd, char *buf, int nbytes) {
   int numread = 0;
   int returnval;

   while (numread < nbytes - 1) {
      returnval = read(fd, buf + numread, 1);
      /* If the read() syscall was interrupted by a signal */
      if ((returnval == -1) && (errno == EINTR))
         continue;
      /* A value of returnval equal to 0 indicates end of file */
      if ( (returnval == 0) && (numread == 0) )
         return 0;
      if (returnval == 0)
         break;
      if (returnval == -1)
         return -1;
      numread++;  
      if (buf[numread-1] == '\n') {
         /* make this buffer a null-terminated string */
         buf[numread] = '\0';
         return numread; 
      }  
   }    
   /* set errno to "invalid argument" */
   errno = EINVAL;
   return -1;
}
```

> Exercise #
>
> Use the function `readline()` to write a program named `echo.c` that writes to
> the standard output everything read from standard input. The program
> terminates when it reads an end of file (generated with `Control-d`).

A given file is associated to a new file descriptor with the system call
`open()` (see `man 2 open`). For example, `myfd = open("/home/julien/file.txt",
O_RDONLY | O_APPEND)` creates an entry in the process file descriptor table
identified by `myfd`. This entry points to a new entry in the open file table
combining an access mode equal to `O_RDONLY` (read-only) and the flag `O_APPEND`
(the file offset is moved to the end of the file). This second entry points to
the in-memory i-node table. Each i-node (short for index node) describes a
particular file residing in the file system. This in-memory table contains an
entry for each *active* file in the system. All the i-nodes are stored on disk
and are loaded in memory as soon as the corresponding file is opened.

> Exercise #
>
> Based on the program skeleton `copy.c`, write a program that copies the
> content of an existing file into a new file. The name of these files should be
> provided as arguments of this program. You have to handle all the errors that
> may occur.

When a process terminates, all of its file descriptors are automatically closed
in order to release its associated kernel resources. There exists a maximal
number of file descriptors for each process. In order to avoid running out of
file descriptors, it is important to close explicitly file descriptors as soon
as they are unneeded with the system call `close()` (see `man 2 close` for more
details).

```C
#include <unistd.h>
int close(int fd);
```

> Exercise #
>
> Modify the previous program so that all the file descriptors are explicitly
> closed.

## Signals
A **signal** is a software notification to a process that an event has occurred.
They are sent by the kernel (*for example, caused by an hardware exception, or
when the user typed special characters such as interrupt with `Control-C` or
suspend with `Control-Z`*) or other processes for synchronization or simple
communication purpose.

The `kill()` system call sends a given signal to another process. Its prototype
is the following (see `man 2 kill`). 

```C
#include <signal.h>
int kill(pid_t pid, int sig);
```

For example, `kill(8907, SIGTERM)` sends the *termination* signal to the process
with `pid` equal to 8907. This signal is a way to *request* a process to
terminate. This signal can be blocked, handled, and ignored. A list of other
signals is available in `man 7 signal`. A well designed application will
terminate gracefully when the signal `SIGTERM` is received. That is to say, a
handler for cleaning up temporary files and releasing other resources. It is
worth noting that a process is not always able to send a signal to any process.
The rules are available in the `man 2 kill`. If the process doesn't have
permission, then `kill()` returns -1 and `errno` is set to `EPERM`.

Once a signal is delivered to the target process, one of the following default
action is performed (depends on the signal):

* The signal is ignored
* The process is terminated (killed)
* The process is suspended
* The process is resumed

A program is able to change the default action that occurs when a signal is
delivered with the system call `signal()`. The function prototype is the
following.

```C
#include <signal.h>
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);
```

Where `sighangler_t` defines a type of pointer to a function passing an `int`
and returning `void`. The first parameter, `signum`, identifies the signal
whose action is to be changed. The second parameter, `handler`, is a pointer to
the function that has to be called when the signal, `signum`, is delivered. The
integer argument passed to the handler, by the kernel, identifies the signal
that caused the invocation, because the same handler might be configured to
catch different types of signals (with multiple call to `signal()`).

Several signals are sent by keyboard shortcuts, such as the `SIGINT` with
`Control-c` (equivalent to `SIGTERM`, but sent to the foreground process group,
see later), `SIGTSTP` with `Control-z`, and `SIGQUIT` with `Control-\ `
(equivalent to `SIGINT`, but generates a core dump as well for debugging
purposes).

> Exercise #
>
> Write a program that prints the value of a counter every 3 seconds, increasing
> as it prints. Create a signal handler that prints a message when the signal
> `SIGINT` is delivered. Use the `SIGQUIT` signal to this process to terminate
> the program. Explain the result.

It is worth noting that it is impossible to change the default actions for
`SIGKILL` and `SIGSTOP` (the process is stopped, *i.e.*, forbidden to run until
being restarted by `SIGCONT`).

## Process creation
A process can create new processes using the **fork()** system call. In other
words, a process is *created* with the *fork()* system call. The process that
calls *fork()* is the **parent process**, and the new process is the **child
process**. Each process is identified by a **pid** (process ID) and identifies
its parent process with a **ppid** (parent process ID). This induces a tree-like
structure starting from the **init** process with a `pid` equal to 1. The new
child process is an almost exact duplicate of the parent. The child obtains
*copies* of the parent's stack, data, heap, and text segments (for better
performance, the kernel implement a *copy-on-write* strategy so that segments
are duplicated only if modified).

After the system call `fork()` has completed its work, two processes exists.
Each process continues form the point where `fork()` returns. What distinguishes
the two processes is the value returned from this function. For the parent, this
value is equal to the child process's PID, whereas for the child, this value is
equal to 0. In case of failure, the function returns -1 as usual.

Try and understand the following program :

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    pid_t pid = fork();
    if (pid == 0) {
        printf("Child process (PID = %d, parent PID = %d)\n", getpid(), getppid());
    } else if (pid != -1) { 
        printf("Parent process (PID = %d, child PID = %d)\n", getpid(), pid);
    } else {
        perror("Failed call to fork");
        return 1;
    }

    return 0;
}
```

> Exercise #
>
> Write a program that illustrates that the child process gets its
> own copy of the stack and data segments. For this purpose, declare a variable
> that is allocated in the data segment, and a second variable allocated in the
> stack segment. For this purpose, you may have to use `sleep()` declared in
> `unistd.h` (see `man 3 sleep`).

> Exercise #
>
> Display the *ppid* of a process before and after its parent process complete its
> execution. What do you conclude ?

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

The following program creates a fan of `n` processes (a parent process and `n-1` child processes), where `n` is passed as a command-line argument.

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

### Summary

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

The following attributes are **not** inherited after a call to an `exec()` function:

* Process IDs (PID, PPID)

## Process termination
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
            printf("This is my output.")
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
            printf("This is my output.\n")
            fork();
            return 0;
        }
```

## Program execution

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

For example, the following program creates a child process to run `/bin/ls` with `argv[0]` equal to `ls` and `argv[1]` equal to `-l`. This list of arguments ends with `NULL`, because `argv` is defined as a `NULL` terminated array.

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

## Process state
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

## Groups, sessions, and terminals

Process groups and sessions are abstractions that provide an hierarchical
relationship between processes. A **process group** (often used as a synonym to
the term *job*) is a collection of one or more processes. Each process group is
identified with a *group process id* (PGID) equal to the PID of the process
leader. A **session** is a collection of process groups. Each session is
identified by a *session id* (SID) equal to the PID of the process that creates
the session.

For example, each shell creates a session composed of several process groups.
One of the process group is the *foreground process group* and others are the
*background process groups*. Each command or pipeline of commands started from
the shell leads to the creation of one or more processes, and a new process
group that contains all of these processes. For example, the pipeline `ls | wc
-l` creates two processes `ls` and `wc -l` in a process group. The symbol `|`,
called a **pipe**, indicates to the shell to redirect the standard output of
the first process with the standard input of the second one. Pipes are explained
with details in a later section. In the shell, a program is executed either as a
*foreground process* (by default), or as a *background process*.

Once the shell program is executed, there is a single process group, the
foreground process group, that contains the shell process. When a command is
executed, the shell uses a `wait()` function to wait for the completion of the
corresponding process group. This process group becomes the foreground process
group and the process group that contains the shell becomes a background
process groups. Once the command execution is over, the shell regains control of
the terminal (*i.e.*, put itself back in the foreground process) and displays
its prompt (`$` in the following examples). For example:

```
$ date
Mon Dec  7 11:18:33 CET 2015
$ sleep 10; date
Mon Dec  7 11:18:43 CET 2015
$
```

When a background process group is executed (by adding an ampersand (`&`) at the
end of the command), the shell's process group remains the foreground process
group, displays immediately its prompt, and continue to wait for commands from
the terminal. For example:

```
$ date
Mon Dec  7 11:20:22 CET 2015
$ (sleep 10; date)&
$ date
Mon Dec  7 11:20:24 CET 2015
$ Mon Dec  7 11:20:32 CET 2015
```

If you try to execute the same set of commands, the shell should output the PGID
of the background process group, `(sleep 10; date)&`, when the process group is
executed and done.

Each new process inherits its parent's process group ID. This value is obtained
using `getpgrp()`.

```C
#include <unistd.h>

pid_t getpgrp(void);
```

> Exercise #
>
> Write a program that displays its PID and PGID, then creates a child process that
> displays its PID and PGID and sleeps for 5 seconds. The parent process has to
> wait for the completion of its child process. Execute this program called
> `pgid` with the following commands. Explain the results.
>
> 1. `./pgid; ./pgid`
> 1. `(./pgid; ./pgid)`
> 1. `./pgid&; ./pgid&`
> 1. `(./pgid; ./pgid)&`
> 1. `(./pgid&; ./pgid&)&`


The system call `setpgid()` allows to change the process group id of a given
process specified by `pid` to `pgid`. That is to say, to create a new process
group (for example, with `setpgid(0,0)`), or to move the process to an existing process
group.

```C
#include <unistd.h>

int setpgid(pid_t pid, pid_t pgid);
``` 

The following restrictions are applied to the value of `pid` and `gpid` :

* The value of `pid` has to be equal to the process id of the calling
    process, or to the process id of one of it's children.
* If the value of `pid` refers to one of its children, then a call to
    `setgpid()` returns an error (`errno` is equal to `EACCES`) if the child has
    performed an `exec()`.  Indeed, this could lead to an unpredictable
    behaviour if this value is modified by both programs after the call to
    `exec()`, because the result would depend on the scheduling of these two
    processes.
* The calling process, the process identified by `pid`, and the process group
    identified by `pgid`, must all be part of the same session.
* The process identified by `pid` cannot be a session leader.

> Exercise #
>
> Compile and execute the following program. Wait several seconds, and then use
> `Control-C` to send a signal to the foreground process group. What do you
> conclude ? This source code is available as `setpgid.c`.

```C
        #include <unistd.h>
        #include <stdio.h>
        #include <sys/types.h>
        #include <sys/stat.h>
        #include <sys/wait.h>
        #include <fcntl.h>
        
        int main(void)
        {
            printf("pid : %ld\n", (long)getpid());
        
            pid_t pid;
            if ((pid = fork()) == 0) {
                printf("Child : pid : %ld ; pgid : %ld\n", (long)getpid(),
                        (long)getpgid(0));
        
                if (setpgid(0,0) < 0)
                    perror("child 1 : setpgid");
        
                pause();
            } else {
                if (fork() == 0) {
                    while (1) {
                        printf("Child 2 : pid : %ld ; pgid : %ld\n", (long)getpid(),
                                (long)getpgid(0));
                        sleep(2);
                    }
                } else {
                    printf("Parent : pid : %ld ; pgid : %ld\n", (long)getpid(),
                            (long)getpgid(0));
        
                    sleep(2);
        
                    if (setpgid(0,pid) < 0)
                        perror("child 2 : setpgid");
        
                    while (1) {
                        printf("Parent : pid : %ld ; pgid : %ld\n", (long)getpid(),
                                (long)getpgid(0));
                        sleep(2);
                    }
                }
            }
        
            return 0;
        }
```

The shell uses `setpgid()`, when it executes a command or a pipeline (*i.e.*, a
job), to place all the related processes in a single process group. For this
purpose, the shell might follow the following code structure:

```C
pid_t childPid;
pid_t pipelinePgid; /* PGID to which processes
                       in a pipeline are to be assigned */

/* ... */

childPid = fork();
switch (childPid) {
    case -1: /* fork() failed */
        /* Handle error */

    case 0: /* Child */
        /* Move the calling process in the process group
        identified by pipelinePgid */
        if (setpgid(0, pipelinePgid) == -1)
            /* Handle error */
        /* Child carries on to exec the required program */

    default: /* Parent (shell) */
        /* Move the child process in the process group
        identified by pipelinePgid */
        if (setpgid(childPid, pipelinePgid) == -1 && errno != EACCES)
            /* Handle error */
        /* Parent carries on to do other things */
}
```

Let us note that in the case of a pipeline, the value of `pipelinePgid`, should
be equal to the PID of the first process in the pipeline.

As you can see, in this code, both parent and child processes call the
`setpgid()`, because the scheduling of parent and child processes are not known
in advance and the system should be able to send signals to the process group as
soon as possible. In other words, we should not have to wait for the execution
of the child process to be able to send a signal to all the processes in this
group. A signal `sig` is sent to a process group identified by `pgid` with a
call to `kill(-pgid, sig)`.

> Exercise #
>
> What is the meaning of `setpgid(0, 0)` (seen `man setpgid`)? Give an
> equivalent call to `setpgid()`, with `pid` different than zero.


The session ID of a given process is returned by a call to `getsid()` and a call
to `setsid()` creates a new session.

```C
#include <unistd.h>

pid_t getsid(pid_t pid);
pid_t setsid(void);
```

If the calling process is not already a process group leader, then `setsid()`
creates a new session with a session ID equal to its process ID. Otherwise, it
returns `-1` and `errno` is set to `EPERM`. Furthermore, the calling process
becomes the group leader of a newly created process group within this new
session. This justify the reason why a leader process cannot call `setsid()`,
because this would lead to two different process groups with the same ID.

\begin{figure}
\centering
\includegraphics[scale=0.06]{img/DEC_VT100_terminal.jpg}
\caption{A DEC VT100 terminal that communicates with a UNIX system via a serial
line.}
\end{figure}

A **terminal** (or a terminal emulator such as `xterm` or `gnome-terminal`) is a
device that process its input line by line. Each line is terminated by
a newline character that is generated when the user presses the *Enter* key. A
driver is associated with the terminal. This terminal driver interprets special
characters such as the interrupt character (`Control-C`) to send a signal
`SIGINT` to the foreground process group, or the quit character (`Control-\ `)
to send a signal `SIGQUIT` to send a `SIGQUIT` signal to the foreground process
group.

> Exercise #
>
> Write a program that illustrates that all the processes in the foreground
> process group receives the `SIGINT` signal generated by `Control-C`, and
> conversely that processes in the background process group do not receive this
> signal.

The terminal driver manages two queues for transferring characters between a
terminal device and a process :

* *The input queue* : contains characters typed at terminal, and read by the
    process
* *The output queue* : contains characters written by the process, and displayed
    at terminal (if echoing is enabled, the terminal driver appends a copy of
    input characters at the end of the output queue)

> Exercise #
>
> The following program sleeps during 5 seconds and then reads from the input
> queue (identified by `STDIN_FILENO`) at most 100 characters every second and
> then write this in the output queue (identified by `STDOUT_FILENO`). Enter
> several lines on the terminal during the initial 5 seconds. What do you
> conclude about the terminal I/O mode? This source code is available as
> `terminal.c`. WHat is the difference when the input file is replaced by a text
> file (for example, `./a.out < myfile`) ? Do not modify the content of
> `terminal.c` for this purpose.


```C
        #include <unistd.h>
        #include <stdio.h>
        
        int main(void) {
            sleep(5);
            printf("START\n");
        
            char buffer[100];
            ssize_t nb_read = 0;
            while((nb_read = read(STDIN_FILENO, buffer, 100)) > 0)
            {
                write(STDOUT_FILENO, buffer, nb_read);
                sleep(1);
            }
        
            return 0;
        }
```

Each session has usually a **controlling terminal** that is associated when the
*session leader* process opens a terminal device.^[The BSD convention is
different, where a process has to call `ioctl()` to obtain a controlling
terminal.] A terminal cannot be the controlling terminal for more than one
session. The process opening this device is known as the *controlling process*.
The controlling terminal is inherited by child processes. Each controlling
terminal refers to a process groups known as the *foreground process group*.
This process group is initially the process group of the session leader. The
terminal only sends signals to members of this group. Two functions are used to
manage background process groups :

* `pid_t tcgetpgrp(int fd)` : returns the PGID of the foreground process group
     of a given terminal identified by a file descriptor `fd`.
* `ìnt tcsetpgrp(int fd, pid_t pgrp)` : makes the process group identified by
    `pgrp` the foreground process of the terminal identified by `fd`.

A file descriptor for the controlling terminal is obtained by opening the
special file `/dev/tty`.

> Exercise #
> 
> Install `xterm` with your package manager (`yum`, `apt-get`, `dnf`,...). Write a
> program that has to be executed as the shell of this terminal emulator with the
> command `xterm` followed by the full path of your binary executable file. This
> program has to display its `pid`, `ppid`, `sid`, `pgid`, and the foreground
> process id of the controlling terminal. Execute the same program as a
> foreground and background process in a shell. What do you conclude ?

When the shell has to execute a command, or a pipeline, in the foreground
process groups, it creates a process group that contains the corresponding
processes and set this group as the foreground process group. When the shell
notices (with `wait()`) that the foreground process group has terminated, the
shell put itself back in the foreground with a call to `tsetpgrp()`.

The kernel sends a `SIGCONT` and a `SIGHUP` signal to the controlling process (for
example, the shell) when the terminal disconnection occurs (for example, the
terminal window is closed).  The default action of `SIGHUP` is to terminate the
process.  Most of the shell handle this signal and send a `SIGHUP` signal to
each of the process groups and then terminate. If the controlling process
terminates, then a `SIGHUP` signal is sent to the foreground process group.

A call to `setsid()` removes the *controlling terminal* of the calling process.

> Exercise #
>
> Use bash and enable the default `SIGHUP` handler with the command `shopt -s
> huponexit`.  Write a program that prints on standard output its `pid` and
> `pgid`, then creates a child process. The parent process wait for the
> completion of the child process. The child process prints every 3 seconds its
> `pid` and `pgid`.  Close the terminal. Then, open a second and check that it is
> terminated with the command `ps` followed by the process's `pid`. Modify this
> program so that the child process, and only this one, remains alive after the
> terminal window is closed.

In the previous exercise, if the parent process does not wait for its child to
complete, then the child process becomes an *orphaned process*. In this case,
the process is also part of an *orphaned process group*, because the parent
process was a process group leader. When the parent process exits, the shell
removes the corresponding process group from its list of jobs. Moreover, when a
process groups becomes *orphaned* and there is a *stopped* process in this
group, then all the members of this group are sent a `SIGHUP` signal followed by
a `SIGCONT`.

> Exercise #
>
> Write a program so that the process group becomes *orphaned*. Use a signal
> handler to display a message when the `SIGHUP` signal is received. Compare the
> behaviour of the child process if its state is suspended (with `sleep()` or
> `pause()`) or stopped when the process group becomes orphaned (use
> `raise(SIGSTOP)` to send `SIGSTOP` to the calling process). Ensure that the
> process group becomes orphaned *after* that the child process changes its
> state.

> Exercise #
>
> What happens when a background process tries to read something from its
> standard input ? For a better understanding, you can use the *strace* command
> that allows to prints the system calls called by a process and the signals
> received by the same process. For example, `strace ./a.out&`.

## Daemons

A **daemon** process is defined as follows:

* It is long-lived (*i.e.*, usually runs from system startup until the system shutdown)
* It runs in the background and has no controlling terminal (this ensure that
    the kernel never automatically send signals such as `SIGHUP` to this
    process)

The HTTP server *httpd* (Apache) is an example of *daemon* process.

A program has to perform the following steps for becoming a daemon:

1. Call the `fork()` system call in order to continue the execution in
   background with the child process.
1. The child process calls `setsid()` in order to remove its link to the
   controlling process.
1. Call a second time the `fork()` system call to ensure that the child is not
   the session leader. This implies that the process never reacquire a
   controlling terminal when it open a terminal device.
1. Change the current working directory to the root directory (`/`) so that it
   does not interfere with operations on the file system.
1. Close all the open file descriptors inherited from its parent for the same
   reason.
1. Use `dup2()` so that file descriptors 0, 1, and 2 refer to the `/dev/null`
   device.^[On this virtual device, a `read()` operations returns an
   end-of-file, and the data written with `write()` is discarded] This ensure
   that potential I/O operations will not fail.

> Exercise #
>
> Write a function called `becomeDaemon()` that performs all these steps. You
> have to use `chdir()` (see `man chdir`) to change the current working
> directory, and to call `sysconf(_SC_OPEN_MAX)` to determine the maximal value
> of file descriptor.

It is worth noting that a well designed daemon process should always handle the
`SIGTERM` signal to shutdown the process, and the `SIGHUP` signal to be able to
ask the process to reinitialize itself by rereading its configuration file.
