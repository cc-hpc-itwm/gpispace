
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char ** argv)
{
  if (argc < 3)
    {
      fprintf (stderr, "usage: %s file1 file2\n", argv[0]);

      exit (EXIT_FAILURE);
    }

  FILE * file[2];

  file[0] = fopen (argv[1], "w");

  if (!file[0])
    {
      fprintf (stderr, "could not open %s\n", argv[1]);

      exit (EXIT_FAILURE);
    }

  file[1] = fopen (argv[2], "w");

  if (!file[1])
    {
      fprintf (stderr, "could not open %s\n", argv[2]);

      exit (EXIT_FAILURE);
    }

  int which = 0;
  int c = getchar();

  while (c != EOF)
    {
      fprintf (file[which], "%c", c);

      which = 1 - which;

      c = getchar();
    }

  fclose (file[0]);
  fclose (file[1]);

  return EXIT_SUCCESS;
}
