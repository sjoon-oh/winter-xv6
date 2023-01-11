// Exercise 7
// Q : Delete the check if(ph.vaddr + ph.memsz < ph.vaddr) in exec.c, 
// and construct a user program that exploits that the check is missing.

// Author: Sukjoon Oh, sjoon@kaist.ac.kr
// 
// Add executable compilation info to Makefile.

#include "types.h"
#include "user.h"

#define LARGE_ENOUGH        (0x7FFFFFF)
// 2147483647

// Large stack, does nothing. 
#define _LOCAL_TEXT_INCREASE \
    do { int big_arr[LARGE_ENOUGH] = { 0xF, }; \
    printf(1, "%x\n", &big_arr[0]); } while(1)

// This is a code factory.
#define _FN_INCREASE(str, call) \
    void fn_##str() { _LOCAL_TEXT_INCREASE; fn_##call(); }

void fn_1() { _LOCAL_TEXT_INCREASE; }

_FN_INCREASE(2, 1) 
_FN_INCREASE(3, 2)
_FN_INCREASE(4, 3)
_FN_INCREASE(5, 4)
_FN_INCREASE(6, 5)
_FN_INCREASE(7, 6)
_FN_INCREASE(8, 7)
_FN_INCREASE(9, 8)
_FN_INCREASE(10, 9)
_FN_INCREASE(11, 10)
_FN_INCREASE(12, 11)
_FN_INCREASE(13, 12)
_FN_INCREASE(14, 13)
_FN_INCREASE(15, 14)
_FN_INCREASE(16, 15)
_FN_INCREASE(17, 16)
_FN_INCREASE(18, 17)
_FN_INCREASE(19, 18)
_FN_INCREASE(20, 19)
_FN_INCREASE(21, 20)
_FN_INCREASE(22, 21)
_FN_INCREASE(23, 22)
_FN_INCREASE(24, 23)
_FN_INCREASE(25, 24)
_FN_INCREASE(26, 25)
_FN_INCREASE(27, 26)
_FN_INCREASE(28, 27)
_FN_INCREASE(29, 28)
_FN_INCREASE(30, 29)
_FN_INCREASE(31, 30)
_FN_INCREASE(32, 31)
_FN_INCREASE(33, 32)
_FN_INCREASE(34, 33)
_FN_INCREASE(35, 34)
_FN_INCREASE(36, 35)
_FN_INCREASE(37, 36)
_FN_INCREASE(38, 37)
_FN_INCREASE(39, 38)
_FN_INCREASE(40, 39)


int
main(int argc, char *argv[])
{
    fn_40();

    return 0;
}