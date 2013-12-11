// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE mmgr_ostab
#include <boost/test/unit_test.hpp>

#include <stdio.h>
#include <stdlib.h>

#include <mmgr/ostab.h>

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
fPrint (const Key_t Key, const Offset_t Offset, const Size_t Size, void*)
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

BOOST_AUTO_TEST_CASE (ostab)
{
  OStab_t ostab = NULL;

  BOOST_REQUIRE_EQUAL (ostab_memused (ostab), 0);
  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 0);

  BOOST_REQUIRE_EQUAL (ostab_ins (&ostab, 23, 0xfff, 1 << 20), False);
  BOOST_REQUIRE_EQUAL (ostab_ins (&ostab, 42, 0xaff, 1 << 12), False);

  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 2);

  Offset_t Offset;
  Size_t Size;

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 23, &Offset, &Size), True);
  BOOST_REQUIRE_EQUAL (Offset, 0xfff);
  BOOST_REQUIRE_EQUAL (Size, 1 << 20);

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 42, &Offset, &Size), True);
  BOOST_REQUIRE_EQUAL (Offset, 0xaff);
  BOOST_REQUIRE_EQUAL (Size, 1 << 12);

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 99, &Offset, &Size), False);

  ostab_del (&ostab, 99);

  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 2);

  ostab_del (&ostab, 23);

  BOOST_REQUIRE_EQUAL (ostab_size (ostab), 1);

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 23, &Offset, &Size), False);

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 42, &Offset, &Size), True);
  BOOST_REQUIRE_EQUAL (Offset, 0xaff);
  BOOST_REQUIRE_EQUAL (Size, 1 << 12);

  BOOST_REQUIRE_EQUAL (ostab_get (ostab, 99, &Offset, &Size), False);

  Size_t const memused (ostab_memused (ostab));

  BOOST_REQUIRE_EQUAL (ostab_free (&ostab), memused);
}

BOOST_AUTO_TEST_CASE (ostab_mega)
{
  OStab_t ostab = NULL;

  for (Word_t i = 0; i < (1 << 20); ++i)
    {
      ostab_ins (&ostab, (Key_t) i, (Offset_t) i, (Size_t) i);
    }

  printf ("Bytes = " FMT_Size_t "\n", ostab_free (&ostab));

  for (Word_t i = 0; i < (1 << 20); ++i)
    {
      ostab_ins (&ostab, (Key_t) i, (Offset_t) i, (Size_t) i);
    }

  for (Word_t i = 0; i < (1 << 20); ++i)
    {
      ostab_del (&ostab, (Key_t) i);
    }

  printf ("Bytes = " FMT_Size_t "\n", ostab_free (&ostab));
}
