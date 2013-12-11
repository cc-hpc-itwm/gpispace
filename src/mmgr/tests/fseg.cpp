// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE mmgr_fseg
#include <boost/test/unit_test.hpp>

#include <stdio.h>
#include <stdlib.h>

#include <mmgr/fseg.h>

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
