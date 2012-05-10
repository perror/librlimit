#include <assert.h>
#include <stdlib.h>
#include <signal.h>

#include <rlimit.h>

int
main ()
{
  int myargc = 2;
  char *myargv[] = { "/bin/sleep", "10" };

  subprocess_t *p = rlimit_subprocess_create (myargc, myargv, NULL);

  rlimit_set_time_limit (p, 2);
  rlimit_subprocess_run (p);
  rlimit_subprocess_wait (p);

  assert (p->retval == SIGKILL);
  assert (p->status == TIMEOUT);

  rlimit_subprocess_delete (p);

  return EXIT_SUCCESS;
}
