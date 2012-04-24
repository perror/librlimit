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

  rlimit_set_time_limit (3, p);
  rlimit_subprocess_run (p);

  /* Waiting for the subprocess to start */
  sleep(1);

  /* Writing to the stdin */
  assert (p->stdin);
  assert(fputs ("42\n", p->stdin) != EOF);
  fflush (p->stdin);

  rlimit_subprocess_wait (p);

  /***** Checking stdout output *****/
  char stdout_buff[100];

  /* Check that stdout is non-empty */
  if (p->stdout)
    assert (fgets (stdout_buff, 100, p->stdout));

  assert(!strncmp(stdout_buff, "stdout", 6));

  if (p->stdout)
    assert (fgets (stdout_buff, 100, p->stdout));

  assert(!strncmp(stdout_buff, "42", 2));

  /***** Checking stderr output *****/
  char stderr_buff[100];

  /* Check that stderr is empty */
  if (p->stderr)
    assert (fgets (stderr_buff, 100, p->stderr));

  assert(!strncmp(stderr_buff, "stderr", 6));

  /***** Checking return value and status output *****/
  assert (p->retval == EXIT_SUCCESS);
  assert (p->status == TERMINATED);

  rlimit_subprocess_delete (p);

  return EXIT_SUCCESS;
}
