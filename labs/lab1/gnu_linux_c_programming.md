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

The **processing** stage consists of executing directives (such as `#include`, `#define`, and
`#ifdef`) and expanding macro calls. The preprocessor output is directed to the standard output with
the following command.

```
$ gcc -E hello.c
```

The **compilation** stage translates the C code into the machine's assembly
language. This translation is created with the following command.

```
$ gcc -S hello.c
```

This command creates an assembly language file named `hello.s`. In this file,
you can see that `gcc` adds calls to other procedures such as `puts` (that
replaces the call to `printf` when there is only one parameter) and
`__stack_chk_fail` (that is called if a stack overflow is detected).

The **assembly** stage consists of translating the assembly code into executable
binary code. The result of this stage is called an *object file* that contains
the machine code and a *symbol table* describing all identifiers in the file.
This file is produced with the `-c` option.

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
produce a single executable file. This operation usually adds several
dependencies to shared libraries such as `libc.so` that gives access to standard
library functions such as `printf` or `scanf`. A list of these dependencies is
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

/* circle.c */
#include "circle.h"
double circle_area(double r) {
    return PI * r * r; 
}
```

These source files are compiled as follows.

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

## Shared library

The source file `circle.c` can be compiled into a shared library so that this
code is shared among multiple programs at run time. For this purpose, we compile
this file with the option `-fPIC`.

```
$ gcc -c -fPIC circle.c
```

This generates an object file `circle.o` with *position independent code*. This allows to load the
code at any virtual address at runtime (for each program that is loading the same library). See LPI
section 41.4.2 for more details. Then the shared library is generated as follows.

```
$ gcc -shared -o libcircle.so circle.o
```

The program is linked with the shared library as follows.

```
$ gcc -o myexe main.o libcircle.so
```

In order to be able to execute `myexe`, from any working directory, the
location of `libcircle.co` has to be specified in the variable
`LD_LIBRARY_PATH`. For example, we can run our program with the following
command.

```
$ LD_LIBRABRY_PATH=/path/to/your/library/ /path/to/your/binary/myexe
```

Another solution is to export a new value of this variable as follows so that the variable is in the
environment of the next executed commands.

```
$ export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/path/to/your/library/"
$ /path/to/you/binary/myexe
```

Use `ldd` to check that the location of this library is determined correctly.

Shared libraries are traditionally installed in several directories such as
`/usr/lib`, `/lib`, `/usr/local/lib`, or one of the directories listed in
`/etc/ld.so.conf`.  For example, if we need to link with a known shared library
such as `libpthread.so`, then we add `-l` followed by `pthread` (i.e., the
library name without `lib`). For example `gcc -o myexe main.o -lpthread`.
In order to speed up the loading of shared libraries, their locations are stored
in `/etc/ld.so.cache`. The content of this file is updated with the command
`ldconfig`, and `ldconfig -p` prints its current content.

We are also able to refer to `libcircle.so` with the option `-l` followed by
`circle` . In this case, the library is not installed in a directory specified
in `/etc/ld.so.conf`. Therefore, we have to specify the location of this library
with the `-L` option (in this case the current working directory (`.`)).

```
$ gcc -L. -o myexe main.o -lcircle
```

## Makefile

You can use a Makefile in order to automate compilation and other repetitive
tasks. The following example can be used to compile the programs from the
previous sections. For this purpose, write the following text in a file called
`Makefile` and try the commands `make`, `make all`, `make clean`, and `make
myexe`.

```Makefile
all: myexe myexe_static

myexe: main.o circle.o 
	gcc main.o circle.o -o myexe

myexe_static: main.o circle.o 
	gcc -static main.o circle.o -o myexe_static

circle.o: circle.c circle.h
	gcc -c circle.c -std=c11 -pedantic -Wall

main.o: main.c circle.h
	gcc -c main.c -std=c11 -pedantic -Wall

clean:
	rm -f myexe circle.o main.o
```

In this case we use options such as `-std=c11` to set the compiler to use the
C11 standard for the C programming language. You can use variables in order to
remove duplicate commands and to be able to change options more easily. Try and
understand the following Makefile:

```Makefile
# The variable CC is the compiler to use.
CC=gcc
CFLAGS=-c -Wall -std=c11 -pedantic

all: myexe myexe_static

myexe: main.o circle.o 
	$(CC) main.o circle.o -o myexe

myexe_static: main.o circle.o
	$(CC) -static main.o circle.o -o myexe_static

circle.o: circle.c circle.h
	$(CC) $(CFLAGS) circle.c

main.o: main.c circle.h
	$(CC) $(CFLAGS) main.c

clean:
	rm -f myexe myexe_static *.o
```

This Makefile can also be improved with the use of special variables such as
`$<` and `$@`. Try and understand the following Makefile:

```Makefile
CC=gcc
CFLAGS=-c -Wall -std=c99 -pedantic

all: myexe myexe_static

# $^ is the name of all the prerequisites (in this case circle.o and main.o)
# $@ is the target (in this case myexe)
myexe: circle.o main.o
	$(CC) $^ -o $@

myexe_static: circle.o main.o
	$(CC) -static $^ -o $@

# $< is name of the first prerequisite
# this rule tells how to make '.o' files from '.c' files (this rule also depend
# on io.h)
%.o: %.c circle.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f myexe myexe_static *.o
```


## The GNU debugger

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
> correct all of them. Finally, ensure that any memory leak is remove from this program.
