#include <boost/test/unit_test.hpp>

#include <drts/cache_management/cache_manager.hpp>
#include <drts/cache_management/circular_cache_manager.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <unordered_set>
#include <unordered_map>
#include <string>

namespace
{
  drts::cache::Dataid gen_dataid(unsigned long id)
  {
    return "buf_" + std::to_string(id);
  }

  std::unordered_set<drts::cache::Dataid> gen_dataid_list
    ( std::unordered_set<unsigned long> ids)
  {
    std::unordered_set<drts::cache::Dataid> dataids;
    for (auto& id : ids)
    {
      dataids.emplace(gen_dataid(id));
    }
    return dataids;
  }

  std::unordered_map<drts::cache::Dataid, unsigned long> gen_cache_elems_list
    ( std::unordered_set<unsigned long> ids
    , unsigned long elem_size
    )
  {
    std::unordered_map<drts::cache::Dataid, unsigned long> elems_to_add;
    for (auto& id : ids)
    {
      elems_to_add.emplace( gen_dataid(id), elem_size);
    }
    return elems_to_add;
  }


  void test_add_to_cache_one_new_elem
    ( drts::cache::cache_manager* const cache_man
    , unsigned long id
    , unsigned long elem_size
    , unsigned long expected_offset
    )
  {
    auto already_in_cache = cache_man->add_chunk_list_to_cache
        ( gen_cache_elems_list
            ( std::unordered_set<unsigned long>({id})
            , elem_size
            ));

    BOOST_REQUIRE_EQUAL (already_in_cache.size(), 0);
    BOOST_REQUIRE_EQUAL (cache_man->is_cached(gen_dataid(id)), true);
    BOOST_REQUIRE_EQUAL (cache_man->offset(gen_dataid(id)), expected_offset);
  }

  void test_add_to_cache_multiple_elems
    ( drts::cache::cache_manager* const cache_man
    , std::unordered_map<drts::cache::Dataid, unsigned long> elems_to_add
    , std::unordered_set<drts::cache::Dataid> expected_already_cached_ids
    )
  {
    auto already_in_cache = cache_man->add_chunk_list_to_cache(elems_to_add);

    BOOST_REQUIRE_EQUAL (already_in_cache.size(), expected_already_cached_ids.size());
    for (auto elem : already_in_cache)
    {
      BOOST_REQUIRE_EQUAL (expected_already_cached_ids.count(elem), 1);
    }
    for (auto elem : elems_to_add)
    {
      BOOST_REQUIRE_EQUAL (cache_man->is_cached(elem.first), true);
    }
  }
}


BOOST_AUTO_TEST_CASE (test_simple_add)
{
  unsigned long size = 100;
  unsigned long elem_size = 10;
  drts::cache::circular_cache_manager cache_man(size);

  BOOST_REQUIRE_EQUAL (cache_man.size(), size);

  // fill the cache
  unsigned long expected_offset = 0;
  for (int i (0); i < 10; ++i)
  {
    test_add_to_cache_one_new_elem(&cache_man, i, elem_size, expected_offset);
    expected_offset += elem_size;
  }

  // reset offset as older cache entries are replaced when the cache is full
  expected_offset = 0;
  for (int i (10); i < 20; ++i)
  {
    test_add_to_cache_one_new_elem(&cache_man, i, elem_size, expected_offset);
    expected_offset += elem_size;
  }

  // add larger cache buffer
  elem_size = 50;
  expected_offset = 0;
  test_add_to_cache_one_new_elem(&cache_man, 1000, elem_size, expected_offset);

  // reset offset when older cache entries are evicted
  elem_size = 10;
  expected_offset = 50;
  for (int i (31); i < 37; ++i)
  {
    test_add_to_cache_one_new_elem(&cache_man, i, elem_size, expected_offset % 100);
    expected_offset += elem_size;
  }

  expected_offset = 10;
  elem_size = 50;
  test_add_to_cache_one_new_elem(&cache_man, 2000, elem_size, expected_offset);
}



