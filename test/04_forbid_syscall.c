#include <assert.h>
#include <stdlib.h>

#include <rlimit.h>

int
main ()
{
  int myargc = 1;
  char *myargv[] = { "./utils/test_fork" };

  subprocess_t *p = rlimit_subprocess_create (myargc, myargv, NULL);

  rlimit_disable_syscall (p, SYS_fork);
  rlimit_disable_syscall (p, SYS_clone);

  rlimit_subprocess_run (p);
  rlimit_subprocess_wait (p);

  assert (p->retval == EXIT_SUCCESS);
  assert (p->status == DENIEDSYSCALL);

  rlimit_subprocess_delete (p);

  return EXIT_SUCCESS;
}
