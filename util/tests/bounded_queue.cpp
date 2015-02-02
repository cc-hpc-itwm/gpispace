// tiberiu.rotaru@itwm.fraunhofer.de
#define BOOST_TEST_MODULE UtilThreadBoundedQueueTest
#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
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

  boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable> const
    producer (&produce_items, boost::ref (items), num_items);

  for (unsigned int k = 0; k < items.capacity(); ++k)
  {
    BOOST_REQUIRE_EQUAL (k, items.get().first);
  }
}

struct fixture_multiple_producers
{
  boost::mutex mtx;
  std::vector<unsigned int> order;

  void produce_one_item ( fhg::thread::bounded_queue<unsigned int>& items
                        , unsigned int item
                        , std::vector<unsigned int>& try_put_results
                        )
  {
    boost::unique_lock<boost::mutex> _ (mtx);
    if (items.try_put (item))
    {
      try_put_results[item] = 1;
      order.push_back (item);
    }
  }
};

BOOST_FIXTURE_TEST_CASE ( thread_bounded_queue_try_put_by_multiple_threads
                        , fixture_multiple_producers
                        )
{
  const std::size_t capacity (10);
  constexpr unsigned int num_producers = capacity + 1;
  std::vector<unsigned int> try_put_results (num_producers, 0);

  fhg::thread::bounded_queue<unsigned int> items (capacity);

  std::vector<boost::thread> producers;
  for (unsigned int k = 0; k < num_producers; k++)
  {
    producers.emplace_back ( &fixture_multiple_producers::produce_one_item
                           , this
                           , boost::ref (items)
                           , k
                           , boost::ref (try_put_results)
                           );
  }

  std::for_each ( producers.begin(), producers.end(), [](boost::thread &t)
                                                      {t.join();}
                );

  BOOST_REQUIRE_EQUAL ( items.capacity()
                      , std::accumulate
                        ( try_put_results.begin()
                        , try_put_results.end()
                        , 0
                        )
                      );

  bool is_first_item (true);
  for (unsigned int item : order)
  {
    std::pair<unsigned int, bool> head (items.get());
    BOOST_REQUIRE_EQUAL (item, head.first);
    if (is_first_item)
    {
      BOOST_REQUIRE ((is_first_item = false, head.second));
    }
    else
    {
      BOOST_REQUIRE (!head.second);
    }
  }
}
