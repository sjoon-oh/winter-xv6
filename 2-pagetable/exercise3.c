// Exercise 3
// Q : Write a user program that grows its address space with 1 byte by calling sbrk(1). 
// Run the program and investigate the page table for the program before the
// call to sbrk and after the call to sbrk. How much space has the kernel allocated?
// What does the pte for the new memory contain?

// Author: Sukjoon Oh, sjoon@kaist.ac.kr
// 
// Add executable compilation info to Makefile.

#include "types.h"
#include "user.h"


int
main(int argc, char *argv[])
{
    printf(1, "exercise3 start.\n");
    sbrk(1);

    printf(1, "exercise3 end.\n");
    exit(); // syscall, exit()
}
