#include <assert.h>
#include <stdlib.h>

#include <rlimit.h>

int
main ()
{
  int myargc = 1;
  char *myargv[] = { "./utils/test_malloc" };

  subprocess_t *p = rlimit_subprocess_create (myargc, myargv, NULL);
  
  rlimit_subprocess_run (p);
  rlimit_subprocess_wait (p);
 
  assert (p->profile->real_time_usec >= 0);
  assert (p->profile->memory_kbytes > 0);
  assert (p->retval == EXIT_SUCCESS);
  assert (p->status == TERMINATED);

  rlimit_subprocess_delete (p);

  return EXIT_SUCCESS;
}
