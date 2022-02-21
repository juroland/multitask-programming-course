# Background

## File I/O

A process refers to an open file with a file descriptor, *i.e.*, a non-negative integer. For this purpose, the kernel maintains for each process a table of open file descriptors. Each entry in this table contains the following information:

* A set of flags (for example, *close on exec*)
* A reference to the open file descriptor in a system wide open file descriptor table

This file descriptor is obtained by a call to the system call `open()` (see `man 2 open` or [http://man7.org/linux/man-pages/man2/open.2.html](http://man7.org/linux/man-pages/man2/open.2.html)). When a process is started by the shell it starts with three standards file descriptors :

* 0 : standard input (stdin)
* 1 : standard output (stderr)
* 2 : standard error (stdout)

Once a file is opened, you can use various system calls such as `read()`, `write()`, and `close()` to perform I/O operations. Instead of using directly these system calls, you can use a set of functions form the *stdio* library. Further information about this library can be found on [http://man7.org/linux/man-pages/man3/stdio.3.html](http://man7.org/linux/man-pages/man3/stdio.3.html).

Almost every system call indicates whether the call succeeded or failed by returning a status value. Usually, a system call returns `-1` in order to indicate that an error occurred and sets the global integer variable `errno` to a positive value that specifies the type of error. An error message can be obtained, based on this value, by calling the functions `perror()` or `strerror()`.
	
**Exercise : ** Write a program that copies the content of an existing file into a new file. The name of these files should be provided as arguments of this program. You ha to use the system calls `open()`, `close()`, `read()`, and `exit()`. Furthermore, you have to handle all the errors that may occur. Compile this program with `-std=c99 -pedantic` to use the C99 standard, if you need to refresh your knowledge on the C programming language, you may refer to [http://learnxinyminutes.com/docs/c/](http://learnxinyminutes.com/docs/c/). You can start with the program skeleton `copy.c` available on the campus web site.

## Processes

A process is an instance of an executing program. A new process, the so-called *child process*, is created by a call to `fork()` by another process, the so-called *parent process*. The Kernel records various information about each process. For example:

* process identifier (PID) -- this ID is obtained with the system call `getpid()`
* parent process identifier (PPID) -- this ID is obtained with the system call `getppid()`
* termination status : obtained by the parent process with a call to `wait()`. A process specifies its termination status by a call to `exit()`. By default, the termination status is equal to `0` in order to indicate that the process succeeded and is equal to a non-zero value if some error occurred.
* Real user ID and real group ID : identifies the user and group to which the process belongs. These IDs are inherited from the parent process.
* Effective user ID and effective group user ID : used to determine the permissions to process has when accessing resources such as files.

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
        exit(EXIT_FAILURE);
	}
		
	exit(EXIT_SUCCESS);
}
```

**Exercise : ** Write a program that executes a command passed as parameter, wait the return value, and print the returned value. For example, `./launch ps -f` executes the command `ps -f` and display its termination status. For this purpose, you can use the system call `execvp()` for executing the command with its arguments, and `wait()` for suspending the executing of the calling process until its children terminates.

## Threads

Each process can have multiple threads of execution. Each thread, within a same process, is executing the same program code, and shares the same data area and heap. Threads also share other attributes such as PID, PPID, file descriptors, etc. However, each thread has also various distinct attributes such as the `errno` variable, signal mask, stack (local variables), thread ID, etc.

Threads are created and managed with the *Pthreads* API declared in `pthread.h`. When a process starts, it contains a single thread. The function `pthread_create()` creates a new thread in the calling process. This new thread starts its execution in a function specified as argument of `pthread_create`.

A thread is terminated in one of the following ways:

* the thread function performs a return
* the thread calls `pthread_exit()`
* the thread is canceled using `pthread_cancel()`
* all the thread terminates immediately when any thread (in the considered process) calls `exit()`, or the main thread returns in the `main()` function

`pthread_join`

`pthread_detach`

`pthread_mutex_lock`

`pthread_mutex_unlock`

Condition Variables

`pthread_cond_signal`

`pthread_cond_broadcast`

`pthread_cond_wait`

`semaphore`

## Signals

A signal is sent by a process A (or the Kernel itself) to another process B to inform this second process that some event has occurred. Here are some examples of events that may lead to send a signal by the Kernel to a process:

* interrupt character (for example, `Control-C`)
* children process termination
* access to an invalid memory access

A process is also able to send a signal with the system call `kill()`. When a process receives such a signal, it can either ignore the signal, be killed by the signal, be suspended until another special purpose signal is received, or handle the signal.


