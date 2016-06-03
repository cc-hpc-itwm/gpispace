// tiberiu.rotaru@itwm.fraunhofer.de

#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/thread/bounded_queue.hpp>

#include <functional>
#include <thread>
#include <vector>

namespace
{
  void produce_items ( fhg::thread::bounded_queue<unsigned int>& items
                     , unsigned int amount
                     )
  {
    for (unsigned int k = 0; k < amount; ++k)
    {
      items.try_put (k);
    }
  }
}

BOOST_AUTO_TEST_CASE (thread_bounded_queue_test_order_and_became_not_full_cond)
{
  const unsigned int num_items = 1000;
  const std::size_t capacity (100);

  fhg::thread::bounded_queue<unsigned int> items (capacity);

  for (unsigned int k = 0; k < num_items; ++k)
  {
    if (k < items.capacity())
    {
      BOOST_REQUIRE (items.try_put (k));
    }
    else
    {
      BOOST_REQUIRE (!items.try_put (k));
    }
  }

  bool is_first_item (true);
  for (unsigned int k = 0; k < items.capacity(); ++k)
  {
    unsigned int item;
    bool became_not_full;

    std::tie (item, became_not_full) = items.get();

    BOOST_REQUIRE_EQUAL (k, item);

    if (is_first_item)
    {
      BOOST_REQUIRE (became_not_full);
      is_first_item = false;
    }
    else
    {
      BOOST_REQUIRE (!became_not_full);
    }
  }
}

BOOST_AUTO_TEST_CASE (thread_bounded_queue_test_expected_order_several_producer_threads)
{
  const unsigned int num_items = 1000;
  const std::size_t capacity (100);

  fhg::thread::bounded_queue<unsigned int>  items (capacity);

  boost::strict_scoped_thread<> const
    producer (&produce_items, boost::ref (items), num_items);

  for (unsigned int k = 0; k < items.capacity(); ++k)
  {
    BOOST_REQUIRE_EQUAL (k, items.get().first);
  }
}

BOOST_AUTO_TEST_CASE (thread_bounded_queue_try_put_by_multiple_threads)
{
  const std::size_t capacity (10);
  constexpr unsigned int num_producers = capacity + 1;
  std::vector<unsigned int> try_put_results (num_producers, 0);

  fhg::thread::bounded_queue<unsigned int> items (capacity);

  boost::thread_group producers;

  for (unsigned int k = 0; k < num_producers; k++)
  {
    producers.create_thread
      ( [&items, &try_put_results, k]
        {
          if (items.try_put (k))
          {
            try_put_results[k] = 1;
          }
        }
      );
  }

  producers.join_all();

  BOOST_REQUIRE_EQUAL ( items.capacity()
                      , std::accumulate
                        ( try_put_results.begin()
                        , try_put_results.end()
                        , 0
                        )
                      );

  BOOST_REQUIRE (items.get().second);
  for (std::size_t k (1); k < items.capacity(); ++k)
  {
    BOOST_REQUIRE (!items.get().second);
  }
}
