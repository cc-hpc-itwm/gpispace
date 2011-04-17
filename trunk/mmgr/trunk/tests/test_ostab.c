
#ifndef __APPLE__
// malloc.h is deprecated on OSX.
#include <malloc.h>
#else
// malloc_stats() is missing on OSX / FreeBSD / Solaris / ...
void malloc_stats() { }
#endif

#include <stdio.h>
#include <stdlib.h>

#include <mmgr/ostab.h>
#include <mmgr/unused.h>

static void
try_look (OStab_t ostab, const Key_t Key)
{
  printf (FMT_Key_t " => ", Key);

  Offset_t offset;
  Size_t size;

  if (ostab_get (ostab, Key, &offset, &size) == True)
    {
      printf ("Just (" FMT_Offset_t "," FMT_Size_t ")\n", offset, size);
    }
  else
    {
      printf ("Nothing\n");
    }
}

static void
fPrint (const Key_t Key, const Offset_t Offset, const Size_t Size,
        void UNUSED (*Pdat))
{
  printf (" " FMT_Key_t "-(" FMT_Offset_t "," FMT_Size_t ")", Key, Offset,
          Size);
}


static void
print (const OStab_t ostab)
{
  printf ("elems = [");

  ostab_work (ostab, &fPrint, NULL);

  printf ("]\n");
}

int
main ()
{
  OStab_t ostab = NULL;

  malloc_stats ();

  printf ("Bytes = " FMT_Size_t "\n", ostab_memused (ostab));

  print (ostab);

  ostab_ins (&ostab, 23, 0xfff, 1 << 20);
  ostab_ins (&ostab, 42, 0xaff, 1 << 12);

  printf ("Bytes = " FMT_Size_t "\n", ostab_memused (ostab));

  print (ostab);

  try_look (ostab, 23);
  try_look (ostab, 42);
  try_look (ostab, 99);

  ostab_del (&ostab, 99);
  ostab_del (&ostab, 23);

  printf ("Bytes = " FMT_Size_t "\n", ostab_memused (ostab));

  try_look (ostab, 23);
  try_look (ostab, 42);
  try_look (ostab, 99);

  printf ("Bytes = " FMT_Size_t "\n", ostab_free (&ostab));

  try_look (ostab, 23);
  try_look (ostab, 42);
  try_look (ostab, 99);

  srand (31415926);

  for (Word_t i = 0; i < (1 << 20); ++i)
    {
      ostab_ins (&ostab, (Key_t) i, (Offset_t) i, (Size_t) i);
    }

  printf ("Bytes = " FMT_Size_t "\n", ostab_free (&ostab));

  srand (31415926);

  for (Word_t i = 0; i < (1 << 20); ++i)
    {
      ostab_ins (&ostab, (Key_t) i, (Offset_t) i, (Size_t) i);
    }

  srand (31415926);

  for (Word_t i = 0; i < (1 << 20); ++i)
    {
      ostab_del (&ostab, (Key_t) i);
    }

  printf ("Bytes = " FMT_Size_t "\n", ostab_free (&ostab));

  malloc_stats ();

  return EXIT_SUCCESS;
}
