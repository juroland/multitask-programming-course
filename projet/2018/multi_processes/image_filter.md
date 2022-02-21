---
title: 'Multi-Task Programming - Assignment 1'
subtitle: 'Parallel Image Processing with UNIX Processes'
---

The purpose of this assignment is to implement a program in C or C++ that is able to perform
traditional image processing such as blurring, sharpening and edge detection. See the following
*wikipedia* article for more details : https://en.wikipedia.org/wiki/Kernel_(image_processing).

This program has to be designed to take advantage of all available processors to reach the best
possible performance by using UNIX processes (see, `man 3 get_nprocs` to get the number of
processors currently available).

A sequential implementation is available on the course web page (see `image_processing.c` compiled
with `gcc image_processing.c -lm`). This program takes two arguments : an input image filename
followed by an output image filename. Images have to follow the *Portable GrayMap* format (magic
number : `P5`).  See, http://netpbm.sourceforge.net/doc/pgm.html for more details.  An example of
input file in available on the course web page (see `eiffel.pgm`). Images can be converted to the
*pgm* format with the `convert` command from the *imagemagick* package. For example, `convert
eiffel.jpg eiffel.pgm`.
