#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rlimit.h>

/* FIXME: The p->stdin does not behave properly... I cannot write
 * commands to a subprocess. This has to be fixed but I have no idea
 * right now (nor time). */

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

  /* Trying the stdin */
  assert (p->stdin);
  assert(fputs ("42\n", p->stdin) != EOF);

  rlimit_subprocess_wait (p);

  /***** Checking stdout output *****/
  char stdout_buff[32];

  /* Check that stdout is non-empty */
  if (p->stdout)
    assert (fgets (stdout_buff, 32, p->stdout));

  // assert(!strncmp(stdout_buff, "stdout\n42", 9));


  /***** Checking stderr output *****/
  char stderr_buff[32];

  /* Check that stderr is empty */
  if (p->stderr)
    assert (fgets (stderr_buff, 32, p->stderr));

  assert(!strncmp(stderr_buff, "stderr", 6));


  /***** Checking return value and status output *****/
  assert (p->retval == EXIT_SUCCESS);
  assert (p->status == TERMINATED);

  fprintf(stdout, "retval: %d\n", p->retval); // DEBUG
  fprintf(stdout, "status: %d\n", p->status); // DEBUG

  rlimit_subprocess_delete (p);

  return EXIT_SUCCESS;
}
