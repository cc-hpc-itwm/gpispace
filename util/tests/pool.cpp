#define BOOST_TEST_MODULE FhgUtilThreadPool
#include <boost/test/unit_test.hpp>

#include <stdlib.h>

#include <iostream>

#include <vector>

#include <fhg/util/thread/atomic.hpp>
#include <fhg/util/thread/pool.hpp>

static void s_work (fhg::thread::atomic<int>* counter)
{
  usleep (rand () % 500);
  ++*counter;
}

BOOST_AUTO_TEST_CASE (simple_pool)
{
  static const size_t NUM_ITERATIONS = 100;

  using namespace fhg::thread;

  fhg::thread::atomic<int> counter (0);

  {
    pool_t pool (13);

    for (size_t i = 0 ; i < NUM_ITERATIONS; ++i)
      pool.execute (boost::bind (&s_work, &counter));
  }

  BOOST_REQUIRE_EQUAL (counter, NUM_ITERATIONS);
}
