# Winter XV6 

Author: Sukjoon Oh, sjoon@kaist.ac.kr


## Exercise Questions

1. Look at real operating systems to see how they size memory.
2. If xv6 had not used super pages, what would be the right declaration for `entrypgdir`?
3. Write a user program that grows its address space with 1 byte by calling
`sbrk(1)`. Run the program and investigate the page table for the program before the call to `sbrk` and after the call to `sbrk`. How much space has the kernel allocated? What does the `pte` for the new memory contain?
4. Modify xv6 so that the pages for the kernel are shared among processes, which reduces memory consumption.
5. Modify xv6 so that when a user program dereferences a null pointer, it will receive a fault. That is, modify xv6 so that virtual address 0 isn’t mapped for user programs.
6. Unix implementations of `exec` traditionally include special handling for shell scripts. If the file to execute begins with the text `#!`, then the first line is taken to be a program to run to interpret the file. For example, if `exec` is called to run `myprog arg1` and `myprog`’s first line is `#!/interp`, then exec runs `/interp` with command line `/interp` `myprog arg1`. Implement support for this convention in xv6.
7. Delete the check `if(ph.vaddr + ph.memsz < ph.vaddr)` in `exec.c`, and construct a user program that exploits that the check is missing.
8. Change xv6 so that user processes run with only a minimal part of the kernel mapped and so that the kernel runs with its own page table that doesn’t include the user process.
9. How would you improve xv6’s memory layout if xv6 where running on a 64-bit
processor?


## Answers

### Sol 1. 

Real Linux (32-bit) memory layout is quite alike as xv6, but differ in terms of stack and heap position. Real Linux grows its stack downwards from the high address of a user space, whereas xv6 has fixed size (predefined) stack that sits below heap area.


### Sol 2.

Superpage is controlled by `PTE_PS` flag.


### Sol 3. 

A simple user program written to `exercise3.c` as shown below should be fine.

```c
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

```

After setting a breakpoint to `sys_sbrk()`, and `allocuvm()` GDB shows:

```sh
Thread 1 hit Breakpoint 2, sys_sbrk () at sysproc.c:47
47	{

...

Thread 1 hit Breakpoint 4, allocuvm (pgdir=0x8dff3000, oldsz=16384, newsz=49152) at vm.c:243
243	{

```

The argument shows how much memory it should increase, after the first call of `sys_sbrk()`. 

Set the breakpoint again to `mappages()`. The first observed pte is:

```sh
(gdb) p pte
$1 = (pte_t *) 0x8dff6010
```

Any other `pte`s can be observed in the same way.


### Sol 4. 

To make the kernel area shared among the processes, the `setupkvm()` function should be modifed since it creates common structure for all new user processes. However, the global mapping of the kernel area should be created at least once. Let the variable `kpgdir_init` represent a flag, that tells whether the kernel mapping is initialized or not. 

```c
// Exercise 4.
// Added. 
int kpgdir_init;
```

At the main entry, the flag `kpgdir_init` flag is initialized to zero before the mapping process. As stated, the mapping is created by `setupkvm()`, which is called at the `kvmalloc()` function. Thus, the flag is set to `1` after the call of `setupkvm()`. 

Note that `kpgdir_init` is only changed by the kernel, which is designed to be single thread in xv6. And, it is read-only after the call of `schedule()`. Thus, there is no need for locks for protection. 

```c
int
main(void)
{
  // Lock
  kinit1(end, P2V(4*1024*1024)); // phys page allocator

  // Exercise 4.
  kpgdir_init = 0;

  kvmalloc();      // kernel page table
  mpinit();        // detect other processors
  lapicinit();     // interrupt controller
  seginit();       // segment descriptors
  picinit();       // disable pic
  ioapicinit();    // another interrupt controller
  consoleinit();   // console hardware
  uartinit();      // serial port
  pinit();         // process table
  tvinit();        // trap vectors
  binit();         // buffer cache
  fileinit();      // file table
  ideinit();       // disk 
  startothers();   // start other processors
  kinit2(P2V(4*1024*1024), P2V(PHYSTOP)); // must come after startothers()
  
  // Always fixed at the call of setupkvm.
  // No need for any locks, since only kernel at start will modify.
  kpgdir_init = 1;
  
  userinit();      // first user process
  mpmain();        // finish this processor's setup
}
```

Now, the `setupkvm()` needs to build mapping just once for all. It checks the flag `kpgdir_init` and skips mapping if it is initialized. If the mapping is created, then the `setupkvm()` looks for the used entries in the `kpgdir` and copies to the newly generated page directory `pgdir`. 

```c
// Set up kernel part of a page table.
pde_t*
setupkvm(void)
{
  pde_t *pgdir;
  struct kmap *k;

  if((pgdir = (pde_t*)kalloc()) == 0)
    return 0;
  memset(pgdir, 0, PGSIZE);
  if (P2V(PHYSTOP) > (void*)DEVSPACE)
    panic("PHYSTOP too high");

  // Exercise 4: 
  // Modify xv6 so that the pages for the kernel are shared among 
  // processes, which reduces memory consumption.

  // If successfully initialized kernel pgdir before,
  // skip all but copy present entries.
  if (kpgdir_init)
  {
    int i;
    for(i = 0; i < NPDENTRIES; i++) 
      if ((kpgdir[i] & PTE_P)) pgdir[i] = kpgdir[i];
  }
  else
  for(k = kmap; k < &kmap[NELEM(kmap)]; k++)
    if(mappages(pgdir, k->virt, k->phys_end - k->phys_start,
                (uint)k->phys_start, k->perm) < 0) {
      freevm(pgdir);
      return 0;
    }
  return pgdir;
}

```

