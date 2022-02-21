---
title: 'Assignment 1 : **Building a simple shell**'
---
This assignment is based on the *shell homework* from the MIT's operating systems course (see,
[https://pdos.csail.mit.edu/6.828/2014/index.html](https://pdos.csail.mit.edu/6.828/2014/index.html))

This program has to be implemented in the C or C++ programming languages and executed by the `xterm`
terminal emulator. The source code `sh.c` and the grammar `sh_grammar.y` are provided as a basis for
implementing it. This program contains a parser that recognises shell commands such as the
following:

```BASH
ls > y
cat < y | sort | uniq | wc > y1
cat y1
rm y1
ls |  sort | uniq | wc
rm y
> out.txt ls
> out.txt
< in.txt > out.txt sort
```

This program has to implement the following functionality:

* Execution of the built-in command `cd`.
* Execution of simple commands such as `ls`.
* Execution of commands as foreground or background process groups. For example,
    `ls -l`, `./myprog&`, etc.
* Redirection of commands standard input and standard output. For example, 
    `echo "hello" > x.txt`, `sort < in.txt`, `ls -l > myfile`, etc.
* Execution of pipelines of commands such as `ls | sort | uniq | wc`.
* To handle `SIGHUP`, `SIGINT`, and `SIGQUIT` signals so that they are sent to
    the right process groups.
