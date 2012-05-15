#include <assert.h>
#include <stdlib.h>
#include <signal.h>

#include <rlimit.h>

int
main ()
{
  int myargc = 3;
  char *myargv[] = { "/bin/ls", "-R", "/" };

  subprocess_t *p = rlimit_subprocess_create (myargc, myargv, NULL);

  rlimit_subprocess_run (p);
  rlimit_subprocess_wait (p);

  rlimit_subprocess_delete (p);

  return EXIT_SUCCESS;
}
