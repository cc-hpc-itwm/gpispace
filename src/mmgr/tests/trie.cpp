// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE mmgr_trie
#include <boost/test/unit_test.hpp>

#include <mmgr/trie.h>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (trie)
{
  TrieMap_t tm = nullptr;

  BOOST_REQUIRE_EQUAL (trie_get (tm, 23), (unsigned long*) nullptr);
  BOOST_REQUIRE_EQUAL (trie_getany (tm), (unsigned long*) nullptr);
  BOOST_REQUIRE_EQUAL (tm, (TrieMap_t) nullptr);

  Bool_t was_there;

  {
    const PValue_t PVal = trie_ins (&tm, 23, &was_there);

    BOOST_REQUIRE_NE (tm, (TrieMap_t) nullptr);
    BOOST_REQUIRE_NE (PVal, (unsigned long*) nullptr);
    BOOST_REQUIRE_EQUAL (was_there, False);

    *PVal = 23;

    const PValue_t PVal2 = trie_ins (&tm, 23, &was_there);

    BOOST_REQUIRE_EQUAL (was_there, True);
    BOOST_REQUIRE_EQUAL (PVal, PVal2);

    BOOST_REQUIRE_EQUAL (trie_get (tm, 23), PVal);
    BOOST_REQUIRE_EQUAL (trie_getany (tm), PVal);
  }

  BOOST_REQUIRE_EQUAL (trie_size (tm), 1);

  trie_del (&tm, 23, fUserNone);

  BOOST_REQUIRE_EQUAL (trie_size (tm), 0);

  BOOST_REQUIRE_EQUAL (trie_get (tm, 23), (unsigned long*) nullptr);
  BOOST_REQUIRE_EQUAL (trie_getany (tm), (unsigned long*) nullptr);

  trie_del (&tm, 23, fUserNone);

  BOOST_REQUIRE_EQUAL (trie_size (tm), 0);

  BOOST_REQUIRE_EQUAL (trie_get (tm, 23), (unsigned long*) nullptr);
  BOOST_REQUIRE_EQUAL (trie_getany (tm), (unsigned long*) nullptr);

  *trie_ins (&tm, 23, &was_there) = 23;
  *trie_ins (&tm, 99, &was_there) = 99;

  BOOST_REQUIRE_EQUAL (trie_size (tm), 2);

  BOOST_REQUIRE_EQUAL (*trie_get (tm, 23), 23);
  BOOST_REQUIRE_EQUAL (*trie_get (tm, 99), 99);
  BOOST_REQUIRE_EQUAL (*trie_getany (tm), 99);
  BOOST_REQUIRE_EQUAL (trie_get (tm, 44), (unsigned long*) nullptr);

  trie_del (&tm, 23, fUserNone);

  BOOST_REQUIRE_EQUAL (trie_get (tm, 23), (unsigned long*) nullptr);

  BOOST_REQUIRE_EQUAL (*trie_get (tm, 99), 99);
  BOOST_REQUIRE_EQUAL (*trie_getany (tm), 99);

  trie_del (&tm, 99, fUserNone);

  BOOST_REQUIRE_EQUAL (trie_get (tm, 99), (unsigned long*) nullptr);
  BOOST_REQUIRE_EQUAL (trie_getany (tm), (unsigned long*) nullptr);

  BOOST_REQUIRE_EQUAL (trie_size (tm), 0);

  *trie_ins (&tm, 23, &was_there) = 23;
  *trie_ins (&tm, 99, &was_there) = 99;
  *trie_ins (&tm, 44, &was_there) = 44;
  *trie_ins (&tm, 13, &was_there) = 13;

  Size_t size (trie_memused (tm, fUserNone));

  BOOST_REQUIRE_EQUAL (trie_free (&tm, fUserNone), size);
}

BOOST_AUTO_TEST_CASE (dups)
{
  TrieMap_t tm = nullptr;

  srand (31415926);

  Word_t dups = 0;

  for (Size_t i = 0; i < (1 << 22); ++i)
  {
    Bool_t was_there;

    BOOST_REQUIRE_NE ( trie_ins (&tm, rand(), &was_there)
                     , (unsigned long*) nullptr
                     );

    dups += (was_there == True) ? 1 : 0;
  }

  BOOST_REQUIRE_EQUAL (dups, 4207);
  BOOST_REQUIRE_EQUAL (trie_size (tm), 4190097);

  Size_t const Bytes (trie_memused (tm, fUserNone));

  BOOST_REQUIRE_EQUAL (Bytes, trie_free (&tm, fUserNone));
}
