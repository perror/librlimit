#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ARRAY_SIZE 1000000

int
main ()
{
  int array[ARRAY_SIZE];

  for (int i = 0; i < ARRAY_SIZE; i++)
    array[i] = 2 << i;

  fprintf (stdout, "array[0] = %d\n", array[0]);

  return EXIT_SUCCESS;
}
