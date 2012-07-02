#include <assert.h>
#include <stdlib.h>
#include <signal.h>

#include <rlimit.h>

int
main ()
{
  int result;
  int myargc = 2;
  char *myargv[] = { "/bin/ls", "-a"};

  subprocess_t *p = rlimit_subprocess_create (myargc, myargv, NULL);

  rlimit_subprocess_run (p);

  /* FIXME: Waiting the subprocess to start properly */
  sleep(1);
  
  result =
    rlimit_expect_stdout(p, "\\.\\.", 10) ? EXIT_SUCCESS : EXIT_FAILURE;

  rlimit_subprocess_wait(p);
  rlimit_subprocess_delete (p);

  return result;
}
