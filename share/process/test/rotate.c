
#include <stdio.h>
#include <stdlib.h>

FILE * do_open (const char * filename, const char * mode)
{
  FILE * f = fopen (filename, mode);

  if (!f)
    {
      fprintf (stderr, "could not open %s\n", filename);

      exit (EXIT_FAILURE);
    }

  return f;
}

void do_copy (FILE * in, FILE * out)
{
  int a = fgetc (in);

  while (a != EOF)
    {
      fprintf (out, "%c", a);

      a = fgetc (in);
    }
}

int
main (int argc, char ** argv)
{
  if (argc < 5)
    {
      fprintf (stderr, "usage: %s in1 in2 out1 out2\n", argv[0]);

      exit (EXIT_FAILURE);
    }

  FILE * in[3];
  FILE * out[3];

  in[0] = stdin;
  in[1] = do_open (argv[1], "r");
  in[2] = do_open (argv[2], "r");

  out[0] = do_open (argv[3], "w");
  out[1] = do_open (argv[4], "w");
  out[2] = stdout;

  for (int i = 0; i < 3; ++i)
    {
      do_copy (in[i], out[i]);

      fclose (in[i]);
      fclose (out[i]);
    }

  fprintf (stderr, "23 bytes sent to stderr");

  return EXIT_SUCCESS;
}
