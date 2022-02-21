# Background

## File I/O

A process (an instance of an executing program) refers to an open file with a **file descriptor**, *i.e.*, a non-negative integer. For this purpose, the kernel maintains for each process a table of open file descriptors. Each entry in this table contains the following information:

* A set of flags (for example, *close on exec*)
* A reference to the open file descriptor in a system wide open file descriptor table

This file descriptor is obtained by a call to the system call `open()` (see `man 2 open` or [http://man7.org/linux/man-pages/man2/open.2.html](http://man7.org/linux/man-pages/man2/open.2.html)). When a process is started by the shell it starts with three standards file descriptors :

* 0 : standard input (stdin)
* 1 : standard output (stderr)
* 2 : standard error (stdout)

Once a file is opened, you can use various system calls such as `read()`, `write()`, and `close()` to perform I/O operations. Instead of using directly these system calls, you can use a set of functions form the *stdio* library. Further information about this library can be found on [http://man7.org/linux/man-pages/man3/stdio.3.html](http://man7.org/linux/man-pages/man3/stdio.3.html).

Almost every system call indicates whether the call succeeded or failed by returning a status value. Usually, a system call returns `-1` in order to indicate that an error occurred and sets the global integer variable `errno` to a positive value that specifies the type of error. An error message can be obtained, based on this value, by calling the functions `perror()` or `strerror()`.
	
**Exercises : **

You have to answer the following questions with the only help of the Linux man pages. If you have any questions, please do not hesitate to ask me.

1. Write a program that copies the content of an existing file into a new file. The name of these files should be provided as arguments of this program. You have to use the system calls `open()`, `close()`, `write()`, and `read()` (no functions from the *stdio* library except `fprintf` and `printf`). Furthermore, you have to handle all the errors that may occur. Compile this program with `-std=c99 -pedantic` to use the C99 standard, if you need to refresh your knowledge on the C programming language, you may refer to [http://learnxinyminutes.com/docs/c/](http://learnxinyminutes.com/docs/c/). You can start with the program skeleton `copy.c` available on the campus web site. What is the meaning of `openFlags` and `filePerms` ?

2. Write a program `cat.c` (without the use of the *stdio* library) that displays on the standard output the content of a file provided as argument of this program.

3. Write a function that contains the code fragment in common in these two programs. Declare this function in `io.h` and define this function in `io.c`. Use this function in the two previous programs.

## Makefile

You can use a Makefile in order to automate compilation and other repetitive tasks. The following example can be used to compile the programs from the previous exercises. For this purpose, write the following text in a file called `Makefile` and try the commands `make`, `make all`, `make clean`, and `make cat`.

```Makefile
all: copy cat

copy: copy.o io.o
	gcc copy.o io.o -o copy

cat: cat.o io.o
	gcc cat.o io.o -o cat

copy.o: copy.c io.h
	gcc -c copy.c -std=c99 -pedantic -Wall

cat.o: cat.c io.h
	gcc -c cat.c -std=c99 -pedantic -Wall

io.o: io.c io.h
	gcc -c io.c -std=c99 -pedantic -Wall

clean:
	rm -f copy cat copy.o cat.o io.o
```

You can use variables in order to remove duplicate commands and to be able to change options more easily. Try and understand the following Makefile:

```Makefile
# The variable CC is the compiler to use.
CC=gcc
CFLAGS=-c -Wall -std=c99 -pedantic

all: copy cat

copy: copy.o io.o
	$(CC) copy.o io.o -o copy

cat: cat.o io.o
	$(CC) cat.o io.o -o cat

copy.o: copy.c io.h
    $(CC) $(CFLAGS) copy.c

cat.o: cat.c io.h
	$(CC) $(CFLAGS) cat.c

io.o: io.c io.h
	$(CC) $(CFLAGS) io.c

clean:
	rm -f copy cat *.o
```

This Makefile can also be improved with the use of special variables such as `$<` and `$@`. Try and understand the following Makefile:

```Makefile
CC=gcc
CFLAGS=-c -Wall -std=c99 -pedantic

all: copy cat

# $^ is the name of all the prerequisites (in this case copy.o and io.o)
# $@ is the target (in this case copy)
copy: copy.o io.o
	$(CC) $^ -o $@

cat: cat.o io.o
	$(CC) $^ -o $@

# $< is name of the first prerequisite
# this rule tells how to make '.o' files from '.c' files (this rule also depend on io.h)
%.o: %.c io.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f copy cat *.o
```

