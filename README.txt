This branch contains code for a custom memory allocator. The alloctor may be used instead of the
standard "malloc" provided, but this is not recommended.

Implimentation is as a Linked List of free nodes. Allocator iterates through Linked List looking
a free block of memory. If one is found, this block is given to the user. If not, a new page is 
pulled from the OS. Blocks are merged when "free" is called.

CODE I WROTE/MODIFIED:
- nu_mem.c
- nu_mem.h

ALL ELSE PROVIDED! 
