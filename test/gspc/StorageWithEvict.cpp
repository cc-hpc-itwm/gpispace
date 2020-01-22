#include <boost/test/unit_test.hpp>

#include <gspc/StorageWithEvict.hpp>

#include <util-generic/testing/random.hpp>

#include <vector>

namespace gspc
{
  using Storage = StorageWithEvict<int, std::string>;
  using fhg::util::testing::random;
  using fhg::util::testing::randoms;
  using fhg::util::testing::unique_random;

  namespace
  {
    void ignore_evict (Storage::Storage&) {}
    void forbid_evict (Storage::Storage&) { BOOST_TEST (false); }

    struct expect_evict
    {
      expect_evict (Storage::Storage elements = {}) : _elements (elements) {}
      void operator() (Storage::Storage& elements)
      {
        BOOST_TEST (0 == _called++);
        BOOST_TEST (_elements == elements);
      }
      ~expect_evict()
      {
        BOOST_TEST (_called == 1);
      }

      unsigned _called {0};
      Storage::Storage _elements;
    };

#define C(value_) [&] { return value_; }
  }

  BOOST_AUTO_TEST_CASE (unknown_element_calls_evict_ctor)
  {
    StorageWithEvict<int, std::string> storage;

    auto const id (random<int>{}());
    auto const value (random<std::string>{}());

    expect_evict expect_evict;

    BOOST_TEST (value == storage.at_or_construct (id, expect_evict, value));
  }
  BOOST_AUTO_TEST_CASE (unknown_element_calls_evict_create)
  {
    StorageWithEvict<int, std::string> storage;

    auto const id (random<int>{}());
    auto const value (random<std::string>{}());

    expect_evict expect_evict;

    BOOST_TEST (value == storage.at_or_create (id, expect_evict, C (value)));
  }

  BOOST_AUTO_TEST_CASE (known_element_does_not_call_evict_ctor)
  {
    StorageWithEvict<int, std::string> storage;

    auto const id (random<int>{}());
    auto const value (random<std::string>{}());

    storage.at_or_construct (id, ignore_evict, value);

    BOOST_TEST (value == storage.at_or_construct (id, forbid_evict, value));
  }
  BOOST_AUTO_TEST_CASE (known_element_does_not_call_evict_create)
  {
    StorageWithEvict<int, std::string> storage;

    auto const id (random<int>{}());
    auto const value (random<std::string>{}());

    storage.at_or_construct (id, ignore_evict, value);

    BOOST_TEST (value == storage.at_or_create (id, forbid_evict, C (value)));
  }

  BOOST_AUTO_TEST_CASE (known_elements_are_not_overwritten_construct)
  {
    StorageWithEvict<int, std::string> storage;

    auto const id (random<int>{}());
    auto const values (randoms<std::vector<std::string>, unique_random> (2));

    storage.at_or_construct (id, ignore_evict, values.at (0));

    BOOST_TEST
      (  values.at (0)
      == storage.at_or_construct (id, forbid_evict, values.at (1))
      );
  }
  BOOST_AUTO_TEST_CASE (known_elements_are_not_overwritten_create)
  {
    StorageWithEvict<int, std::string> storage;

    auto const id (random<int>{}());
    auto const values (randoms<std::vector<std::string>, unique_random> (2));

    storage.at_or_construct (id, ignore_evict, values.at (0));

    BOOST_TEST
      ( values.at (0)
      == storage.at_or_create (id, forbid_evict, C (values.at (1)))
      );
  }

  BOOST_AUTO_TEST_CASE (evict_might_evict_ctor)
  {
    StorageWithEvict<int, std::string> storage;

    auto const ids (randoms<std::vector<int>, unique_random> (2));
    auto const values (randoms<std::vector<std::string>> (3));

    storage.at_or_construct (ids.at (0), ignore_evict, values.at (0));
    storage.at_or_construct ( ids.at (1)
                            , [&] (Storage::Storage& elements)
                              {
                                BOOST_TEST (1 == elements.erase (ids.at (0)));
                              }
                            , values.at (1)
                            );

    expect_evict expect_evict ({{ids.at (1), values.at (1)}});

    BOOST_TEST
      (  values.at (2)
      == storage.at_or_construct (ids.at (0), expect_evict, values.at (2))
      );
  }
  BOOST_AUTO_TEST_CASE (evict_might_evict_create)
  {
    StorageWithEvict<int, std::string> storage;

    auto const ids (randoms<std::vector<int>, unique_random> (2));
    auto const values (randoms<std::vector<std::string>> (3));

    storage.at_or_create (ids.at (0), ignore_evict, C (values.at (0)));
    storage.at_or_create ( ids.at (1)
                         , [&] (Storage::Storage& elements)
                           {
                             BOOST_TEST (1 == elements.erase (ids.at (0)));
                           }
                         , C (values.at (1))
                         );

    expect_evict expect_evict ({{ids.at (1), values.at (1)}});

    BOOST_TEST
      (  values.at (2)
      == storage.at_or_create (ids.at (0), expect_evict, C (values.at (2)))
      );
  }
}
