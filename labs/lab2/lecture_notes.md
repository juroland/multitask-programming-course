---
title: 'Processes'
---

A **process** is an instance of an executing program. It is characterised by one
or more sequences of instructions executed in the same *private address space*.
This means that one process is not able to read or modify the memory of another
process. However, as explained later in more detail, several processes can share
memory by executing the same program or with shared memory techniques.

The execution of a process depends on the value of the instruction pointer. This
value is incremented by the CPU to point to the next instruction and modified by
instructions such as `jmp`. This sequence of instruction is called a
*thread of execution*.

# Credentials

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

The kernel provides information about each process on the system in the `/proc`
file system. For each process id PID, there exists a directory named `/proc/PID`
that contains various files containing information about that process (see, `man
proc` for more details).

# Memory layout

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

The memory layout of a given process is displayed with the command `cat
/proc/PID/maps` where PID is the process id of the considered process. Each line
describes an address space with the corresponding permissions (r = read, w =
write, x = execute, s = shared) and pathname.

# Virtual memory management

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

# System calls

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

# Signals

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

# File I/O

\centerline{\includegraphics[scale=0.8]{../image/file_tables.pdf}}

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
operations.

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

# Process creation
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

When a child process is created, it receives a copy of all the parent's file
descriptors. Therefore, they point to the same entry in the open file table
that contains, among other things, the current file offset. Consequently, if
this offset is modified by the child process, then it is also modified for the
parent process.

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

The following attributes are **not** inherited after a call to an `exec()` function:

* Process IDs (PID, PPID)

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
