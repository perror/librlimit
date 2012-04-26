#include <stdio.h>
#include <stdlib.h>

int
main ()
{
  char str[3];

  /* Test on stdout output */
  fprintf (stdout, "stdout\n");
  fflush (stdout);

  /* Test on stderr output */
  fprintf (stderr, "stderr\n");
  fflush (stderr);

  /* Test on stdin input */
  int ret = fgets (str, 3, stdin);
  fprintf (stdout, "%s\n", str);
  fflush (stdout);

  return EXIT_SUCCESS;
}
