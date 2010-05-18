
#include <stdio.h>
#include <stdlib.h>

#include <mmgr/tmmgr.h>

#include <sys/time.h>

static inline double
current_time ()
{
  struct timeval tv;

  gettimeofday (&tv, NULL);

  return (double) tv.tv_sec + (double) tv.tv_usec * 1E-6;
}

static Word_t knum = 0;
static unsigned int iProc = 433;        // out of (1 << 20)

static Word_t
gen_key ()
{
  ++knum;

  return (knum << 20) + iProc;
}

int
main (int argc, char **argv)
{
  if (!(argc > 4))
    {
      fprintf (stderr, "usage: %s val size align alloc\n", argv[0]);
      fprintf (stderr,
               " val: r == random, k == gen key, else == sequential\n");
      fprintf (stderr, " size: nonneg\n");
      fprintf (stderr, " align: nonneg\n");
      fprintf (stderr, " alloc: nonneg\n");
      exit (EXIT_FAILURE);
    }

  int use_rand = (argv[1][0] == 'r') ? 1 : 0;
  int use_gen = (argv[1][0] == 'k') ? 1 : 0;
  MemSize_t size = atol (argv[2]);
  Align_t align = atol (argv[3]);
  Count_t alloc = atol (argv[4]);

  Tmmgr_t tmmgr = NULL;

  tmmgr_init (&tmmgr, size, align);

  srand (31415926);
  knum = 0;

  double t = -current_time ();

  for (Count_t i = 0; i < alloc; ++i)
    {
      tmmgr_alloc
        (&tmmgr,
         (Handle_t) (use_rand ? rand () : (use_gen ? gen_key () : i)),
         (size / alloc));
    }

  t += current_time ();

  tmmgr_info (tmmgr, NULL);

  printf (FMT_Size_t " time alloc = %g (%g us per alloc)\n", alloc, t,
          1e6 * t / (double) alloc);

  srand (31415926);
  knum = 0;

  t = -current_time ();

  for (Count_t i = 0; i < alloc; ++i)
    {
      tmmgr_free
        (&tmmgr,
         (Handle_t) (use_rand ? rand () : (use_gen ? gen_key () : i)));
    }

  t += current_time ();

  tmmgr_info (tmmgr, NULL);

  printf (FMT_Size_t " time free = %g (%g us per free)\n", alloc, t,
          1e6 * t / (double) alloc);

  srand (31415926);
  knum = 0;

  for (Count_t i = 0; i < alloc; ++i)
    {
      tmmgr_alloc
        (&tmmgr,
         (Handle_t) (use_rand ? rand () : (use_gen ? gen_key () : i)),
         (size / alloc));
    }

  srand (31415926);
  knum = 0;

  for (Count_t i = 0; i < alloc / 2; ++i)
    {
      tmmgr_free
        (&tmmgr,
         (Handle_t) (use_rand ? rand () : (use_gen ? gen_key () : i)));
    }

  tmmgr_info (tmmgr, NULL);

  t = -current_time ();

  tmmgr_defrag (&tmmgr, NULL, NULL, NULL);

  t += current_time ();

  tmmgr_info (tmmgr, NULL);

  printf (FMT_Size_t " time defrag = %g\n", alloc, t);

  tmmgr_finalize (&tmmgr);

  return EXIT_SUCCESS;
}
