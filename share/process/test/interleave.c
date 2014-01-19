
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

  FILE * file1 = fopen (argv[1], "r");

  if (!file1)
    {
      fprintf (stderr, "could not open %s\n", argv[1]);

      exit (EXIT_FAILURE);
    }

  FILE * file2 = fopen (argv[2], "r");

  if (!file2)
    {
      fprintf (stderr, "could not open %s\n", argv[2]);

      exit (EXIT_FAILURE);
    }

  int a, b;

 STEP:

  a = fgetc (file1);
  b = fgetc (file2);

  if (a == EOF)
    {
      goto DONE;
    }

  if (b == EOF)
    {
      goto DONE;
    }

  printf ("%c%c", a, b);

  goto STEP;

 DONE:

  fclose (file1);
  fclose (file2);

  return EXIT_SUCCESS;
}
