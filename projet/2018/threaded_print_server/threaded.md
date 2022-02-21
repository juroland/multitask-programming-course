---
title: 'Multi-Task Programming - Assignment 3'
subtitle: 'Threaded Print Server'
---

The purpose of this assignment is to develop a simulation of a threaded print server based on
*producer-consumer synchronization*.

The simulation scenario is described in a text file. For example :

```
print_file 1 3
sleep 3
print_file 2 2
print_file 3 7
print_file 4 5
sleep 1
print_file 5 3
sleep 2
print_file 6 6
```

Each line in this file describes an *event*. A line that starts with `print_file` represents a
new input request to print a file. The integer that directly follows this keyword is an id that
uniquely identifies this file. The next integer indicates the time (in seconds) that is required
to print this file by a printer. The type of event is `sleep` followed by an integer that
indicates the time to wait before the execution of the next event.

We consider the simulation of a print server connected to three printers. When an event
`print_file` occurs, it request is placed by the server in a queue. In practice, when a printer
becomes free, the server writes the file (in the beginning of the queue) to the printer device.
In order to simulate this behavior, the server has to writes a new line that contains the number
of seconds since *epoch* followed by the string "print file" followed by the file unique *id* in
the file "print.i.txt", where *i* is the printer identifier (equal to 1, 2, or 3) and simulate
the time required to print this file by using `sleep()`. For example, the content of
`print.2.txt` might be what follows :

```
1518020048 print file 4
1518020057 print file 7
1518020102 print file 12
```

Your implementation has to follow the following architecture.

\begin{center}
\includegraphics[scale=0.3]{architecture.pdf}
\end{center}

*Hint : This architecture requires the use of exactly 4 threads.*
