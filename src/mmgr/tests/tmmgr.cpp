// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE mmgr_tmmgr
#include <boost/test/unit_test.hpp>

#include <mmgr/tmmgr.h>

#include <stdio.h>
#include <stdlib.h>

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
          const MemSize_t Size, void *PDat)
{
  unsigned long* count (static_cast<unsigned long*> (PDat));

  switch (*count)
  {
  case 0:
    BOOST_REQUIRE_EQUAL (OffsetDest, 6);
    BOOST_REQUIRE_EQUAL (OffsetSrc, 9);
    BOOST_REQUIRE_EQUAL (Size, 1);
    break;
  case 1:
    BOOST_REQUIRE_EQUAL (OffsetDest, 7);
    BOOST_REQUIRE_EQUAL (OffsetSrc, 10);
    BOOST_REQUIRE_EQUAL (Size, 4);
    break;

  case 2:
    BOOST_REQUIRE_EQUAL (OffsetDest, 11);
    BOOST_REQUIRE_EQUAL (OffsetSrc, 16);
    BOOST_REQUIRE_EQUAL (Size, 5);
    break;

  case 3:
    BOOST_REQUIRE_EQUAL (OffsetDest, 16);
    BOOST_REQUIRE_EQUAL (OffsetSrc, 31);
    BOOST_REQUIRE_EQUAL (Size, 5);
    break;

  case 4:
    BOOST_REQUIRE_EQUAL (OffsetDest, 21);
    BOOST_REQUIRE_EQUAL (OffsetSrc, 36);
    BOOST_REQUIRE_EQUAL (Size, 14);
    break;
  }

  printf ("CALLBACK-%lu: Moving " FMT_MemSize_t " Byte(s) from " FMT_Offset_t
          " to " FMT_Offset_t "\n", *count, Size,
          OffsetSrc, OffsetDest);

  ++ (*count);
}

#define REQUIRE_ALLOC_SUCCESS(h,s,o)                                    \
  {                                                                     \
    BOOST_REQUIRE_EQUAL (tmmgr_alloc (&tmmgr, h, s), ALLOC_SUCCESS);    \
                                                                        \
    Offset_t Offset;                                                    \
    Size_t Size;                                                        \
                                                                        \
    tmmgr_offset_size (tmmgr, h, &Offset, &Size);                       \
    BOOST_REQUIRE_EQUAL (Offset, o);                                    \
    BOOST_REQUIRE_EQUAL (Size, s);                                      \
  }

