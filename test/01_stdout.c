#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <rlimit.h>

int
main ()
{
  int myargc = 2;
  char *myargv[] = { "/bin/ls", "/" };
  
  subprocess_t *p = rlimit_subprocess_create (myargc, myargv, NULL);

  rlimit_subprocess_run (p);
  rlimit_subprocess_wait (p);

    /* Printing the i/o */
  char buff[1024];

  /* Check that stdout is non-empty */
  if (p->stdout)
    {
      if (!fgets (buff, 100, p->stdout))
	{
	  assert (false);
	}
    }

  /* Check that stderr is empty */
  if (p->stderr)
    {
      if (fgets (buff, 100, p->stderr))
	{
	  assert (false);
	}
    }

  /* Check that retval is '0' */
  assert (p->retval == 0);
  
  rlimit_subprocess_delete (p);

  return EXIT_SUCCESS;
}
