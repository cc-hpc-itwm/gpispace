
#include <mmgr/dtmmgr.h>

#include <stdio.h>
#include <stdlib.h>

#ifndef __APPLE__
// malloc.h is deprecated on OSX.
#include <malloc.h>
#else
// malloc_stats() is missing on OSX / FreeBSD / Solaris / ...
void malloc_stats() { }
#endif

static const char *showArena[2] = { "ARENA_UP", "ARENA_DOWN" };
static const Arena_t Other[2] = { ARENA_DOWN, ARENA_UP };

static void
do_alloc (DTmmgr_t DTmmgr, Handle_t Handle, Arena_t Arena, Size_t Size)
{
  if (DTmmgr == NULL)
    return;

  AllocReturn_t AllocReturn = dtmmgr_alloc (&DTmmgr, Handle, Arena, Size);

  printf ("ALLOC: (Handle = " FMT_Handle_t ", Arena = %s, Size = " FMT_Size_t
          ") => ", Handle, showArena[Arena], Size);

  switch (AllocReturn)
    {
    case ALLOC_SUCCESS:
      printf ("Success");
      {
        Offset_t Offset = -1;

        dtmmgr_offset_size (DTmmgr, Handle, Arena, &Offset, NULL);

        printf (", Offset = " FMT_Offset_t, Offset);
      }
      break;
    case ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY:
      printf ("INSUFFICIENT_CONTIGUOUS_MEMORY");
      break;
    case ALLOC_DUPLICATE_HANDLE:
      printf ("ALLOC_DUPLICATE_HANDLE");
      break;
    case ALLOC_INSUFFICIENT_MEMORY:
      printf ("ALLOC_INSUFFICIENT_MEMORY");
      break;
    case ALLOC_FAILURE:
      printf ("ALLOC_FAILURE");
      break;
    default:
      fprintf (stderr, "STRANGE\n");
      exit (EXIT_FAILURE);
      break;
    }

  printf ("\n");
}

static void
do_free (DTmmgr_t DTmmgr, Handle_t Handle, Arena_t Arena)
{
  if (DTmmgr == NULL)
    return;

  printf ("FREE: (Handle = " FMT_Handle_t ", Arena = %s) => ", Handle,
          showArena[Arena]);

  switch (dtmmgr_free (&DTmmgr, Handle, Arena))
    {
    case RET_SUCCESS:
      printf ("RET_SUCCESS");
      break;
    case RET_HANDLE_UNKNOWN:
      printf ("RET_HANDLE_UNKNOWN");
      break;
    case RET_FAILURE:
      printf ("RET_FAILURE");
      break;
    default:
      fprintf (stderr, "STRANGE\n");
      exit (EXIT_FAILURE);
      break;
    }

  printf ("\n");
}

/*
static void
do_resize (PTmmgr_t PTmmgr, MemSize_t NewSize)
{
  if (PTmmgr == NULL)
    return;

  printf ("RESIZE: NewSize = " FMT_MemSize_t " => ", NewSize);

  switch (tmmgr_resize (&PTmmgr, NewSize))
    {
    case RESIZE_SUCCESS:
      printf ("RESIZE_SUCCESS");
      break;
    case RESIZE_BELOW_HIGHWATER:
      printf ("RESIZE_BELOW_HIGHWATER");
      break;
    case RESIZE_BELOW_MEMUSED:
      printf ("RESIZE_BELOW_MEMUSED");
      break;
    case RESIZE_FAILURE:
      printf ("RESIZE_FAILURE");
      break;
    default:
      fprintf (stderr, "STRANGE\n");
      exit (EXIT_FAILURE);
      break;
    }

  printf ("\n");
}

*/

static void
fMemmove (const OffsetDest_t OffsetDest, const OffsetSrc_t OffsetSrc,
          const MemSize_t Size, void *PDat)
{
  printf ("CALLBACK-%lu: Moving " FMT_MemSize_t " Byte(s) from " FMT_Offset_t
          " to " FMT_Offset_t "\n", (*(unsigned long *) PDat)++, Size,
          OffsetSrc, OffsetDest);
}

int
main ()
{
  DTmmgr_t dtmmgr = NULL;

  dtmmgr_init (&dtmmgr, 45, 2);

  dtmmgr_info (dtmmgr);

  Arena_t Arena = ARENA_UP;

  for (Word_t i = 0; i < 10; ++i)
    {
      do_alloc (dtmmgr, (Handle_t) i, Arena, (MemSize_t) 1);

      Arena = Other[Arena];
    }

  dtmmgr_info (dtmmgr);

  do_free (dtmmgr, 2, ARENA_UP);
  do_free (dtmmgr, 6, ARENA_UP);
  do_free (dtmmgr, 3, ARENA_DOWN);
  do_free (dtmmgr, 7, ARENA_DOWN);
  do_free (dtmmgr, 8, ARENA_UP);

  dtmmgr_status (dtmmgr);

  unsigned long callback_count = 0;

  dtmmgr_defrag (&dtmmgr, ARENA_UP, &fMemmove, NULL, &callback_count);
  dtmmgr_defrag (&dtmmgr, ARENA_DOWN, &fMemmove, NULL, &callback_count);

  dtmmgr_status (dtmmgr);

  {
    printf ("%p %p\n", &dtmmgr, dtmmgr);

    Size_t Bytes = dtmmgr_finalize (&dtmmgr);

    printf ("Bytes = " FMT_Size_t "\n", Bytes);
  }

  {
    printf ("%p %p\n", &dtmmgr, dtmmgr);

    Size_t Bytes = dtmmgr_finalize (&dtmmgr);

    printf ("Bytes = " FMT_Size_t "\n", Bytes);
  }

  malloc_stats ();

  return EXIT_SUCCESS;
}