BOOST_AUTO_TEST_CASE (test_exceptions)
{
  unsigned long size = 100;
  unsigned long elem_size = 300;
  drts::cache::circular_cache_manager  cache_man(size);

  unsigned long id = 300;
  fhg::util::testing::require_exception
    ([&cache_man, id, elem_size]()
     {
      cache_man.add_chunk_list_to_cache
              ( gen_cache_elems_list
                  ( std::unordered_set<unsigned long>({id})
                  , elem_size
                  ));
     }
    , std::runtime_error ("At least one cached buffer is larger than the cache size")
    );

  elem_size = 30;
  auto elems_to_add = gen_cache_elems_list
      ( std::unordered_set<unsigned long>({0, 1, 2, 3})
      , elem_size
      );

  fhg::util::testing::require_exception
    ([&cache_man, &elems_to_add]()
     {
      cache_man.add_chunk_list_to_cache(elems_to_add);
     }
    , std::runtime_error (( boost::format
        ( "The cached memory buffers do not fit in cache free=%1%, begin=%2% end=%3%")
        % std::to_string(90) %  std::to_string(0)
        %  std::to_string(0)
    ).str())
    );

  unsigned long expected_offset = 0;
  elem_size = 50;
  id = 2000;

  cache_man.clear();
  test_add_to_cache_one_new_elem(&cache_man, id, elem_size, expected_offset);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(id)), true);

  elem_size = 30;
  elems_to_add = gen_cache_elems_list
      ( std::unordered_set<unsigned long>({11, 12, 13})
      , elem_size
      );

  fhg::util::testing::require_exception
    ([&cache_man, &elems_to_add]()
     {
      cache_man.add_chunk_list_to_cache(elems_to_add);
     }
    , std::runtime_error (( boost::format
        ( "The cached memory buffers do not fit in cache free=%1%, begin=%2% end=%3%")
        % std::to_string(80) %  std::to_string(0)
        %  std::to_string(0)
    ).str())
    );
}

BOOST_AUTO_TEST_CASE (test_already_cached_elems)
{
  unsigned long size = 80;
  unsigned long elem_size = 10;
  drts::cache::circular_cache_manager cache_man(size);

  test_add_to_cache_multiple_elems
    ( &cache_man
    , gen_cache_elems_list(std::unordered_set<unsigned long>({0, 1, 2, 3}), elem_size)
    , {});
  // cached elements now: 0, 1, 2, 3

  test_add_to_cache_multiple_elems
    ( &cache_man
    , gen_cache_elems_list(std::unordered_set<unsigned long>({1, 3, 4, 5, 6}), elem_size)
    , gen_dataid_list({1, 3}));
  // cached elements now: 0, 1, 2, 3, 4, 5, 6

  test_add_to_cache_multiple_elems
    ( &cache_man
    , gen_cache_elems_list(std::unordered_set<unsigned long>({0, 2, 3, 4, 5, 6, 7, 8}), elem_size)
    , gen_dataid_list({0, 2, 3, 4, 5, 6}));
  // cached elements now: 0, 2, 3, 4, 5, 6, 7, 8

  // the cache is full, it is not known which elements will be evicted,
  // unless only one can be evicted for a given list
  // replace elem 4 with 100
  test_add_to_cache_multiple_elems
    ( &cache_man
    , gen_cache_elems_list(std::unordered_set<unsigned long>({0, 2, 3, 5, 6, 7, 8, 100}), elem_size)
    , gen_dataid_list({0, 2, 3, 5, 6, 7, 8}));

  // replace elem 0 with 101
  test_add_to_cache_multiple_elems
    ( &cache_man
    , gen_cache_elems_list(std::unordered_set<unsigned long>({2, 3, 5, 6, 7, 8, 100, 101}), elem_size)
    , gen_dataid_list({100, 2, 3, 5, 6, 7, 8}));

  test_add_to_cache_multiple_elems
    ( &cache_man
    , gen_cache_elems_list(std::unordered_set<unsigned long>({10, 11, 12}), elem_size)
    , {});
}

BOOST_AUTO_TEST_CASE (test_multiple_sizes)
{
  unsigned long size = 90;
  unsigned long elem_size = 300;
  drts::cache::circular_cache_manager cache_man(size);

  unsigned long expected_offset = 0;
  unsigned long id = 1000;
  elem_size = 50;

  test_add_to_cache_one_new_elem(&cache_man, id, elem_size, expected_offset);

  elem_size = 30;
  test_add_to_cache_one_new_elem(&cache_man, 0, elem_size, 50);
  test_add_to_cache_one_new_elem(&cache_man, 1, elem_size, 0);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(0)), true);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(1)), true);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(id)), false);

  elem_size = 15;
  test_add_to_cache_one_new_elem(&cache_man, 2000, elem_size, 30);
  elem_size = 7;
  test_add_to_cache_one_new_elem(&cache_man, 2001, elem_size, 80);
  elem_size = 4;
  test_add_to_cache_one_new_elem(&cache_man, 2002, elem_size, 45);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(0)), true);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(1)), true);

  elem_size = 4;
  test_add_to_cache_one_new_elem(&cache_man, 2003, elem_size, 49);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(0)), false);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(1)), true);

  elem_size = 4;
  test_add_to_cache_one_new_elem(&cache_man, 2004, elem_size, 53);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(0)), false);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(1)), true);
}


