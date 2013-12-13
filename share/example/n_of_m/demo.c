
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char ** argv)
{
  const int n = (argc > 1) ? atoi (argv[1]) : -1;
  const int m = (argc > 2) ? atoi (argv[2]) : -1;

  printf ("RUN %i of %i\n", n, m);

  sleep (1);

  return EXIT_SUCCESS;
}
