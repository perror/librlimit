#include <assert.h>
#include <stdlib.h>

#include <rlimit.h>

int
main ()
{
  int myargc = 2;
  char *myargv[] = { "/bin/sleep", "5" };

  subprocess_t *p = rlimit_subprocess_create (myargc, myargv, NULL);
  
  rlimit_subprocess_run (p);
  rlimit_subprocess_wait (p);

  assert (p->real_time_usec >= 5000);
  assert (p->memory_kbytes > 0);
  assert (p->retval == EXIT_SUCCESS);
  assert (p->status == TERMINATED);
  
  rlimit_subprocess_delete (p);

  return EXIT_SUCCESS;
}
