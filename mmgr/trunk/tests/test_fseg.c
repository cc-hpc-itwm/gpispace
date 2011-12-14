
#include <stdio.h>
#include <stdlib.h>

#ifndef __APPLE__
// malloc.h is deprecated on OSX.
#include <malloc.h>
#else
// malloc_stats() is missing on OSX / FreeBSD / Solaris / ...
void malloc_stats() { }
#endif

#include <mmgr/fseg.h>

static void
ins (PFSeg_t PFSeg, const Key_t Key, const Value_t Value)
{
  fseg_ins (PFSeg, Key, Value);

  printf ("ins %p (" FMT_Key_t "," FMT_Value_t ")\n", *PFSeg, Key, Value);
}

static void
get_atleast_minimal (const FSeg_t FSeg, const Key_t Key)
{
  Key_t Got = Key;
  PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Got);

  printf ("get_atleast %p: " FMT_Key_t ": ", FSeg, Key);

  if (PVal != NULL)
    {
      printf ("Just (*PVal == " FMT_Value_t ", Got = " FMT_Key_t ")", *PVal,
              Got);
    }
  else
    {
      printf ("Nothing");
    }
  printf ("\n");
}

static void
get_atleast (const FSeg_t FSeg, const Key_t Key)
{
  Key_t Got = Key;
  PValue_t PVal = fseg_get_atleast (FSeg, &Got);

  printf ("get_atleast %p: " FMT_Key_t ": ", FSeg, Key);

  if (PVal != NULL)
    {
      printf ("Just (*PVal == " FMT_Value_t ", Got = " FMT_Key_t ")", *PVal,
              Got);
    }
  else
    {
      printf ("Nothing");
    }
  printf ("\n");
}

static void
get (const FSeg_t FSeg, const Key_t Key)
{
  PValue_t PVal = fseg_get (FSeg, Key);

  printf ("get %p: " FMT_Key_t ": ", FSeg, Key);

  if (PVal != NULL)
    {
      printf ("Just (*PVal == " FMT_Value_t ")", *PVal);
    }
  else
    {
      printf ("Nothing");
    }
  printf ("\n");

  get_atleast (FSeg, Key);
  get_atleast_minimal (FSeg, Key);
}

int
main ()
{
  FSeg_t FSeg = NULL;

  get (FSeg, 1 << 9);
  get (FSeg, 1 << 10);
  get (FSeg, 1 << 11);
  get (FSeg, 1 << 12);
  get (FSeg, 1 << 13);

  ins (&FSeg, 1 << 10, 10);
  ins (&FSeg, 1 << 10, 11);
  ins (&FSeg, 1 << 11, 10);
  ins (&FSeg, 1 << 12, 11);

  get (FSeg, 1 << 9);
  get (FSeg, 1 << 10);
  get (FSeg, 1 << 11);
  get (FSeg, 1 << 12);
  get (FSeg, 1 << 13);

  fseg_del (&FSeg, 1 << 10, 10, SMAP_DEL_DEFAULT);

  get (FSeg, 1 << 9);
  get (FSeg, 1 << 10);
  get (FSeg, 1 << 11);
  get (FSeg, 1 << 12);
  get (FSeg, 1 << 13);

  fseg_del (&FSeg, 1 << 10, 10, SMAP_DEL_DEFAULT);

  get (FSeg, 1 << 9);
  get (FSeg, 1 << 10);
  get (FSeg, 1 << 11);
  get (FSeg, 1 << 12);
  get (FSeg, 1 << 13);

  fseg_del (&FSeg, 1 << 10, 11, SMAP_DEL_DEFAULT);

  get (FSeg, 1 << 9);
  get (FSeg, 1 << 10);
  get (FSeg, 1 << 11);
  get (FSeg, 1 << 12);
  get (FSeg, 1 << 13);

  fseg_del (&FSeg, 1 << 11, 10, SMAP_DEL_DEFAULT);
  fseg_del (&FSeg, 1 << 12, 11, SMAP_DEL_DEFAULT);

  get (FSeg, 1 << 9);
  get (FSeg, 1 << 10);
  get (FSeg, 1 << 11);
  get (FSeg, 1 << 12);
  get (FSeg, 1 << 13);

  ins (&FSeg, 1 << 10, 10);
  ins (&FSeg, 1 << 10, 11);
  ins (&FSeg, 1 << 11, 10);
  ins (&FSeg, 1 << 12, 11);

  printf ("size = " FMT_Size_t "\n", fseg_size (FSeg));
  printf ("memused = " FMT_Size_t "\n", fseg_memused (FSeg));

  malloc_stats ();

  fseg_free (&FSeg);

  malloc_stats ();

  return EXIT_SUCCESS;
}
