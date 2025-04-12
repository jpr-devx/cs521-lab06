
### Allocator Overview
#### How it Works
This allocator implements malloc, free, calloc and realloc by grabbing chunks of memory from the OS using mmap, freeing
them using munmap, and memcpy for when reallocation is requested. Each chunk of memory that is handed out includes 
information before the actual usable memory–this information keeps track of how big the block is and links to other 
memory blocks that were cached into a linked list to store memory blocks that were previously freed. Through this 
approach, the amount of system calls for memory is minimized due to the caching of freed memory, confining more within 
the user-space. When more allocation requests occur, the linked list is searched for memory blocks that can accommodate 
the amount of memory requested. If there is a reallocation request, it is first checked if the memory block at the given 
address can accommodate the new size, otherwise a new memory block is mapped to memory.

#### How it is Used
The allocator can be compiled with the command 
```terminaloutput 
cc allocator.c -shared -fPIC -DLOGGER=0 -DTRACE_ON=0 -o allocator.so
```

Once built, the allocation threshold is set and our custom allocator is used with the command below where 100 can be 
modified with any integer. The ideal allocation threshold is discussed for the memory allocator in the later sections of 
this README:
```terminaloutput
ALLOC_THRESH=100 LD_PRELOAD=$(pwd)/allocator.so find /
```

The chosen command follows the LD_PRELOAD argument. Above involves the ```find``` command being exccuted over the entire
file system tree.

<hr>

### Installing Termgraph
During the installation process, I ran into PEP 668 protections, which prevents installing Python packages system-wide 
with pip where pacman manages things. To resolve this issue and get termgraph installed, I was recommended the following 
when initially attempting to install it:
```terminaloutput
If you wish to install a non-Arch packaged Python application,
it may be easiest to use 'pipx install xyz', which will manage a
virtual environment for you. Make sure you have python-pipx
installed via pacman.
```
Following this the following was able to be done:
```terminaloutput
~/.local/bin/termgraph benchmark.out

0 : ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 2.74 
10: ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 1.65 
20: ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 1.47 
30: ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 1.32 
40: ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 1.18 

```

Extending the benchmarking out to an ALLOC_THRESHOLD of 400 showed a plateau. To minimize the amount needed, the best 
amount for the ALLOC_THRESHOLD is recommended to be 200 given the diminishing benefits increasing the ALLOC_THRESHOLD 
results in.
```terminaloutput
0  : ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 2.74
10 : ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 1.67
20 : ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 1.44
30 : ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 1.29
40 : ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 1.20
50 : ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 1.14
60 : ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 1.09
70 : ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 1.04
80 : ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 980.00T (0.98)
90 : ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 950.00T (0.95)
100: ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 920.00T (0.92)
110: ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 880.00T (0.88)
120: ▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 850.00T (0.85)
130: ▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 820.00T (0.82)
140: ▇▇▇▇▇▇▇▇▇▇▇▇▇▇ 790.00T (0.79)
150: ▇▇▇▇▇▇▇▇▇▇▇▇▇ 760.00T (0.76)
160: ▇▇▇▇▇▇▇▇▇▇▇▇▇ 740.00T (0.74)
170: ▇▇▇▇▇▇▇▇▇▇▇▇▇ 720.00T (0.72)
180: ▇▇▇▇▇▇▇▇▇▇▇▇▇ 720.00T (0.72)
190: ▇▇▇▇▇▇▇▇▇▇▇▇ 710.00T (0.71)
200: ▇▇▇▇▇▇▇▇▇▇▇▇ 700.00T (0.70)
210: ▇▇▇▇▇▇▇▇▇▇▇▇ 700.00T (0.70)
220: ▇▇▇▇▇▇▇▇▇▇▇▇ 700.00T (0.70)
230: ▇▇▇▇▇▇▇▇▇▇▇▇ 690.00T (0.69)
240: ▇▇▇▇▇▇▇▇▇▇▇▇ 700.00T (0.70)
250: ▇▇▇▇▇▇▇▇▇▇▇▇ 690.00T (0.69)
260: ▇▇▇▇▇▇▇▇▇▇▇▇ 700.00T (0.70)
270: ▇▇▇▇▇▇▇▇▇▇▇▇ 690.00T (0.69)
280: ▇▇▇▇▇▇▇▇▇▇▇▇ 690.00T (0.69)
290: ▇▇▇▇▇▇▇▇▇▇▇▇ 690.00T (0.69)
300: ▇▇▇▇▇▇▇▇▇▇▇▇ 680.00T (0.68)
310: ▇▇▇▇▇▇▇▇▇▇▇▇ 680.00T (0.68)
320: ▇▇▇▇▇▇▇▇▇▇▇▇ 680.00T (0.68)
330: ▇▇▇▇▇▇▇▇▇▇▇▇ 680.00T (0.68)
340: ▇▇▇▇▇▇▇▇▇▇▇▇ 670.00T (0.67)
350: ▇▇▇▇▇▇▇▇▇▇▇▇ 680.00T (0.68)
360: ▇▇▇▇▇▇▇▇▇▇▇▇ 680.00T (0.68)
370: ▇▇▇▇▇▇▇▇▇▇▇▇ 680.00T (0.68)
380: ▇▇▇▇▇▇▇▇▇▇▇▇ 680.00T (0.68)
390: ▇▇▇▇▇▇▇▇▇▇▇▇ 680.00T (0.68)
400: ▇▇▇▇▇▇▇▇▇▇▇▇ 680.00T (0.68)
```

There was a formatting error from termgraph that appended T and multiplied the times by 1000 once they were less than 1
second. What is seen in the benchmark.out file has been appended to the misformatted numbers after the fact.