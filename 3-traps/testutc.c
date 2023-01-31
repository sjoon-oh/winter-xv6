#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main()
{
  int utc;
  utc = utctime();
  printf(1, "utctime: %d\n", utc);

  exit();
}
