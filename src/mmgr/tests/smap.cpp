// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE mmgr_smap
#include <boost/test/unit_test.hpp>

#include <stdio.h>
#include <stdlib.h>

#include <mmgr/smap.h>

static void
get (const SMap_t sm, const Key_t key)
{
  const PValue_t PVal = smap_get (sm, key);

  printf ("get " FMT_Key_t ": sm = %p, PVal = %p", key, sm, PVal);

  if (PVal != NULL)
    {
      printf (", Just (*Pval = " FMT_Value_t ")\n", *PVal);
    }
  else
    {
      printf (", Nothing\n");
    }
}

static void
get_atleast (const SMap_t sm, const Key_t key)
{
  Key_t key_got = key;

  {
    const PValue_t PVal = smap_get_atleast (sm, &key_got);

    printf ("get_atleast " FMT_Key_t ": sm = %p, PVal = %p", key, sm, PVal);

    if (PVal != NULL)
      {
        printf (", Just (*Pval = " FMT_Value_t ", key_got = " FMT_Key_t ")\n",
                *PVal, key_got);
      }
    else
      {
        printf (", Nothing\n");
      }
  }

  {
    key_got = key;

    const PValue_t PVal = smap_get_atleast_minimal (sm, &key_got);

    printf ("get_atleast_minimal " FMT_Key_t ": sm = %p, PVal = %p", key, sm,
            PVal);

    if (PVal != NULL)
      {
        printf (", Just (*Pval = " FMT_Value_t ", key_got = " FMT_Key_t ")\n",
                *PVal, key_got);
      }
    else
      {
        printf (", Nothing\n");
      }
  }
}

static void
ins (PSMap_t sm, const Key_t key, const Value_t val)
{
  const Bool_t was_there = smap_ins (sm, key, val);

  printf ("ins (" FMT_Key_t "," FMT_Value_t "): sm = %p, was_there = %s\n",
          key, val, sm, (was_there == True) ? "True" : "False");
}

static void
del (PSMap_t sm, const Key_t key)
{
  const Bool_t was_there = smap_del (sm, key, SMAP_DEL_DEFAULT);

  printf ("del " FMT_Key_t ": sm = %p, was_there = %s\n", key, sm,
          (was_there == True) ? "True" : "False");
}

static void
fPrint (const Key_t Key, const Value_t Value, void*)
{
  printf (" (" FMT_Key_t "," FMT_Value_t ")", Key, Value);
}

static void
smap_print_sorted (const SMap_t PCTree)
{
  smap_work_inorder (PCTree, &fPrint, NULL);
}

static Cut_t
fPrintCut (const Key_t Key, const Value_t Value, const Level_t Level,
           void *Pdat)
{
  Level_t MaxLevel = *(PLevel_t) Pdat;

  for (Level_t i = 0; i < Level; ++i)
    printf (" ");

  printf (FMT_Key_t " " FMT_Value_t "\n", Key, Value);

  return (Level >= MaxLevel) ? Cut : NoCut;;
}


static void
smap_print_max (const SMap_t sm, const Level_t MaxLevel)
{
  Level_t L = MaxLevel;

  smap_work_preorder (sm, &fPrintCut, &L);
}

