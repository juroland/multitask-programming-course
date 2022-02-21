---
title: 'Multi-Task Programming - Assignment 2'
subtitle: 'Parallel word count with pipelines'
---

The purpose of this assignment is to implement a program in C or C++ that computes the absolute
frequency of words in a text file. The following pipeline can be used for this purpose :

```
cat big.txt | tr -s '[:digit:]' ' ' | tr '[A-Z]' '[a-z]' \
    | tr -s '[:punct:]' ' ' | tr -s '\n\f\t\r ' '\n' | sort | uniq -c
```

1. (10 points) Write a program `pipeline.c` that executes this pipeline. This program has to take as input a text file an write on the standard output the output of the last command.

The final program has to be designed to take advantage of all available processors to reach the best
possible performance by using UNIX processes and pipes. This program has to implement the following architecture :

\begin{center}
\includegraphics[scale=0.7]{graph.pdf}
\end{center}

For this purpose, you can use the following system calls : `fork()`, `exec()`, `dup2()`, `pipe()`,
`write()` and `read()`.

The next programs have to take as input a text file and write on the standard output an alphabetically ordered
list of tokens along with their absolute frequency. For example:

```
$ ./freq file1
amet 9
duis 4
ipsum 5
lorem 2
paratur 2
sed 1
```

2. (6 points) With the help of `merge_sum.cpp` (that is be compiled with the command `g++ merge_sum.cpp -std=c++14`) write a program `parallel_pipeline.c` that executes exactly 2 pipelines in parallel.
3. (4 points) Write a third program `n_parallel_pipeline.c` that generalizes this previous program by executing $n$ parallel pipelines, where the value of $n$ has to be provided as the second argument.
