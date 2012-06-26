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
    rlimit_expect_stdout(p, "\.\.aa", 1) ? EXIT_FAILURE : EXIT_SUCCESS;

  rlimit_subprocess_delete (p);

  return result;
}