BOOST_AUTO_TEST_CASE (smap)
{
  SMap_t sm = NULL;

  BOOST_REQUIRE_EQUAL (smap_memused (sm), 0);
  BOOST_REQUIRE_EQUAL (smap_size (sm), 0);

  BOOST_REQUIRE_EQUAL (smap_get (sm, 23), (PValue_t) NULL);

  BOOST_REQUIRE_EQUAL (smap_ins (&sm, 1023, 23), False);
  BOOST_REQUIRE_EQUAL (smap_size (sm), 1);
  BOOST_REQUIRE_EQUAL (smap_ins (&sm, 1023, 23), True);
  BOOST_REQUIRE_EQUAL (smap_size (sm), 1);

  PValue_t PVal;

  PVal = smap_get (sm, 1023);

  BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (*PVal, 23);

  BOOST_REQUIRE_EQUAL (smap_del (&sm, 1023, SMAP_DEL_DEFAULT), True);
  BOOST_REQUIRE_EQUAL (smap_size (sm), 0);
  BOOST_REQUIRE_EQUAL (smap_get (sm, 1023), (PValue_t) NULL);

  BOOST_REQUIRE_EQUAL (smap_del (&sm, 1023, SMAP_DEL_DEFAULT), False);
  BOOST_REQUIRE_EQUAL (smap_size (sm), 0);
  BOOST_REQUIRE_EQUAL (smap_get (sm, 1023), (PValue_t) NULL);

  BOOST_REQUIRE_EQUAL (smap_ins (&sm, 1023, 23), False);
  BOOST_REQUIRE_EQUAL (smap_ins (&sm, 1099, 99), False);

  PVal = smap_get (sm, 1023);

  BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (*PVal, 23);

  PVal = smap_get (sm, 1099);

  BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (*PVal, 99);

  BOOST_REQUIRE_EQUAL (smap_get (sm, 1044), (PValue_t) NULL);

  BOOST_REQUIRE_EQUAL (smap_del (&sm, 1044, SMAP_DEL_DEFAULT), False);
  BOOST_REQUIRE_EQUAL (smap_size (sm), 2);

  PVal = smap_get (sm, 1023);

  BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (*PVal, 23);

  PVal = smap_get (sm, 1099);

  BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (*PVal, 99);

  BOOST_REQUIRE_EQUAL (smap_get (sm, 1044), (PValue_t) NULL);

  BOOST_REQUIRE_EQUAL (smap_del (&sm, 1023, SMAP_DEL_DEFAULT), True);

  BOOST_REQUIRE_EQUAL (smap_size (sm), 1);

  BOOST_REQUIRE_EQUAL (smap_get (sm, 1023), (PValue_t) NULL);

  PVal = smap_get (sm, 1099);

  BOOST_REQUIRE_NE (PVal, (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (*PVal, 99);

  BOOST_REQUIRE_EQUAL (smap_get (sm, 1044), (PValue_t) NULL);

  BOOST_REQUIRE_EQUAL (smap_del (&sm, 1099, SMAP_DEL_DEFAULT), True);

  BOOST_REQUIRE_EQUAL (smap_size (sm), 0);

  BOOST_REQUIRE_EQUAL (smap_get (sm, 1023), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (smap_get (sm, 1099), (PValue_t) NULL);
  BOOST_REQUIRE_EQUAL (smap_get (sm, 1044), (PValue_t) NULL);

  BOOST_REQUIRE_EQUAL (smap_ins (&sm, 1023, 23), False);
  BOOST_REQUIRE_EQUAL (smap_ins (&sm, 1099, 99), False);
  BOOST_REQUIRE_EQUAL (smap_ins (&sm, 1044, 44), False);
  BOOST_REQUIRE_EQUAL (smap_ins (&sm, 1013, 13), False);

  smap_print_max (sm, 5);
  smap_print_sorted (sm);
  printf ("\n");

  for (Key_t k = 5; k < 110; k += 5)
    get_atleast (sm, 1000 + k);

  get_atleast (sm, 1013);
  get_atleast (sm, 1023);
  get_atleast (sm, 1044);
  get_atleast (sm, 1099);

  Size_t Bytes = smap_memused (sm);

  printf ("sm = %p, Bytes = " FMT_Size_t "\n", sm, Bytes);

  Bytes = smap_free (&sm);

  printf ("sm = %p, Bytes = " FMT_Size_t "\n", sm, Bytes);

  Word_t dups = 0;

  srand (31415926);

  for (Word_t i = 0; i < (1 << 20); ++i)
    {
      Bool_t was_there = smap_ins (&sm, (Key_t) rand (), (Value_t) i);

      dups += (was_there == True) ? 1 : 0;
    }

  smap_print_max (sm, 5);

  Size_t Size = smap_size (sm);
  Bytes = smap_memused (sm);

  printf ("dups = " FMT_Word_t ", size = " FMT_Size_t ", mem = " FMT_Size_t
          "\n", dups, Size, Bytes);

  srand (31415926);

  for (Word_t i = 0; i < (1 << 20); ++i)
    {
      smap_del (&sm, (Key_t) rand (), SMAP_DEL_INORDER_SUCC);
    }

  srand (27182818);

  for (Word_t i = 0; i < (1 << 20); ++i)
    {
      smap_ins (&sm, (Key_t) rand (), (Value_t) i);
    }

  Bytes = smap_free (&sm);

  printf ("sm = %p, Bytes = " FMT_Size_t "\n", sm, Bytes);
}
