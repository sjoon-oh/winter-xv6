// Semaphores

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "semaphore.h"

// Set the maximum threads to count and initialize anything you need.
void
initsema(struct semaphore *lk, int count)
{
  initlock(&lk->lk, "semaphore");
  lk->ori_res = count;
  lk->res = count;
}

// Similar with acquiring the lock, but need to check how many threads are in there.
int
downsema(struct semaphore *lk)
{
  int ret;
  acquire(&lk->lk);
  if (lk->res > 0){
    ret = --lk->res;
  }else{
    while (lk->res <= 0);
    ret = --lk->res;
  }
  if (ret < 0)
    ret = 0;
  release(&lk->lk);
  return ret;
}

// Similar with releasing the lock. Return the number of remained threads that can access the section
int
upsema(struct semaphore *lk)
{
  int ret = lk->ori_res;
  acquire(&lk->lk);
  if (lk->res < lk->ori_res){
    ret = ++lk->res;
    if (ret < 0)
      ret = 0;
  }
  release(&lk->lk);
  return ret;
}