The user process however destroys all the memory it consumed. The mappings are destroyed at `freevm()` for the user area and the kernel area. The kernel area should not be unmapped, since it is globally accessed. Thus, the section that destroys mappings are commented out, as shown in the code below.

```c
void
freevm(pde_t *pgdir)
{
  // Exercise 4:
  // Deleted below:
  // uint i;

  if(pgdir == 0)
    panic("freevm: no pgdir");
  deallocuvm(pgdir, KERNBASE, 0);

  // Exercise 4:
  // Deleted below:
  // Should not delete all that exist in the kernel area.

  /*
  for(i = 0; i < NPDENTRIES; i++){
    if(pgdir[i] & PTE_P){
      char * v = P2V(PTE_ADDR(pgdir[i]));
      kfree(v);
    }
  }
  */
  kfree((char*)pgdir);
}

```


### Sol 5.

To prevent the xv6 from mapping the `NULL` address to the page table, following code should be modified: `exec.c:exec()`.

```c
  ...

  // Load program into memory.
  // Exercise 5: Dereferencing a NULL pointer
  sz = PGSIZE;
  // sz = 0;

  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }

  ...
```

The `allocuvm()` function increases memory use from old size to new size. Since the initial old size is set by the variable `sz` to `0`, xv6 allocates virtual memory from `0x00` to `ph.vaddr + ph.memsz` if not modified. Setting the initial value `sz` to `PGSIZE` prevents the first lowest page to be mapped to the page table. 

Of course, that not mapping the `0` to the page table makes other addresses that sits in the range of the first page table not accessible. 

*The code above is not implemented in this repository. Not mapping the first page to the page table causes panic. Please refer to the memory layout of a user program.*


### Sol 6.

To make a shell deal with the prefix `#!`, a user program defined in `sh.c` should be modified. The command is parsed by the `parsecmd()` and executed by the call of `exec()` system call. The `exec()` call sits in the `EXEC` case condition of the `runcmd()` function as shown in the snippet below. 

What needs to be done is that the `sh` progrm should first read a file whether it is a shell script or not to check the very first two characters `#!`. If the condition is met, the command string should be parsed again by `parsecmd()` and fill in the command struct `struct cmd`. The parsed string in newly created `ecmd` is passed to the `exec()` system call. 

```c
  case EXEC:
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit();

    // Exercise 6: 
    char* path = ecmd->argv[0];

    int fd;
    char fbuf[100] = { 0, };

    // Case when the path exists.
    if ((fd = open(path, O_RDONLY)) > 0)
    {
      read(fd, fbuf, sizeof(fbuf));
      
      if (fbuf[0] == '#' && fbuf[1] == '!')
      {
        path = (fbuf + 2);
        ecmd = (struct execcmd*)parsecmd(path);
      }

      close(fd);
    }

    exec(ecmd->argv[0], ecmd->argv);
    printf(2, "exec %s failed\n", ecmd->argv[0]);
    break;
```

To test whether it runs fine, a shell script is created in the repository as `exercise6.sh` that contains:

```sh
#!echo Hello
```

The modification to the `Makefile` as shown in the below makes it to copy the file to `fs.img` automatically at every compilation.

```
fs.img: mkfs README $(UPROGS)
	./mkfs fs.img README $(UPROGS) exercise6.sh
```

The result is shown as below:

```sh
cpu1: starting 1
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ exercise6.sh
Hello
```


### Sol 7. 

If the given condition check is deleted, then the check for whether the sum overflows a 32-bit integer. The danger is that a user could construct an ELF binary with a `ph.vaddr` that points into the kernel, and `ph.memsz` large enough that the sum overflows to `0x1000`.

To make a user program to overflow the specific value `0x1000`, the code size (`.text`) should increase and the memory usage (`.data` or stack) should also increase. (Refer to xv6-book chapter 2.)

The code below increases both, which generates nested function calls. Refer to the file `exercise7.c`.

```c
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

...

_FN_INCREASE(40, 39)


int
main(int argc, char *argv[])
{
    fn_40();
    return 0;
}
```

When observing using `readelf`, the result is shown below.

```sh
ee488@final:/vbox/winter-xv6/2-pagetable$ readelf -l _exercise7

Elf file type is EXEC (Executable file)
Entry point 0x0
There are 2 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x000080 0x00000000 0x00000000 0x010d8 0x010e4 RWE 0x10
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x10

 Section to Segment mapping:
  Segment Sections...
   00     .text .rodata .eh_frame .bss 
   01 
```

If the condition check is not removed, the xv6 shuts down the program.

```sh
$ exercise7
pid 3 exercise7: trap 14 err 6 on cpu 1 eip 0x33 addr 0xe0003fb8--kill proc
```


### Sol 8.

### Sol 9. 

64-bit address system has nearly unlimited range of usuable address. Mapping the kernel area to the page table for user program is necessary for performance, which makes it reasonable to stay unchanged. However, the stack and heap area may benefit from the enlarged memory area. Setting the kernel boundary high enough and letting stack and heap lay somewhere in between may be ideal. 
