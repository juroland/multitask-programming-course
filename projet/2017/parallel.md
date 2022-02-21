---
title: 'Assignment 2 : **Parallel Programming with Pthreads**'
---

The purpose of this assignment is to implement a program in C or C++ that computes the absolute
frequency of tokens in a set of text files, where a token is any sequence of characters separated by
whitespace.

This program has to be designed to take advantage of all available processors to reach the best
possible performance (see, `man 3 get_nprocs` to get the number of processors currently available)
by using POSIX threads. For this purpose, you have to use patterns presented in the paper
*Structured Parallel Programming with Deterministic Patterns* available online at
[https://software.intel.com/sites/default/files/m/d/4/1/d/8/HotPar_mccool_patterns_final_pub.pdf](https://software.intel.com/sites/default/files/m/d/4/1/d/8/HotPar_mccool_patterns_final_pub.pdf).
In particular, the so-called *Map* and *Reduction* patterns are of clear interest to tackle this
problem.

This program has to take as input a variable number of text files and write on the standard output
an **alphabetically ordered** list of tokens along with their absolute frequency. For example :

```
$ ./freq file1 file2.txt foo file12.txt
amet, 9
Duis 4
ipsum 5
Lorem 2
paratur. 2
sed 1
```

A text generator is available on the course web page (see `generator.cpp`). This program (compiled
with the command `g++ -std=c++14 generator.cpp`) takes as single argument an integer named `n` and
writes on the standard output a randomly generated text that contains at least `n` tokens.

You can read a text file as usual and then use the `strtok()` function to break a string into
a sequence of tokens. However, do not forget to pay attention to the memory usage of your program.
