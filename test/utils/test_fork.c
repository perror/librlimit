#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>

int
main ()
{
  sleep(2);
  switch (fork ())
    {
    case -1:
      fprintf (stderr, "forking: error: fork failed\n");
      exit (EXIT_FAILURE);
      break;

    case 0:
      fprintf (stdout, "I am the child process (fork succeeded)\n");
      break;

    default:
      fprintf (stdout, "I am the parent process (fork succeeded)\n");
    }

  return EXIT_SUCCESS;
}
