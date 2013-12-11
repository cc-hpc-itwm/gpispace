// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE mmgr_smap
#include <boost/test/unit_test.hpp>

#include <stdio.h>
#include <stdlib.h>

#include <mmgr/smap.h>

#include <fhg/util/first_then.hpp>

#include <iostream>
#include <sstream>
#include <string>

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

namespace
{
  struct print_sorted_state
  {
  public:
    print_sorted_state (std::ostream& os)
      : _os (os)
      , _sep ("", ", ")
    {}

    void operator() ( const Key_t Key
                    , const Value_t Value
                    ) const
    {
      _os << _sep << "(" << Key << ", " << Value << ")";
    }

  private:
    std::ostream& _os;
    fhg::util::first_then<std::string> _sep;
  };

  void fPrintSorted ( const Key_t Key
                    , const Value_t Value
                    , void* PDat
                    )
  {
    return static_cast<print_sorted_state*> (PDat)->operator() (Key, Value);
  }

  void smap_print_sorted ( std::ostream& os
                         , const SMap_t sm
                         )
  {
    print_sorted_state s (os);

    smap_work_inorder (sm, &fPrintSorted, &s);
  }

  struct print_cut_state
  {
  public:
    print_cut_state (Level_t max_level, std::ostream& os)
      : _max_level (max_level)
      , _os (os)
    {}

    Cut_t operator() ( const Key_t Key
                     , const Value_t Value
                     , const Level_t Level
                     ) const
    {
      _os << std::string (Level, ' ') << Key << " " << Value << std::endl;

      return (Level >= _max_level) ? Cut : NoCut;
    }

  private:
    Level_t _max_level;
    std::ostream& _os;
  };

  Cut_t fPrintCut ( const Key_t Key
                  , const Value_t Value
                  , const Level_t Level
                  , void* PDat
                  )
  {
    return static_cast<print_cut_state*> (PDat)->operator() (Key, Value, Level);
  }

  void smap_print_max ( std::ostream& os
                      , const SMap_t sm
                      , const Level_t MaxLevel
                      )
  {
    print_cut_state s (MaxLevel, os);

    smap_work_preorder (sm, &fPrintCut, &s);
  }
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

  {
    std::ostringstream oss;
    smap_print_max (oss, sm, 5);

    BOOST_REQUIRE_EQUAL
      ( "1023 23\n"
        " 1013 13\n"
        " 1099 99\n"
        "  1044 44\n"
      , oss.str()
      );
  }

  {
    std::ostringstream oss;
    smap_print_sorted (oss, sm);

    BOOST_REQUIRE_EQUAL
      ( "(1013, 13), (1023, 23), (1044, 44), (1099, 99)"
      , oss.str()
      );
  }

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

  {
    std::ostringstream oss;
    smap_print_max (oss, sm, 5);

    BOOST_REQUIRE_EQUAL
      ("1018338111 0\n"
       " 410317456 3\n"
       "  156771579 10\n"
       "   10861679 23\n"
       "    5966934 54\n"
       "     4463125 190\n"
       "     10151106 152\n"
       "    27304684 33\n"
       "     11637943 226\n"
       "     57765334 40\n"
       "   206624788 15\n"
       "    159055947 105\n"
       "     157346747 1040\n"
       "     177287748 134\n"
       "    371612659 16\n"
       "     224977038 29\n"
       "     394083169 31\n"
       "  976470493 6\n"
       "   424377862 9\n"
       "    423064182 26\n"
       "     415820650 45\n"
       "     423550893 455\n"
       "    821559718 11\n"
       "     644021682 12\n"
       "     970921189 13\n"
       "   986088075 71\n"
       "    983101390 115\n"
       "     979170834 250\n"
       "     984812264 366\n"
       "    990966349 149\n"
       "     990425667 831\n"
       "     996805036 284\n"
       " 1710946552 1\n"
       "  1162573936 4\n"
       "   1096140465 76\n"
       "    1042725381 126\n"
       "     1038049013 146\n"
       "     1087900298 147\n"
       "    1117299594 87\n"
       "     1113039640 242\n"
       "     1148027986 107\n"
       "   1443454879 5\n"
       "    1311918041 39\n"
       "     1249773945 55\n"
       "     1393461975 66\n"
       "    1504294775 18\n"
       "     1470759563 36\n"
       "     1536561341 21\n"
       "  1876219691 2\n"
       "   1780871119 37\n"
       "    1779853520 88\n"
       "     1747891044 123\n"
       "     1780060076 99\n"
       "    1867589939 44\n"
       "     1801784273 52\n"
       "     1874155639 192\n"
       "   1936366940 7\n"
       "    1888979153 20\n"
       "     1880742071 58\n"
       "     1935923590 32\n"
       "    1988642125 8\n"
       "     1985272747 17\n"
       "     2050951043 22\n"
      , oss.str()
      );
  }

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
