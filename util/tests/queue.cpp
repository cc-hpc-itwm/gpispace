#define BOOST_TEST_MODULE UtilThreadQueueTest
#include <boost/test/unit_test.hpp>

#include <stdlib.h>
#include <iostream>

#include <list>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <fhg/util/thread/queue.hpp>

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

  boost::thread filler = boost::thread ( &fill_items
                                       , boost::ref (items)
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

  std::vector<boost::thread*> threads;
  for (int i = 0 ; i < NUM_THREADS ; i++)
  {
    threads.push_back (new boost::thread ( &fill_items
                                         , boost::ref (items)
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
    delete threads [i]; threads [i] = 0;
  }
}

BOOST_AUTO_TEST_CASE (thread_queue_INDICATES_A_RACE_clear)
{
  static const int NUM_ITEMS_TO_PUT = 10000;

  items_t items;

  fill_items (items, NUM_ITEMS_TO_PUT);

  BOOST_REQUIRE_EQUAL ( items.INDICATES_A_RACE_size ()
                      , static_cast<items_t::size_type>(NUM_ITEMS_TO_PUT)
                      );

  items.INDICATES_A_RACE_clear ();

  BOOST_REQUIRE_EQUAL (items.INDICATES_A_RACE_size (), 0u);
}
