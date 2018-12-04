#include <boost/test/unit_test.hpp>

#include <drts/cache_management/cache_manager.hpp>
#include <drts/cache_management/circular_cache_manager.hpp>
#include <drts/cache_management/error.hpp>

#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/string.hpp>

#include <unordered_set>
#include <unordered_map>
#include <string>

namespace
{
  drts::cache::Dataid gen_dataid(unsigned long id)
  {
    return std::to_string(id);
  }

  std::unordered_map<drts::cache::Dataid, unsigned long> gen_cache_elems_list
    ( std::vector<unsigned long> ids
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
        ( gen_cache_elems_list ({id}, elem_size));

    BOOST_REQUIRE_EQUAL (already_in_cache.size(), 0);
    BOOST_REQUIRE_EQUAL (cache_man->is_cached(gen_dataid(id)), true);
    BOOST_REQUIRE_EQUAL (cache_man->offset(gen_dataid(id)), expected_offset);
  }

  void test_add_to_cache_multiple_elems
    ( drts::cache::cache_manager* const cache_man
    , std::vector<unsigned long> elems_to_add
    , std::unordered_set<unsigned long> expected_already_cached_ids
    , unsigned long elem_size
    )
  {
    auto already_in_cache = cache_man->add_chunk_list_to_cache(
        gen_cache_elems_list(elems_to_add, elem_size));

    BOOST_REQUIRE_EQUAL (already_in_cache.size(), expected_already_cached_ids.size());
    for (auto elem : expected_already_cached_ids)
    {
      BOOST_REQUIRE_EQUAL (already_in_cache.count(gen_dataid(elem)), 1);
    }
    for (auto elem : elems_to_add)
    {
      BOOST_REQUIRE_EQUAL (cache_man->is_cached(gen_dataid(elem)), true);
    }
  }
}


BOOST_AUTO_TEST_CASE (test_simple_add)
{
  auto size = 100;
  auto nelems = 10;
  fhg::util::testing::unique_random<unsigned long> dataid_pool;
  drts::cache::circular_cache_manager cache_man(size);
  BOOST_REQUIRE_EQUAL (cache_man.size(), size);

  // fill the cache
  {
    auto elem_size = size / 10;
    auto expected_offset = 0;
    for (auto i(0); i < nelems; ++i)
    {
      test_add_to_cache_one_new_elem( &cache_man
                                    , dataid_pool()
                                    , elem_size
                                    , expected_offset);
      expected_offset += elem_size;
    }
  }

  // reset offset as older cache entries are replaced when the cache is full
  {
    auto elem_size = size / 10;
    auto expected_offset = 0;
    for (auto i(0); i < nelems; ++i)
    {
      test_add_to_cache_one_new_elem( &cache_man
                                    , dataid_pool()
                                    , elem_size
                                    , expected_offset);
      expected_offset += elem_size;
    }
  }

  // add larger cache buffer
  {
    auto elem_size = size / 2;
    auto expected_offset = 0;
    test_add_to_cache_one_new_elem( &cache_man
                                  , dataid_pool()
                                  , elem_size
                                  , expected_offset);
  }

  // reset offset when older cache entries are evicted
  {
    auto elem_size = size / 10;
    auto expected_offset = 50;
    for (auto i (0); i < nelems; ++i)
    {
      test_add_to_cache_one_new_elem( &cache_man
                                    , dataid_pool()
                                    , elem_size
                                    , expected_offset);
      expected_offset = (expected_offset + elem_size) % size;
    }
  }

  {
    auto elem_size = size / 2;
    auto expected_offset = 50;
    test_add_to_cache_one_new_elem( &cache_man
                                  , dataid_pool()
                                  , elem_size
                                  , expected_offset);
  }
}


BOOST_AUTO_TEST_CASE (test_exceptions_simple_caching)
{
  auto size = 100;
  auto elem_size = 300;
  fhg::util::testing::unique_random<unsigned long> dataid_pool;
  drts::cache::circular_cache_manager cache_man(size);

  auto id {dataid_pool()};
  fhg::util::testing::require_exception_with_message
    <drts::cache::error::buffer_larger_than_cache_size>
    ([&cache_man, id, elem_size]()
     {
      cache_man.add_chunk_list_to_cache (gen_cache_elems_list ({id}, elem_size));
     }
    , boost::format( "ERROR: Buffer size is larger than the cache size: ! (%1% > %2%)")
    % elem_size
    % size
    );


  fhg::util::testing::require_exception_with_message
    <drts::cache::error::buffer_not_in_cache>
    ([&cache_man, id]()
     {
      cache_man.offset (gen_dataid(id));
     }
    , boost::format( "ERROR: The required buffer with id=%1% is not cached")
    % gen_dataid(id)
    );

  {
    auto elem_size = 30;
    auto nelems = 4;
    auto elems_to_add = gen_cache_elems_list
        ( fhg::util::testing::randoms< std::vector<unsigned long>
                                     , fhg::util::testing::unique_random
                                     > (nelems)
        , elem_size
        );

    fhg::util::testing::require_exception_with_message
      <drts::cache::error::insufficient_space_available_in_cache>
      ([&cache_man, &elems_to_add]()
       {
        cache_man.add_chunk_list_to_cache(elems_to_add);
       }
      , boost::format (
          "ERROR: The cached memory buffers do not fit in cache (cache_size=%1%, offset_free=%2%, gap_begin=%3%, gap_end=%4%)")
      % std::to_string(size)
      % std::to_string(90)
      % std::to_string(0)
      % std::to_string(0)
      );
  }
}

