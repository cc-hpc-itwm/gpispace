
#include <tmmgr.h>

#include <stdio.h>
#include <stdlib.h>

#include <malloc.h>

static void
do_alloc (PTmmgr_t ptmmgr, Handle_t Handle, Size_t Size)
{
  AllocReturn_t AllocReturn = tmmgr_alloc (ptmmgr, Handle, Size);

  printf ("ALLOC: (Handle = " FMT_Handle_t ", Size = " FMT_Size_t ") => ",
          Handle, Size);

  switch (AllocReturn)
    {
    case ALLOC_SUCCESS:
      printf ("Success");
      {
        Offset_t Offset;

        tmmgr_offset_size (ptmmgr, Handle, &Offset, NULL);

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
    default:
      fprintf (stderr, "STRANGE\n");
      exit (EXIT_FAILURE);
      break;
    }

  printf ("\n");
}

static void
do_free (PTmmgr_t ptmmgr, Handle_t Handle)
{
  printf ("FREE: (Handle = " FMT_Handle_t ") => ", Handle);

  switch (tmmgr_free (ptmmgr, Handle))
    {
    case RET_SUCCESS:
      printf ("RET_SUCCESS");
      break;
    case RET_HANDLE_UNKNOWN:
      printf ("RET_HANDLE_UNKNOWN");
      break;
    default:
      fprintf (stderr, "STRANGE\n");
      exit (EXIT_FAILURE);
      break;
    }

  printf ("\n");
}

static void
do_resize (PTmmgr_t ptmmgr, MemSize_t NewSize)
{
  printf ("RESIZE: NewSize = " FMT_MemSize_t " => ", NewSize);

  switch (tmmgr_resize (ptmmgr, NewSize))
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
  PTmmgr_t ptmmgr = tmmgr_init (45, 1);

  tmmgr_info (ptmmgr);

  for (Word_t i = 0; i < 10; ++i)
    do_alloc (ptmmgr, i, 1);

  tmmgr_info (ptmmgr);

  do_free (ptmmgr, 2);
  tmmgr_info (ptmmgr);
  do_free (ptmmgr, 6);
  tmmgr_info (ptmmgr);
  do_free (ptmmgr, 3);
  tmmgr_info (ptmmgr);
  do_free (ptmmgr, 7);
  tmmgr_info (ptmmgr);
  do_free (ptmmgr, 8);
  tmmgr_info (ptmmgr);
  do_free (ptmmgr, 1);
  tmmgr_info (ptmmgr);
  do_free (ptmmgr, 5);
  tmmgr_info (ptmmgr);
  do_free (ptmmgr, 4);
  tmmgr_info (ptmmgr);

  do_alloc (ptmmgr, 1, 1);
  tmmgr_info (ptmmgr);

  do_alloc (ptmmgr, 11, 4);
  do_alloc (ptmmgr, 12, 4);
  do_alloc (ptmmgr, 11, 4);
  do_alloc (ptmmgr, 12, 4);
  do_alloc (ptmmgr, 13, 35);
  do_alloc (ptmmgr, 13, 34);

  do_free (ptmmgr, 13);

  tmmgr_info (ptmmgr);

  /* complete defragmentation */
  tmmgr_defrag (ptmmgr, &fMemmove, NULL);

  tmmgr_info (ptmmgr);

  do_alloc (ptmmgr, 26, 5);
  do_alloc (ptmmgr, 27, 5);
  do_alloc (ptmmgr, 28, 5);
  do_alloc (ptmmgr, 29, 5);
  do_alloc (ptmmgr, 30, 5);

  tmmgr_info (ptmmgr);

  do_free (ptmmgr, 26);
  do_free (ptmmgr, 28);
  do_free (ptmmgr, 29);

  tmmgr_info (ptmmgr);

  /* only up to a free chunk of size 12 */
  MemSize_t FreeSizeWanted = 12;
  tmmgr_defrag (ptmmgr, &fMemmove, &FreeSizeWanted);

  tmmgr_info (ptmmgr);

  tmmgr_resize (ptmmgr, 50);

  tmmgr_info (ptmmgr);

  do_alloc (ptmmgr, 101, 14);

  tmmgr_info (ptmmgr);

  do_resize (ptmmgr, 45);

  tmmgr_defrag (ptmmgr, &fMemmove, NULL);

  tmmgr_info (ptmmgr);

  do_resize (ptmmgr, 34);
  do_resize (ptmmgr, 35);

  tmmgr_info (ptmmgr);

  do_resize (ptmmgr, 50);

  tmmgr_info (ptmmgr);

  do_resize (ptmmgr, 45);

  tmmgr_info (ptmmgr);

  do_free (ptmmgr, 0);
  do_free (ptmmgr, 1);
  do_free (ptmmgr, 9);
  do_free (ptmmgr, 11);
  do_free (ptmmgr, 12);
  do_free (ptmmgr, 27);
  do_free (ptmmgr, 30);
  do_free (ptmmgr, 101);

  tmmgr_info (ptmmgr);

  do_resize (ptmmgr, 0);

  tmmgr_info (ptmmgr);

  do_resize (ptmmgr, 3141);

  tmmgr_info (ptmmgr);

  printf ("\n");

  Size_t Bytes = tmmgr_finalize (ptmmgr);

  printf ("Bytes = " FMT_Size_t "\n", Bytes);

  malloc_stats ();

  return EXIT_SUCCESS;
}
