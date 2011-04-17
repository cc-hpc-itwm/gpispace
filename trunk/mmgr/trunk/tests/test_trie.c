
#ifndef __APPLE__
// malloc.h is deprecated on OSX.
#include <malloc.h>
#else
// malloc_stats() is missing on OSX / FreeBSD / Solaris / ...
void malloc_stats() { }
#endif

#include <stdio.h>
#include <stdlib.h>

#include <mmgr/trie.h>
#include <mmgr/unused.h>

static void
getany (const TrieMap_t tm)
{
  const PValue_t PVal = trie_getany (tm);

  printf ("getany: tm = %p, PVal = %p", tm, PVal);

  if (PVal != NULL)
    {
      printf (", Just (*Pval = " FMT_Value_t ")\n", *PVal);
    }
  else
    {
      printf (", Nothing\n");
    }

}

static void
get (const TrieMap_t tm, const Key_t key)
{
  const PValue_t PVal = trie_get (tm, key);

  printf ("get " FMT_Key_t ": tm = %p, PVal = %p", key, tm, PVal);

  if (PVal != NULL)
    {
      printf (", Just (*Pval = " FMT_Value_t ")\n", *PVal);
    }
  else
    {
      printf (", Nothing\n");
    }

  getany (tm);
}

static void
ins (PTrieMap_t tm, const Key_t key, const Value_t val)
{
  Bool_t was_there;
  const PValue_t PVal = trie_ins (tm, key, &was_there);

  *PVal = val;

  printf ("ins (" FMT_Key_t "," FMT_Value_t
          "): tm = %p, PVal = %p, was_there = %s\n", key, val, tm, PVal,
          (was_there == True) ? "True" : "False");
}

static void
del (PTrieMap_t tm, const Value_t key)
{
  trie_del (tm, key, fUserNone);

  printf ("del " FMT_Key_t ": tm = %p\n", key, tm);
}

static void
fPrint (const Key_t Key, const PValue_t PVal, void UNUSED (*Pdat))
{
  printf (" " FMT_Key_t "-" FMT_Value_t, Key, *PVal);
}

static void
print (const TrieMap_t tm)
{
  printf ("elems = [");

  trie_work (tm, &fPrint, NULL);

  printf ("]\n");
}

int
main ()
{
  TrieMap_t tm = NULL;

  malloc_stats ();

  printf ("tm = %p\n", tm);

  get (tm, 23);
  ins (&tm, 23, 23);
  ins (&tm, 23, 23);
  ins (&tm, 23, 23);
  get (tm, 23);
  del (&tm, 23);
  get (tm, 23);
  del (&tm, 23);
  get (tm, 23);
  ins (&tm, 23, 23);
  ins (&tm, 99, 99);
  get (tm, 23);
  get (tm, 99);
  get (tm, 44);
  del (&tm, 44);
  get (tm, 23);
  get (tm, 99);
  get (tm, 44);
  del (&tm, 23);
  get (tm, 23);
  get (tm, 99);
  get (tm, 44);
  del (&tm, 99);
  get (tm, 23);
  get (tm, 99);
  get (tm, 44);
  ins (&tm, 23, 23);
  ins (&tm, 99, 99);
  ins (&tm, 44, 44);
  ins (&tm, 13, 13);

  Size_t Bytes = trie_memused (tm, fUserNone);

  printf ("tm = %p, Bytes = " FMT_Size_t "\n", tm, Bytes);

  print (tm);

  Bytes = trie_free (&tm, fUserNone);

  printf ("tm = %p, Bytes = " FMT_Size_t "\n", tm, Bytes);

  print (tm);

  srand (31415926);

  Word_t dups = 0;

  for (Size_t i = 0; i < (1 << 22); ++i)
    {
      Bool_t was_there;

      trie_ins (&tm, rand (), &was_there);

      dups += (was_there == True) ? 1 : 0;
    }

  Size_t Size = trie_size (tm);
  Bytes = trie_memused (tm, fUserNone);

  printf ("dups = " FMT_Word_t ", size = " FMT_Size_t ", memused = "
          FMT_Size_t "\n", dups, Size, Bytes);

  malloc_stats ();

  Bytes = trie_free (&tm, fUserNone);

  printf ("tm = %p, Bytes = " FMT_Size_t "\n", tm, Bytes);

  malloc_stats ();

  return EXIT_SUCCESS;
}
