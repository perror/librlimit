#include <assert.h>
#include <signal.h>
#include <stdlib.h>

#include <rlimit.h>

int
main ()
{
  int myargc = 1;
  char *myargv[] = { "./utils/test_fork" };

  subprocess_t *p = rlimit_subprocess_create (myargc, myargv, NULL);

  rlimit_set_time_limit (p, 1);
  rlimit_disable_syscall (p, SYS_fork);
  rlimit_disable_syscall (p, SYS_clone);

  rlimit_subprocess_run (p);
  rlimit_subprocess_wait (p);

  assert (p->retval == SIGKILL);
  assert (p->status == TIMEOUT);

  rlimit_subprocess_delete (p);

  return EXIT_SUCCESS;
}
