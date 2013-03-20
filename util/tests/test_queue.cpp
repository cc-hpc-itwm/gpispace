#define BOOST_TEST_MODULE UtilThreadQueueTest
#include <boost/test/unit_test.hpp>

#include <stdlib.h>
#include <iostream>

#include <list>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <fhg/util/thread/queue.hpp>

typedef fhg::thread::queue<int, std::list> items_t;

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

BOOST_AUTO_TEST_CASE (thread_queue_timed_get_empty_queue)
{
  items_t items;

  try
  {
    BOOST_REQUIRE (items.empty ());
    items.get (boost::posix_time::milliseconds (500));
    BOOST_ERROR ("thread::queue::get(500ms) did not timeout");
  }
  catch (fhg::thread::operation_timedout const &)
  {
    // expected
  }
}

BOOST_AUTO_TEST_CASE (thread_queue_timed_get_nonempty_queue)
{
  items_t items;
  items.put (0);

  try
  {
    BOOST_REQUIRE (not items.empty ());
    int item = items.get (boost::posix_time::milliseconds (500));
    BOOST_REQUIRE_EQUAL (item, 0);
  }
  catch (fhg::thread::operation_timedout const &)
  {
    BOOST_ERROR ("thread::queue::get(500ms) from empty queue timed out");
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

static bool is_even (int i)
{
  return (i % 2) == 0;
}

BOOST_AUTO_TEST_CASE (thread_queue_remove_if)
{
  static const int NUM_ITEMS_TO_PUT = 10000;

  items_t items;
  fill_items (items, NUM_ITEMS_TO_PUT);

  BOOST_REQUIRE_EQUAL ( items.size()
                      , static_cast<items_t::size_type>(NUM_ITEMS_TO_PUT)
                      );

  std::size_t num_removed = items.remove_if (is_even);

  BOOST_REQUIRE_EQUAL ( num_removed
                      , static_cast<items_t::size_type>(NUM_ITEMS_TO_PUT / 2)
                      );
  BOOST_REQUIRE_EQUAL ( items.size ()
                      , static_cast<items_t::size_type>(NUM_ITEMS_TO_PUT / 2)
                      );
}

BOOST_AUTO_TEST_CASE (thread_queue_erase)
{
  static const int NUM_ITEMS_TO_PUT = 10000;

  items_t items;
  fill_items (items, NUM_ITEMS_TO_PUT);

  BOOST_REQUIRE_EQUAL ( items.size ()
                      , static_cast<items_t::size_type>(NUM_ITEMS_TO_PUT)
                      );

  for (int i = 0 ; i < NUM_ITEMS_TO_PUT ; i++)
  {
    size_t num_erased = items.erase (i);
    BOOST_REQUIRE_EQUAL (num_erased, 1u);
    BOOST_REQUIRE_EQUAL
      ( items.size ()
      , static_cast<items_t::size_type>(NUM_ITEMS_TO_PUT - i - 1)
      );
  }
}

BOOST_AUTO_TEST_CASE (thread_queue_clear)
{
  static const int NUM_ITEMS_TO_PUT = 10000;

  items_t items;

  fill_items (items, NUM_ITEMS_TO_PUT);

  BOOST_REQUIRE_EQUAL ( items.size ()
                      , static_cast<items_t::size_type>(NUM_ITEMS_TO_PUT)
                      );

  items.clear ();

  BOOST_REQUIRE_EQUAL (items.size (), 0);
  BOOST_REQUIRE       (items.empty ());
}

BOOST_AUTO_TEST_CASE (thread_queue_timed_put_empty_queue)
{
  items_t items (1);

  try
  {
    BOOST_REQUIRE (items.empty ());
    items.put (0, boost::posix_time::milliseconds (500));
  }
  catch (fhg::thread::operation_timedout const &)
  {
    BOOST_ERROR ("thread::queue::put(0, 500ms) timed out");
  }
}

BOOST_AUTO_TEST_CASE (thread_queue_timed_put_full_queue)
{
  items_t items (1);

  try
  {
    BOOST_REQUIRE (items.empty ());

    items.put (0, boost::posix_time::milliseconds (500));
    BOOST_REQUIRE_EQUAL (items.size (), 1u);

    items.put (1, boost::posix_time::milliseconds (500));

    BOOST_ERROR ("thread::queue::put(1, 500ms) did not timeout");
  }
  catch (fhg::thread::operation_timedout const &)
  {
    // expected
  }
}
