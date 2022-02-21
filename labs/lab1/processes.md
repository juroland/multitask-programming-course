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
> Compile the following two source files as object files (`gcc -c`). Then, compare
> and explain, the output of the `size` command.

```C
        /* size1.c */
        int main(int argc, char **argv) {
            return 0;
        }
        
        /* size2.c */
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
