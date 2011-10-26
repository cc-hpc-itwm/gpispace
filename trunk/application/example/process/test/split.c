
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char ** argv)
{
  if (argc < 2)
    {
      exit (EXIT_FAILURE);
    }

  FILE * file[2];

  file[0] = fopen (argv[1], "w");

  if (!file[0])
    {
      exit (EXIT_FAILURE);
    }

  file[1] = fopen (argv[2], "w");

  if (!file[1])
    {
      exit (EXIT_FAILURE);
    }

  int which = 0;
  int c = getchar();

  while (c != EOF)
    {
      fprintf (file[which], "%c", c);

      which = 1 - which;
    }

  close (file[0]);
  close (file[1]);

  return EXIT_SUCCESS;
}
