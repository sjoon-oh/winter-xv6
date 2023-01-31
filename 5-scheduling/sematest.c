#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main()
{
  int i, pid;

  // initialize the semaphore
  sematest(0);

  for(i = 0; i < 10; i++){
    pid = fork();

    if (!pid)
      break;
  }

  if(pid){
    sleep(300);
    for(i = 0; i < 10; i++)
      wait();
    sematest(1);
    printf(1, "Final %d\n", sematest(2));
  }else{
    printf(1, "%d Down : %d\n", i, sematest(1));
    //sleep(100);
    printf(1, "%d Up : %d\n", i, sematest(2));
  }

  exit();
}
