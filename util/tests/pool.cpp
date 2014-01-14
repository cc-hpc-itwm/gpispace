#define BOOST_TEST_MODULE FhgUtilThreadPool
#include <boost/test/unit_test.hpp>

#include <fhg/util/thread/atomic.hpp>
#include <fhg/util/thread/pool.hpp>

namespace
{
  void sleep_and_increase (fhg::thread::atomic<int>* counter)
  {
    usleep (rand () % 500);
    ++*counter;
  }
}

BOOST_AUTO_TEST_CASE (simple_pool)
{
  static const size_t NUM_ITERATIONS = 100;

  using namespace fhg::thread;

  fhg::thread::atomic<int> counter (0);

  {
    pool_t pool (13);

    for (size_t i = 0 ; i < NUM_ITERATIONS; ++i)
      pool.execute (boost::bind (&sleep_and_increase, &counter));
  }

  BOOST_REQUIRE_EQUAL (counter, NUM_ITERATIONS);
}
