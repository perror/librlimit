#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <rlimit.h>

int
main ()
{
  int myargc = 1;
  char *myargv[] = { "./utils/test_fork" };

  subprocess_t *p = rlimit_subprocess_create (myargc, myargv, NULL);

  rlimit_disable_syscall (SYS_fork, p);
  rlimit_disable_syscall (SYS_clone, p);

  rlimit_subprocess_run (p);
  rlimit_subprocess_wait (p);

  assert (p->retval == EXIT_SUCCESS);
  assert (p->status == DENIEDSYSCALL);

  rlimit_subprocess_delete (p);

  return EXIT_SUCCESS;
}
