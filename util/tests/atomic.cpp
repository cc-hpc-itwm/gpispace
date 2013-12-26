#define BOOST_TEST_MODULE FhgUtilAtomicCounter
#include <boost/test/unit_test.hpp>

#include <fhg/util/thread/atomic.hpp>

#include <boost/thread.hpp>

BOOST_AUTO_TEST_CASE (empty_is_zero)
{
  fhg::thread::atomic<int> atom;

  BOOST_REQUIRE_EQUAL (atom, 0);
}

BOOST_AUTO_TEST_CASE (increment)
{
  fhg::thread::atomic<int> atom;
  ++atom;
  BOOST_REQUIRE_EQUAL (atom, 1);
}

namespace
{
  void inc (fhg::thread::atomic<int>& a, int N)
  {
    while (N --> 0)
    {
      ++a;
    }
  }
}

BOOST_AUTO_TEST_CASE (threaded_increment)
{
  static const size_t NUM_THREADS (4);
  static const size_t NUM_ITERATIONS (1000000);

  fhg::thread::atomic<int> atom;

  boost::thread_group threads;

  for (size_t i (0); i < NUM_THREADS; ++i)
  {
    threads.create_thread
      (boost::bind (&inc, boost::ref (atom), NUM_ITERATIONS));
  }

  threads.join_all();

  BOOST_REQUIRE_EQUAL (atom, NUM_THREADS * NUM_ITERATIONS);
}
