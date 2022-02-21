---
title: 'Lab 1 : Background'
author: Julien Roland
date: 2018
---

# C Programming in the GNU/Linux environment

The purpose of this section if to familiarize yourself with the main tools we'll
be using in this part of the course.  It covers programs and tools such as `gcc`, `ldd`,
`make`, and `gdb`.

Let us start with a simple C program. Save the following code in a file called
`hello.c` with a text editor of your choice.

```C
#include <stdio.h>

#define ARRAY_SIZE 10
#define PI 3.1415926536

static void print_hello() {
    printf("Hello, world !\n");
}

double circle_area(double r) {
    return PI * r * r; 
}

int main(int argc, char **argv) {
    int v[ARRAY_SIZE];
    print_hello();
    printf("%f \n", circle_area(2.0));
    return 0;
}
```

We use `gcc` to make an executable program from this source code as follows. In
this document, the `$` symbol stands for the shell prompt. Try these commands
by yourself.

```
$ gcc hello.c
```

This command produces an executable program file named `a.out` ("assembler
output") in the current working directory. The `-o` option allows to specify an
executable file name different than `a.out`. For example,

```
$ gcc hello.c -o hello
```

produces an executable program file named hello.

The complete compilation process is composed of four stages :

1. Processing
2. Compilation
3. Assembly
4. Linking

The **assembly** stage consists of translating the assembly code into executable
binary code. The result of this stage is called an *object file* that contains
the machine code and a *symbol table* describing all identifiers in the file.
This file (resulting from this stage) is produced with the `-c` option. This file is named
`hello.o` by default.

```
$ gcc -c hello.c
```

The symbol table is displayed with the following command.

```
$ nm hello.o
```

Line starting with an upper case `U` identify undefined symbols such as
`printf` that is defined in the *libc* shared library. Two other kinds of
symbols are distinguished: local symbols (lower case `t`) and externally
visible symbols (upper case `T`). Local symbols cannot be used (i.e., called or
referenced) in another translation unit (i.e., a C source file with all the
headers files). Conversely, externally visible symbols can be used anywhere in
the entire program.

The **linking** stage consists of joining several object files together to
produce a single executable file. The result from this last stage (in this case named `hello`) can
be directly obtained with the following command:

```
$ gcc hello.c -o hello
```

This executable can also be obtained from the object file `hello.o` with the following command:

```
$ gcc hello.o -o hello
```

This operation usually adds several dependencies to shared libraries such as `libc.so` that gives
access to standard library functions such as `printf` or `scanf`. A list of these dependencies is
displayed with the following command.

```
$ ldd hello
```

A program can be linked statically to these libraries with the `-static` option.

```
$ gcc -static hello.c -o hello
```

In this case, the resulting executable file contains a copy of all the object
files that were linked to the program which increases the size of your
application. Obviously, in this case `ldd hello` displays no dependencies.

> Exercise #
>
> Write a program that displays on the standard output the number of arguments and each argument
> along with its length (*i.e.*, the number of characters).
>

> Exercise #
>
> Define two functions named `double circle_perimeter(double r)` and `double square(double r)`
> so that only `circle_perimeter` is visible from external translation units. Use `square` in the
> implementation of `circle_area`.

Let us split up the previous program `hello.c` into two separate source files
named `circle.c`, `main.c` and a header file `circle.h`.
```C
/* main.c */
#include <stdio.h>
#include "circle.h"

#define ARRAY_SIZE 10

static void print_hello() {
    printf("Hello, world !\n");
}

int main(int argc, char **argv) {
    int v[ARRAY_SIZE];
    print_hello();
    printf("%f \n", circle_area(2.0));
    return 0;
}

/* circle.h */
#define PI 3.1415926536

double circle_area(double r);

/* add remaining declarations */

/* circle.c */
#include "circle.h"

double circle_area(double r) {
    return PI * square(r);
}

/* add remaining definitions */
```

Object files from these source files can be obtained as follows:

```
$ gcc -c circle.c
$ gcc -c main.c
```

These commands produce two object files named `circle.o` and `main.o`. What is
the output of `nm main.o` and `nm circle.o`? 

These two object files are linked together with the following command to produce the
executable command
named `myexe`.

```
$ gcc -o myexe circle.o main.o
```

> Exercise #
>
> Modify the source files so that the function `double square(double)` can be used in `main.c`. How
> can you check this before trying to use it in `main.c` ?

> Exercise #
>
> What happens if you add a second definition of `double square(double)` inside `main.c` how would
> you fix this without removing the implementation from `circle.c` ?

> Exercise #
>
> In the previous exercise, do both definition of `double square(double)` have to be declared as
> static ? Why ?

# The GNU debugger

The GNU debugger `gdb` allows to run a program in an environment that allows to
control its execution (i.e., to run the program step by step or to break its
execution when a certain condition is met) and to observe the contents of
variables, memory locations, and CPU registers.

Consider the following source code.

```C
/* gdb_example.c */
#include <stdio.h>

void swap( int *p1, int *p2 );

int main()
{
    int a = 10, b = 20;
    printf( "The old values: a = %d; b = %d.\n", a, b );
    swap( &a, &b );
    printf( "The new values: a = %d; b = %d.\n", a, b );
    return 0;
}

void swap( int *p1, int *p2 )
{
    int p = *p1;
    *p1 = *p2;
    *p2 = p;
}
```

This source code is compiled with the `-g` option in order to add some
information for the debugger.

```
$ gcc -g gdb_example.c -o gdb_example
```

In this case, `gdb` is invoked as follows.

```
$ gdb gdb_example
```

Once this command has been executed, the program is loaded by `gdb`. This
interactive program waits for your instructions. The program is ready to be
executed. The following commands are the most commonly used 

* `list` (or `l`) : List a few lines of source code where the program is
  presently stopped (i.e., the next line that is ready to be executed).
* `break` (or `b`) : Specify where the execution of the program has to be
  interrupted.  For example, `break 12` indicates the line 12, and `break swap`
  indicates the function `swap`.
* `run` (or `r`) : Start you program. 
* `c` : Continue running your program. For example, after being stopped by a
  breakpoint.
* `next` : Execute the next program line, and interrupts the program again at
  the following line. A function call is considered as a single step.
* `step` : Execute the next program line. If the next line contains a function,
  then it interrupts the program again at the first statement in the function.
* `print` : Display the value of a given expression. For example if `pi` is a
  pointer to an integer, then `print *pi` displays the integer value pointed by
  `pi`.
* `quit` : Exit from GDB.
* `bt` : Display the program stack.
* `info breakpoints` : Display all the breakpoints.
* `delete` : Delete a given breakpoint.
* `frame` : Select the stack frame specified by the given number. For example
  `frame 3`.

> Exercise #
>
> Use gdb to display the values pointed by `p1` and `p2` at different points of
> `gdb_example.c`.

> Exercise #
>
> Compile and execute the program `fail.c`. This program executed as `./a.out 10` should lead to
> a segmentation fault. Without reading its source code, determine the function that leads to this
> error, as well as the function that called it. Then, determine each error with the help of gdb and
> correct all of them (Hint : look at the value of pointers). Finally, ensure that any memory leak is
> removed from this program.

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

The return type is `pid_t`, defined as an unsigned integer type. Its a common
practice to cast this value to `long` for printing, because there is no guarantee
that a `pid_t` will fit in a `int`.

> Exercise #
>
> Write a C program that displays the process ID. See `man getpid` for more
> details about this system call.

The kernel provides information about each process on the system in the `/proc`
file system. For each process id (PID), there exists a directory named `/proc/PID`
that contains various files containing information about that process (see, `man
proc` for more details).

> Exercise #
>
> Create a program that displays its `pid` and then waits for some input before
> returning. Execute the program with three arguments. For example, `./a.out one
> two three`. In a second terminal, display the content of `/proc/PID/cmdline`
> (with `od -c /proc/PID/cmdline`), and list the file attributes of
> `/proc/PID/exe`, and `/proc/PID/cwd` with `ls -l`. Explain the result.

> Exercise #
>
> Use the command `ps -p` followed by a `pid` to display details about the process
> from the previous exercise.

> Exercise #
>
> The status of every process on the system is displayed with the command `ps -e`. What is
> displayed with `ps -e -o pid,comm,%mem,%cpu` ?

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
> Use `ls -l` to compare the size of the object files for the following
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
> Create a program that displays its `pid`, allocates (and initializes with not null values) an
> array in the heap, declares and initializes a global array, declares a global array (not
> initialized), and then waits for some input before returning. Display its memory layout and
> identify its text, data, stack, and heap segments. Further segments are also displayed.
> A read-only segment that contains data such as constant strings. Several segments refer to shared
> libraries if the source code is not compiled with the `-static` option. Other segments such as
> `vvar`, `vdso` and `vsyscall` are used on Linux for improving the performance of frequent system
> calls (see `man vdso` for more details).
