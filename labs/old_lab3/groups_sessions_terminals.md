---
title: '**Groups, sessions, and terminals**'
---

Process groups and sessions are abstractions that provide an hierarchical
relationship between processes. A **process group** (often used as a synonym to
the term *job*) is a collection of one or more processes. Each process group is
identified with a *group process id* (PGID) equal to the PID of the process
leader. A **session** is a collection of process groups. Each session is
identified by a *session id* (SID) equal to the PID of the process that creates
the session. This process is called the **session leader**.

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

# Process groups

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
> 1. `./pgid& ./pgid&`
> 1. `(./pgid; ./pgid)&`
> 1. `(./pgid& ./pgid)&`


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
                        perror("parent : setpgid");
        
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

# Sessions

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

\newpage
# Terminals

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
to send a `SIGQUIT` signal to the foreground process group.

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
> `terminal.c`. What is the difference when the input file is replaced by a text
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
terminal refers to a process group known as the *foreground process group*.
This process group is initially the process group of the session leader. The
terminal only sends signals to members of this group.

\begin{figure}[h]
\centering
\includegraphics[scale=1.1]{../image/processes_groups_sessions_terminals.pdf}
\caption{Relationships between process groups, sessions, and the controlling terminal. (From LPI
book, section 34.1)}
\label{34.1}
\end{figure}

For example, in Figure \ref{34.1} are presented the different groups, the session, an their
relationships resulting from the execution of the following commands :

```BASH
$ echo $$
400
$ find / 2> /dev/null | wc -l &
[1] 659
$ sort < longlist | uniq -c
```

Two functions are used to manage background process groups :

* `pid_t tcgetpgrp(int fd)` : returns the PGID of the foreground process group
     of a given terminal identified by a file descriptor `fd`. The two first letters `tc` indicates
     that `tcgetpgrp` is part of the *terminal control* functions.
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
terminates, then a `SIGHUP` signal is sent to the foreground process group. For example, if the
controlling process (for example, the shell) is killed (*i.e.*, the `SIGKILL` signal is sent to this process which cannot be handled by the
target process). Then, the kernel send a `SIGHUP` signal to the foreground process.

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
process group becomes *orphaned* and there is a *stopped* process in this
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

# Daemons

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