BOOST_AUTO_TEST_CASE (tmmgr)
{
  Tmmgr_t tmmgr = NULL;

  tmmgr_init (&tmmgr, 45, 1);

  BOOST_REQUIRE_EQUAL (tmmgr_memsize (tmmgr), 45);
  BOOST_REQUIRE_EQUAL (tmmgr_memfree (tmmgr), 45);
  BOOST_REQUIRE_EQUAL (tmmgr_memused (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_minfree (tmmgr), 45);
  BOOST_REQUIRE_EQUAL (tmmgr_highwater (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_numhandle (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_numalloc (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_numfree (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_sumalloc (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_sumfree (tmmgr), 0);

  for (int i (0); i < 10; ++i)
  {
    REQUIRE_ALLOC_SUCCESS (i, 1, i);
  }

  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 2), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 6), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 3), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 7), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 8), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 1), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 5), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 4), RET_SUCCESS);

  BOOST_REQUIRE_EQUAL (tmmgr_memfree (tmmgr), 43);
  BOOST_REQUIRE_EQUAL (tmmgr_memused (tmmgr), 2);
  BOOST_REQUIRE_EQUAL (tmmgr_minfree (tmmgr), 35);
  BOOST_REQUIRE_EQUAL (tmmgr_highwater (tmmgr), 10);
  BOOST_REQUIRE_EQUAL (tmmgr_numhandle (tmmgr), 2);
  BOOST_REQUIRE_EQUAL (tmmgr_numalloc (tmmgr), 10);
  BOOST_REQUIRE_EQUAL (tmmgr_numfree (tmmgr), 8);
  BOOST_REQUIRE_EQUAL (tmmgr_sumalloc (tmmgr), 10);
  BOOST_REQUIRE_EQUAL (tmmgr_sumfree (tmmgr), 8);

  REQUIRE_ALLOC_SUCCESS (1, 1, 1);

  REQUIRE_ALLOC_SUCCESS (11, 4, 2);
  REQUIRE_ALLOC_SUCCESS (12, 4, 10);

  BOOST_REQUIRE_EQUAL (tmmgr_alloc (&tmmgr, 11, 4), ALLOC_DUPLICATE_HANDLE);
  BOOST_REQUIRE_EQUAL (tmmgr_alloc (&tmmgr, 12, 4), ALLOC_DUPLICATE_HANDLE);
  BOOST_REQUIRE_EQUAL (tmmgr_alloc (&tmmgr, 13, 35), ALLOC_INSUFFICIENT_MEMORY);
  BOOST_REQUIRE_EQUAL ( tmmgr_alloc (&tmmgr, 13, 34)
                      , ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY
                      );

  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 13), RET_HANDLE_UNKNOWN);

  BOOST_REQUIRE_EQUAL (tmmgr_memfree (tmmgr), 34);
  BOOST_REQUIRE_EQUAL (tmmgr_memused (tmmgr), 11);
  BOOST_REQUIRE_EQUAL (tmmgr_minfree (tmmgr), 34);
  BOOST_REQUIRE_EQUAL (tmmgr_highwater (tmmgr), 14);
  BOOST_REQUIRE_EQUAL (tmmgr_numhandle (tmmgr), 5);
  BOOST_REQUIRE_EQUAL (tmmgr_numalloc (tmmgr), 13);
  BOOST_REQUIRE_EQUAL (tmmgr_numfree (tmmgr), 8);
  BOOST_REQUIRE_EQUAL (tmmgr_sumalloc (tmmgr), 19);
  BOOST_REQUIRE_EQUAL (tmmgr_sumfree (tmmgr), 8);

  unsigned long callback_count = 0;

  /* complete defragmentation */
  tmmgr_defrag (&tmmgr, &fMemmove, NULL, &callback_count);

  BOOST_REQUIRE_EQUAL (tmmgr_memfree (tmmgr), 34);
  BOOST_REQUIRE_EQUAL (tmmgr_memused (tmmgr), 11);
  BOOST_REQUIRE_EQUAL (tmmgr_minfree (tmmgr), 34);
  BOOST_REQUIRE_EQUAL (tmmgr_highwater (tmmgr), 11);
  BOOST_REQUIRE_EQUAL (tmmgr_numhandle (tmmgr), 5);
  BOOST_REQUIRE_EQUAL (tmmgr_numalloc (tmmgr), 13);
  BOOST_REQUIRE_EQUAL (tmmgr_numfree (tmmgr), 8);
  BOOST_REQUIRE_EQUAL (tmmgr_sumalloc (tmmgr), 19);
  BOOST_REQUIRE_EQUAL (tmmgr_sumfree (tmmgr), 8);

  REQUIRE_ALLOC_SUCCESS (26, 5, 11);
  REQUIRE_ALLOC_SUCCESS (27, 5, 16);
  REQUIRE_ALLOC_SUCCESS (28, 5, 21);
  REQUIRE_ALLOC_SUCCESS (29, 5, 26);
  REQUIRE_ALLOC_SUCCESS (30, 5, 31);

  BOOST_REQUIRE_EQUAL (tmmgr_memfree (tmmgr), 9);
  BOOST_REQUIRE_EQUAL (tmmgr_memused (tmmgr), 36);
  BOOST_REQUIRE_EQUAL (tmmgr_minfree (tmmgr), 9);
  BOOST_REQUIRE_EQUAL (tmmgr_highwater (tmmgr), 36);
  BOOST_REQUIRE_EQUAL (tmmgr_numhandle (tmmgr), 10);
  BOOST_REQUIRE_EQUAL (tmmgr_numalloc (tmmgr), 18);
  BOOST_REQUIRE_EQUAL (tmmgr_numfree (tmmgr), 8);
  BOOST_REQUIRE_EQUAL (tmmgr_sumalloc (tmmgr), 44);
  BOOST_REQUIRE_EQUAL (tmmgr_sumfree (tmmgr), 8);

  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 26), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 28), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 29), RET_SUCCESS);

  BOOST_REQUIRE_EQUAL (tmmgr_memfree (tmmgr), 24);
  BOOST_REQUIRE_EQUAL (tmmgr_memused (tmmgr), 21);
  BOOST_REQUIRE_EQUAL (tmmgr_minfree (tmmgr), 9);
  BOOST_REQUIRE_EQUAL (tmmgr_highwater (tmmgr), 36);
  BOOST_REQUIRE_EQUAL (tmmgr_numhandle (tmmgr), 7);
  BOOST_REQUIRE_EQUAL (tmmgr_numalloc (tmmgr), 18);
  BOOST_REQUIRE_EQUAL (tmmgr_numfree (tmmgr), 11);
  BOOST_REQUIRE_EQUAL (tmmgr_sumalloc (tmmgr), 44);
  BOOST_REQUIRE_EQUAL (tmmgr_sumfree (tmmgr), 23);

  BOOST_REQUIRE_EQUAL ( tmmgr_alloc (&tmmgr, 99, 12)
                      , ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY
                      );

  /* only up to a free chunk of size 12 */
  MemSize_t FreeSizeWanted = 12;
  tmmgr_defrag (&tmmgr, &fMemmove, &FreeSizeWanted, &callback_count);

  REQUIRE_ALLOC_SUCCESS (99, 12, 16);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 99), RET_SUCCESS);

  BOOST_REQUIRE_EQUAL (tmmgr_resize (&tmmgr, 50), RESIZE_SUCCESS);

  BOOST_REQUIRE_EQUAL (tmmgr_memfree (tmmgr), 29);
  BOOST_REQUIRE_EQUAL (tmmgr_memused (tmmgr), 21);
  BOOST_REQUIRE_EQUAL (tmmgr_minfree (tmmgr), 9);
  BOOST_REQUIRE_EQUAL (tmmgr_highwater (tmmgr), 36);
  BOOST_REQUIRE_EQUAL (tmmgr_numhandle (tmmgr), 7);
  BOOST_REQUIRE_EQUAL (tmmgr_numalloc (tmmgr), 19);
  BOOST_REQUIRE_EQUAL (tmmgr_numfree (tmmgr), 12);
  BOOST_REQUIRE_EQUAL (tmmgr_sumalloc (tmmgr), 56);
  BOOST_REQUIRE_EQUAL (tmmgr_sumfree (tmmgr), 35);

  REQUIRE_ALLOC_SUCCESS (101, 14, 36);

  BOOST_REQUIRE_EQUAL (tmmgr_memfree (tmmgr), 15);
  BOOST_REQUIRE_EQUAL (tmmgr_memused (tmmgr), 35);
  BOOST_REQUIRE_EQUAL (tmmgr_minfree (tmmgr), 9);
  BOOST_REQUIRE_EQUAL (tmmgr_highwater (tmmgr), 50);
  BOOST_REQUIRE_EQUAL (tmmgr_numhandle (tmmgr), 8);
  BOOST_REQUIRE_EQUAL (tmmgr_numalloc (tmmgr), 20);
  BOOST_REQUIRE_EQUAL (tmmgr_numfree (tmmgr), 12);
  BOOST_REQUIRE_EQUAL (tmmgr_sumalloc (tmmgr), 70);
  BOOST_REQUIRE_EQUAL (tmmgr_sumfree (tmmgr), 35);

  BOOST_REQUIRE_EQUAL (tmmgr_resize (&tmmgr, 45), RESIZE_BELOW_HIGHWATER);

  tmmgr_defrag (&tmmgr, &fMemmove, NULL, &callback_count);

  BOOST_REQUIRE_EQUAL (tmmgr_resize (&tmmgr, 34), RESIZE_BELOW_MEMUSED);
  BOOST_REQUIRE_EQUAL (tmmgr_resize (&tmmgr, 35), RESIZE_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_resize (&tmmgr, 50), RESIZE_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_resize (&tmmgr, 45), RESIZE_SUCCESS);

  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 0), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 1), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 9), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 11), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 12), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 27), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 30), RET_SUCCESS);
  BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, 101), RET_SUCCESS);

  BOOST_REQUIRE_EQUAL (tmmgr_memfree (tmmgr), 45);
  BOOST_REQUIRE_EQUAL (tmmgr_memused (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_minfree (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_highwater (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_numhandle (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_numalloc (tmmgr), 20);
  BOOST_REQUIRE_EQUAL (tmmgr_numfree (tmmgr), 20);
  BOOST_REQUIRE_EQUAL (tmmgr_sumalloc (tmmgr), 70);
  BOOST_REQUIRE_EQUAL (tmmgr_sumfree (tmmgr), 70);

  BOOST_REQUIRE_EQUAL (tmmgr_resize (&tmmgr, 0), RESIZE_SUCCESS);

  BOOST_REQUIRE_EQUAL (tmmgr_memfree (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_memused (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_minfree (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_highwater (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_numhandle (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_numalloc (tmmgr), 20);
  BOOST_REQUIRE_EQUAL (tmmgr_numfree (tmmgr), 20);
  BOOST_REQUIRE_EQUAL (tmmgr_sumalloc (tmmgr), 70);
  BOOST_REQUIRE_EQUAL (tmmgr_sumfree (tmmgr), 70);

  BOOST_REQUIRE_EQUAL (tmmgr_resize (&tmmgr, 3141), RESIZE_SUCCESS);

  BOOST_REQUIRE_EQUAL (tmmgr_memfree (tmmgr), 3141);
  BOOST_REQUIRE_EQUAL (tmmgr_memused (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_minfree (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_highwater (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_numhandle (tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_numalloc (tmmgr), 20);
  BOOST_REQUIRE_EQUAL (tmmgr_numfree (tmmgr), 20);
  BOOST_REQUIRE_EQUAL (tmmgr_sumalloc (tmmgr), 70);
  BOOST_REQUIRE_EQUAL (tmmgr_sumfree (tmmgr), 70);

  BOOST_REQUIRE_NE (tmmgr_finalize (&tmmgr), 0);
  BOOST_REQUIRE_EQUAL (tmmgr_finalize (&tmmgr), 0);
}

BOOST_AUTO_TEST_CASE (tmmgr_aligned)
{
  Tmmgr_t tmmgrAligned = NULL;

  tmmgr_init (&tmmgrAligned, 45, (1 << 4));

  tmmgr_status (tmmgrAligned, NULL);

  do_resize (tmmgrAligned, 67);
  tmmgr_status (tmmgrAligned, NULL);

  do_resize (tmmgrAligned, 64);
  tmmgr_status (tmmgrAligned, NULL);

  for (Word_t i = 0; i < 10; ++i)
    do_alloc (tmmgrAligned, (Handle_t) i, (MemSize_t) 1);

  tmmgr_status (tmmgrAligned, NULL);

  do_resize (tmmgrAligned, 1000);

  for (Word_t i = 0; i < 10; ++i)
    do_alloc (tmmgrAligned, (Handle_t) i, (MemSize_t) 1);

  tmmgr_status (tmmgrAligned, NULL);

  do_resize (tmmgrAligned, 150);

  do_free (tmmgrAligned, 0);

  do_resize (tmmgrAligned, 150);

  unsigned long callback_count = 5;

  tmmgr_defrag (&tmmgrAligned, &fMemmove, NULL, &callback_count);

  do_resize (tmmgrAligned, 150);

  tmmgr_status (tmmgrAligned, NULL);

  tmmgr_finalize (&tmmgrAligned);
}
