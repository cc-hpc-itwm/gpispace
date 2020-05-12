#include <boost/test/unit_test.hpp>

#include <stdlib.h>
#include <iostream>

#include <list>
#include <thread>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/thread/queue.hpp>

#include <functional>

typedef fhg::thread::queue<int> items_t;

static void fill_items (items_t & items, int NUM)
{
  for (int i = 0 ; i < NUM ; ++i)
  {
    items.put (i);
  }
}

BOOST_AUTO_TEST_CASE (thread_queue_put_get)
{
  static const int NUM_ITEMS_TO_PUT = 1000;

  items_t items;

  fill_items (items, NUM_ITEMS_TO_PUT);

  for (int i = 0 ; i < NUM_ITEMS_TO_PUT ; ++i)
  {
    int item = items.get ();
    BOOST_REQUIRE_EQUAL (i, item);
  }
}

BOOST_AUTO_TEST_CASE (thread_queue_put_by_other_thread)
{
  static const int NUM_ITEMS_TO_PUT = 10000;

  items_t items;

  std::thread filler = std::thread ( &fill_items
                                   , std::ref (items)
                                   , NUM_ITEMS_TO_PUT
                                   );

  for (int i = 0 ; i < NUM_ITEMS_TO_PUT ; ++i)
  {
    int item = items.get ();
    BOOST_REQUIRE_EQUAL (i, item);
  }

  filler.join ();
}

BOOST_AUTO_TEST_CASE (thread_queue_put_by_multiple_threads)
{
  static const int NUM_ITEMS_TO_PUT = 10000;
  static const int NUM_THREADS = 10;

  items_t items;

  std::vector<std::thread*> threads;
  for (int i = 0 ; i < NUM_THREADS ; i++)
  {
    threads.push_back (new std::thread ( &fill_items
                                       , std::ref (items)
                                       , NUM_ITEMS_TO_PUT
                                       )
                      );
  }

  size_t items_consumed = 0;
  for (int i = 0 ; i < NUM_THREADS*NUM_ITEMS_TO_PUT ; ++i)
  {
    items.get ();
    ++items_consumed;
  }
  BOOST_REQUIRE_EQUAL ( items_consumed
                      , (size_t)(NUM_ITEMS_TO_PUT * NUM_THREADS)
                      );

  for (int i = 0 ; i < NUM_THREADS ; i++)
  {
    threads [i]->join ();
    delete threads [i]; threads [i] = nullptr;
  }
}