BOOST_AUTO_TEST_CASE (test_exceptions_cache_after_eviction)
{
  auto size = 100;
  fhg::util::testing::unique_random<unsigned long> dataid_pool;
  drts::cache::circular_cache_manager cache_man(size);

  auto expected_offset = 0;
  auto elem_size = 50;
  auto id {dataid_pool()};

  test_add_to_cache_one_new_elem(&cache_man, id, elem_size, expected_offset);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(id)), true);

  {
    auto nelems = 3;
    auto elem_size = 30;
    auto elems_to_add = gen_cache_elems_list
        ( fhg::util::testing::randoms< std::vector<unsigned long>
                                     , fhg::util::testing::unique_random
                                     > (nelems)
        , elem_size
        );

    fhg::util::testing::require_exception_with_message
    <drts::cache::error::insufficient_space_available_in_cache>
      ([&cache_man, &elems_to_add]()
       {
        cache_man.add_chunk_list_to_cache(elems_to_add);
       }
      , boost::format ( "ERROR: The cached memory buffers do not fit in cache (cache_size=%1%, offset_free=%2%, gap_begin=%3%, gap_end=%4%)")
      % std::to_string(size)
      % std::to_string(80)
      % std::to_string(0)
      % std::to_string(0)
      );
  }
}

BOOST_AUTO_TEST_CASE (test_already_cached_elems)
{
  auto size = 80;
  auto elem_size = 10;
  drts::cache::circular_cache_manager cache_man(size);

  auto nelems = 15;
  auto ids { fhg::util::testing::randoms< std::vector<unsigned long>
                                        , fhg::util::testing::unique_random
                                        > (nelems)
           };

  test_add_to_cache_multiple_elems
    ( &cache_man
    , {ids[0], ids[1], ids[2], ids[3]}
    , {}
    , elem_size);
  // cached elements now: 0, 1, 2, 3

  test_add_to_cache_multiple_elems
    ( &cache_man
    , {ids[1], ids[3], ids[4], ids[5], ids[6]}
    , {ids[1], ids[3]}
    , elem_size);
  // cached elements now: 0, 1, 2, 3, 4, 5, 6

  // the cache is full, it is not known which elements will be evicted,
  // unless only one can be evicted for a given list
  // replace elem 1 with 8
  test_add_to_cache_multiple_elems
    ( &cache_man
    , {ids[0], ids[2], ids[3], ids[4], ids[5], ids[6], ids[7], ids[8]}
    , {ids[0], ids[2], ids[3], ids[4], ids[5], ids[6]}
    , elem_size);
  // cached elements now: 0, 2, 3, 4, 5, 6, 7, 8

  // replace elem 4 with 9
  test_add_to_cache_multiple_elems
    ( &cache_man
    , {ids[0], ids[2], ids[3], ids[5], ids[6], ids[7], ids[8], ids[9]}
    , {ids[0], ids[2], ids[3], ids[5], ids[6], ids[7], ids[8]}
    , elem_size);

  // replace elem 0 with 10
  test_add_to_cache_multiple_elems
    ( &cache_man
    , {ids[2], ids[3], ids[5], ids[6], ids[7], ids[8], ids[9], ids[10]}
    , {ids[2], ids[3], ids[5], ids[6], ids[7], ids[8], ids[9]}
    , elem_size);

  // add more new elements
  test_add_to_cache_multiple_elems
    ( &cache_man
    , {ids[11], ids[12], ids[13], ids[14]}
    , {}
    , elem_size);
}

BOOST_AUTO_TEST_CASE (test_multiple_sizes)
{
  auto size = 90;
  drts::cache::circular_cache_manager cache_man(size);

  auto nelems = 8;
  auto ids{ fhg::util::testing::randoms< std::vector<unsigned long>
                                       , fhg::util::testing::unique_random
                                       > (nelems)
          };

  auto elem_size = 50;
  test_add_to_cache_one_new_elem(&cache_man, ids[0], elem_size, 0);

  elem_size = 30;
  test_add_to_cache_one_new_elem(&cache_man, ids[1], elem_size, 50);
  test_add_to_cache_one_new_elem(&cache_man, ids[2], elem_size, 0);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(ids[1])), true);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(ids[2])), true);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(ids[0])), false);

  elem_size = 15;
  test_add_to_cache_one_new_elem(&cache_man, ids[3], elem_size, 30);
  elem_size = 7;
  test_add_to_cache_one_new_elem(&cache_man, ids[4], elem_size, 80);
  elem_size = 4;
  test_add_to_cache_one_new_elem(&cache_man, ids[5], elem_size, 45);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(ids[1])), true);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(ids[2])), true);

  elem_size = 4;
  test_add_to_cache_one_new_elem(&cache_man, ids[6], elem_size, 49);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(ids[1])), false);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(ids[2])), true);

  elem_size = 4;
  test_add_to_cache_one_new_elem(&cache_man, ids[7], elem_size, 53);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(ids[1])), false);
  BOOST_REQUIRE_EQUAL (cache_man.is_cached(gen_dataid(ids[2])), true);
}


