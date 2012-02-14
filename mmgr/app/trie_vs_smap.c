
#include <stdio.h>
#include <stdlib.h>

#include <mmgr/trie.h>
#include <mmgr/smap.h>
#include <mmgr/unused.h>

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
  if (!(argc > 3))
    {
      fprintf (stderr, "usage: %s struct val size\n", argv[0]);
      fprintf (stderr, " struct: i == Trie, else == Tree\n");
      fprintf (stderr,
               " val: r == random, k == gen key, else == sequential\n");
      fprintf (stderr, " size: nonneg\n");
      exit (EXIT_FAILURE);
    }

  int use_trie = (argv[1][0] == 'i') ? 1 : 0;
  int use_rand = (argv[2][0] == 'r') ? 1 : 0;
  int use_gen = (argv[2][0] == 'k') ? 1 : 0;
  Size_t size = atol (argv[3]);

  SMap_t sm = NULL;
  TrieMap_t tm = NULL;
  Word_t dups = 0;

  srand (31415926);

  double t = -current_time ();

  if (use_trie)
    {
      for (Size_t i = 0; i < size; ++i)
        {
          Bool_t was_there;

          trie_ins (&tm, use_rand ? rand () : (use_gen ? gen_key () : i),
                    &was_there);

          dups += (was_there == True) ? 1 : 0;
        }
    }
  else
    {
      for (Size_t i = 0; i < size; ++i)
        {
          Bool_t was_there =
            smap_ins (&sm, use_rand ? rand () : (use_gen ? gen_key () : i),
                      i);

          dups += (was_there == True) ? 1 : 0;
        }
    }

  t += current_time ();

  printf (FMT_Size_t " time insert = %g (%g us per ins)\n", size, t,
          1e6 * t / (double) size);

  t = -current_time ();

  const Size_t Size = use_trie ? trie_size (tm) : smap_size (sm);

  t += current_time ();

  printf (FMT_Size_t " time size = %g (%g us per elem)\n", size, t,
          1e6 * t / (double) Size);

  t = -current_time ();

  int rep = 1000;

  for (int k = 0; k < rep; ++k)
    {
      srand (31415926);

      if (use_trie)
        {
          for (Size_t i = 0; i < size; ++i)
            {
              trie_get (tm, use_rand ? rand () : (use_gen ? gen_key () : i));
            }
        }
      else
        {
          for (Size_t i = 0; i < size; ++i)
            {
              smap_get (sm, use_rand ? rand () : (use_gen ? gen_key () : i));
            }
        }
    }

  t += current_time ();

  printf (FMT_Size_t " time lookup = %g (%g ns per elem)\n", size, t,
          1e9 * t / ((double) rep * (double) Size));

  t = -current_time ();

  const Size_t Bytes =
    use_trie ? trie_free (&tm, fUserNone) : smap_free (&sm);

  t += current_time ();

  printf (FMT_Size_t " time free = %g (%g us per elem)\n", size, t,
          1e6 * t / (double) Size);

  printf (FMT_Size_t " dups = " FMT_Word_t "\n", size, dups);
  printf (FMT_Size_t " size = " FMT_Size_t "\n", size, Size);
  printf (FMT_Size_t " bytes = " FMT_Size_t " (%g bytes per elem)\n", size,
          Bytes, (double) Bytes / (double) Size);

  return EXIT_SUCCESS;
}
