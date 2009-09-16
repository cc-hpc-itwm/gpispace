
#include <tmmgr.h>

#include <stdio.h>
#include <stdlib.h>

#include <malloc.h>

static void
do_alloc (PTmmgr_t PTmmgr, Handle_t Handle, Size_t Size)
{
  if (PTmmgr == NULL)
    return;

  AllocReturn_t AllocReturn = tmmgr_alloc (&PTmmgr, Handle, Size);

  printf ("ALLOC: (Handle = " FMT_Handle_t ", Size = " FMT_Size_t ") => ",
          Handle, Size);

  switch (AllocReturn)
    {
    case ALLOC_SUCCESS:
      printf ("Success");
      {
        Offset_t Offset;

        tmmgr_offset_size (PTmmgr, Handle, &Offset, NULL);

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
do_free (PTmmgr_t PTmmgr, Handle_t Handle)
{
  if (PTmmgr == NULL)
    return;

  printf ("FREE: (Handle = " FMT_Handle_t ") => ", Handle);

  switch (tmmgr_free (&PTmmgr, Handle))
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

static void
fMemmove (const OffsetDest_t OffsetDest, const OffsetSrc_t OffsetSrc,
          const MemSize_t Size)
{
  printf ("CALLBACK: Moving %lu Byte(s) from %lu to %lu\n", Size, OffsetSrc,
          OffsetDest);
}

int
main ()
{
  Tmmgr_t tmmgr = (Tmmgr_t) NULL;

  tmmgr_init (&tmmgr, 45, 1);

  tmmgr_info (tmmgr);

  for (Word_t i = 0; i < 10; ++i)
    do_alloc (tmmgr, (Handle_t) i, (MemSize_t) 1);

  tmmgr_info (tmmgr);

  printf ("beep\n");

  do_free (tmmgr, 2);
  tmmgr_info (tmmgr);
  do_free (tmmgr, 6);
  tmmgr_info (tmmgr);
  do_free (tmmgr, 3);
  tmmgr_info (tmmgr);
  do_free (tmmgr, 7);
  tmmgr_info (tmmgr);
  do_free (tmmgr, 8);
  tmmgr_info (tmmgr);
  do_free (tmmgr, 1);
  tmmgr_info (tmmgr);
  do_free (tmmgr, 5);
  tmmgr_info (tmmgr);
  do_free (tmmgr, 4);
  tmmgr_info (tmmgr);

  do_alloc (tmmgr, 1, 1);
  tmmgr_info (tmmgr);

  do_alloc (tmmgr, 11, 4);
  do_alloc (tmmgr, 12, 4);
  do_alloc (tmmgr, 11, 4);
  do_alloc (tmmgr, 12, 4);
  do_alloc (tmmgr, 13, 35);
  do_alloc (tmmgr, 13, 34);

  do_free (tmmgr, 13);

  tmmgr_info (tmmgr);

  /* complete defragmentation */
  tmmgr_defrag (&tmmgr, &fMemmove, NULL);

  tmmgr_info (tmmgr);

  do_alloc (tmmgr, 26, 5);
  do_alloc (tmmgr, 27, 5);
  do_alloc (tmmgr, 28, 5);
  do_alloc (tmmgr, 29, 5);
  do_alloc (tmmgr, 30, 5);

  tmmgr_info (tmmgr);

  do_free (tmmgr, 26);
  do_free (tmmgr, 28);
  do_free (tmmgr, 29);

  tmmgr_info (tmmgr);

  /* only up to a free chunk of size 12 */
  MemSize_t FreeSizeWanted = 12;
  tmmgr_defrag (&tmmgr, &fMemmove, &FreeSizeWanted);

  tmmgr_info (tmmgr);

  do_resize (tmmgr, 50);

  tmmgr_info (tmmgr);

  do_alloc (tmmgr, 101, 14);

  tmmgr_info (tmmgr);

  do_resize (tmmgr, 45);

  tmmgr_defrag (&tmmgr, &fMemmove, NULL);

  tmmgr_info (tmmgr);

  do_resize (tmmgr, 34);
  do_resize (tmmgr, 35);

  tmmgr_info (tmmgr);

  do_resize (tmmgr, 50);

  tmmgr_info (tmmgr);

  do_resize (tmmgr, 45);

  tmmgr_info (tmmgr);

  do_free (tmmgr, 0);
  do_free (tmmgr, 1);
  do_free (tmmgr, 9);
  do_free (tmmgr, 11);
  do_free (tmmgr, 12);
  do_free (tmmgr, 27);
  do_free (tmmgr, 30);
  do_free (tmmgr, 101);

  tmmgr_info (tmmgr);

  do_resize (tmmgr, 0);

  tmmgr_info (tmmgr);

  do_resize (tmmgr, 3141);

  tmmgr_info (tmmgr);

  printf ("\n");

  {
    printf ("%p %p\n", &tmmgr, tmmgr);

    Size_t Bytes = tmmgr_finalize (&tmmgr);

    printf ("Bytes = " FMT_Size_t "\n", Bytes);
  }

  {
    printf ("%p %p\n", &tmmgr, tmmgr);

    Size_t Bytes = tmmgr_finalize (&tmmgr);

    printf ("Bytes = " FMT_Size_t "\n", Bytes);
  }

  malloc_stats ();

  Tmmgr_t tmmgrAligned = (Tmmgr_t) NULL;

  tmmgr_init (&tmmgrAligned, 45, (1 << 4));

  tmmgr_info (tmmgrAligned);

  do_resize (tmmgrAligned, 67);
  tmmgr_info (tmmgrAligned);

  do_resize (tmmgrAligned, 64);
  tmmgr_info (tmmgrAligned);

  for (Word_t i = 0; i < 10; ++i)
    do_alloc (tmmgrAligned, (Handle_t) i, (MemSize_t) 1);

  tmmgr_info (tmmgrAligned);

  do_resize (tmmgrAligned, 1000);

  for (Word_t i = 0; i < 10; ++i)
    do_alloc (tmmgrAligned, (Handle_t) i, (MemSize_t) 1);

  tmmgr_info (tmmgrAligned);

  do_resize (tmmgrAligned, 150);

  do_free (tmmgrAligned, 0);

  do_resize (tmmgrAligned, 150);

  tmmgr_defrag (&tmmgrAligned, &fMemmove, NULL);

  do_resize (tmmgrAligned, 150);

  tmmgr_info (tmmgrAligned);

  tmmgr_finalize (&tmmgrAligned);

  malloc_stats();

  return EXIT_SUCCESS;
}
