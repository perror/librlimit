#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rlimit.h>

int
main ()
{
  int myargc = 1;
  char *myargv[] = { "./utils/test_io" };

  subprocess_t *p = rlimit_subprocess_create (myargc, myargv, NULL);

  rlimit_subprocess_run (p);

  /* Waiting for the subprocess to start */
  sleep(1);

  /* Writing to the stdin */
  assert (p->stdin);
  rlimit_write_stdin ("42\n", p);

  rlimit_subprocess_wait (p);

  /***** Checking stdout output *****/
  assert(p->stdout);
  assert(!strncmp(rlimit_read_stdout (p), "stdout\n42\n", 10));

  /***** Checking stderr output *****/
  assert(p->stderr);
  assert(!strncmp(rlimit_read_stderr (p), "stderr\n", 7));

  /***** Checking return value and status output *****/
  assert (p->retval == EXIT_SUCCESS);
  assert (p->status == TERMINATED);

  rlimit_subprocess_delete (p);

  return EXIT_SUCCESS;
}
