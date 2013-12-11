// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE mmgr_fseg
#include <boost/test/unit_test.hpp>

#include <stdio.h>
#include <stdlib.h>

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

BOOST_AUTO_TEST_CASE (fseg)
{
  FSeg_t FSeg = NULL;
  Key_t Key;

  Key = 1 << 9;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 9);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 9);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 9);

  Key = 1 << 10;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 10);

  Key = 1 << 11;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 11);

  Key = 1 << 12;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 12);

  Key = 1 << 13;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);

  BOOST_REQUIRE_EQUAL (fseg_size (FSeg), 0);

  fseg_ins (&FSeg, 1 << 10, 10);
  BOOST_REQUIRE_EQUAL (fseg_size (FSeg), 1);

  fseg_ins (&FSeg, 1 << 10, 11);
  BOOST_REQUIRE_EQUAL (fseg_size (FSeg), 2);

  fseg_ins (&FSeg, 1 << 11, 10);
  BOOST_REQUIRE_EQUAL (fseg_size (FSeg), 3);

  fseg_ins (&FSeg, 1 << 12, 11);
  BOOST_REQUIRE_EQUAL (fseg_size (FSeg), 4);

  Key = 1 << 9;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 9);
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  }
  {
    Key = 1 << 9;
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  }

  Key = 1 << 10;
  {
    PValue_t PVal = fseg_get (FSeg, Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  }
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  }
  {
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  }

  Key = 1 << 11;
  {
    PValue_t PVal = fseg_get (FSeg, Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }
  {
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }

  Key = 1 << 12;
  {
    PValue_t PVal = fseg_get (FSeg, Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  }
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  }
  {
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  }

  Key = 1 << 13;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);

  fseg_del (&FSeg, 1 << 10, 10, SMAP_DEL_DEFAULT);

  BOOST_REQUIRE_EQUAL (fseg_size (FSeg), 3);

  Key = 1 << 9;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 9);
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  }
  {
    Key = 1 << 9;
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  }

  Key = 1 << 10;
  {
    PValue_t PVal = fseg_get (FSeg, Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  }
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  }
  {
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  }

  Key = 1 << 11;
  {
    PValue_t PVal = fseg_get (FSeg, Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }
  {
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }

  Key = 1 << 12;
  {
    PValue_t PVal = fseg_get (FSeg, Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  }
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  }
  {
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  }

  Key = 1 << 13;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);

  fseg_del (&FSeg, 1 << 10, 10, SMAP_DEL_DEFAULT);

  BOOST_REQUIRE_EQUAL (fseg_size (FSeg), 3);

  fseg_del (&FSeg, 1 << 10, 11, SMAP_DEL_DEFAULT);

  BOOST_REQUIRE_EQUAL (fseg_size (FSeg), 2);

  Key = 1 << 9;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 9);
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }
  {
    Key = 1 << 9;
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }

  Key = 1 << 10;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }
  {
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }

  Key = 1 << 11;
  {
    PValue_t PVal = fseg_get (FSeg, Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }
  {
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 10);
    BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  }

  Key = 1 << 12;
  {
    PValue_t PVal = fseg_get (FSeg, Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  }
  {
    PValue_t PVal = fseg_get_atleast (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  }
  {
    PValue_t PVal = fseg_get_atleast_minimal (FSeg, &Key);
    BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
    BOOST_REQUIRE_EQUAL (*PVal, 11);
    BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  }

  Key = 1 << 13;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);

  fseg_del (&FSeg, 1 << 11, 10, SMAP_DEL_DEFAULT);
  fseg_del (&FSeg, 1 << 12, 11, SMAP_DEL_DEFAULT);

  BOOST_REQUIRE_EQUAL (fseg_size (FSeg), 0);

  Key = 1 << 9;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 9);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 9);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 9);

  Key = 1 << 10;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 10);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 10);

  Key = 1 << 11;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 11);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 11);

  Key = 1 << 12;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 12);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 12);

  Key = 1 << 13;
  BOOST_REQUIRE_EQUAL (fseg_get (FSeg, Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);
  BOOST_REQUIRE_EQUAL (fseg_get_atleast_minimal (FSeg, &Key), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (Key, 1 << 13);

  fseg_ins (&FSeg, 1 << 10, 10);
  fseg_ins (&FSeg, 1 << 10, 11);
  fseg_ins (&FSeg, 1 << 11, 10);
  fseg_ins (&FSeg, 1 << 12, 11);

  BOOST_REQUIRE_EQUAL (fseg_size (FSeg), 4);

  Size_t const memused (fseg_memused (FSeg));

  BOOST_REQUIRE_EQUAL (memused, fseg_free (&FSeg));
}
