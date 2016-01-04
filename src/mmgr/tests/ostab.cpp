// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE mmgr_ostab
#include <boost/test/unit_test.hpp>

#include <mmgr/ostab.h>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (ostab)
{
  OStab_t ostab = nullptr;

  BOOST_REQUIRE_EQUAL (ostab_memused (ostab), 0);
  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 0);

  BOOST_REQUIRE_EQUAL (ostab_ins (&ostab, 23, 0xfff, 1 << 20), false);
  BOOST_REQUIRE_EQUAL (ostab_ins (&ostab, 42, 0xaff, 1 << 12), false);

  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 2);

  Offset_t Offset;
  Size_t Size;

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 23, &Offset, &Size), true);
  BOOST_REQUIRE_EQUAL (Offset, 0xfff);
  BOOST_REQUIRE_EQUAL (Size, 1 << 20);

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 42, &Offset, &Size), true);
  BOOST_REQUIRE_EQUAL (Offset, 0xaff);
  BOOST_REQUIRE_EQUAL (Size, 1 << 12);

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 99, &Offset, &Size), false);

  ostab_del (&ostab, 99);

  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 2);

  ostab_del (&ostab, 23);

  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 1);

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 23, &Offset, &Size), false);

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 42, &Offset, &Size), true);
  BOOST_REQUIRE_EQUAL (Offset, 0xaff);
  BOOST_REQUIRE_EQUAL (Size, 1 << 12);

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 99, &Offset, &Size), false);

  Size_t const memused (ostab_memused (ostab));

  BOOST_REQUIRE_EQUAL (ostab_free (&ostab), memused);
}

BOOST_AUTO_TEST_CASE (ostab_mega)
{
  OStab_t ostab = nullptr;

  for (Word_t i = 0; i < (1 << 20); ++i)
  {
    ostab_ins (&ostab, (Key_t) i, (Offset_t) i, (Size_t) i);
  }

  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 1 << 20);

  {
    Size_t const memused (ostab_memused (ostab));

    BOOST_REQUIRE_EQUAL (ostab_free (&ostab), memused);
  }

  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 0);
  BOOST_REQUIRE_EQUAL (ostab_memused (ostab), 0);

  for (Word_t i = 0; i < (1 << 20); ++i)
  {
    ostab_ins (&ostab, (Key_t) i, (Offset_t) i, (Size_t) i);
  }

  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 1 << 20);

  for (Word_t i = 0; i < (1 << 20); ++i)
  {
    ostab_del (&ostab, (Key_t) i);
  }

  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 0);
  BOOST_REQUIRE_EQUAL (ostab_memused (ostab), 0);
  BOOST_REQUIRE_EQUAL (ostab_free (&ostab), 0);
}
