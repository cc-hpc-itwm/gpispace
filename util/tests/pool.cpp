#define BOOST_TEST_MODULE FhgUtilThreadPool
#include <boost/test/unit_test.hpp>

#include <stdlib.h>

#include <iostream>

#include <vector>

#include <fhg/util/thread/atomic.hpp>
#include <fhg/util/thread/pool.hpp>

static fhg::thread::atomic<int> s_work_count (0);

static void s_work ()
{
  usleep (rand () % 500);
  ++s_work_count;
}

BOOST_AUTO_TEST_CASE (simple_pool)
{
  static const size_t NUM_ITERATIONS = 100;

  using namespace fhg::thread;

  {
    pool_t pool (13);

    for (size_t i = 0 ; i < NUM_ITERATIONS; ++i)
      pool.execute (s_work);
  }

  BOOST_REQUIRE_EQUAL (s_work_count, NUM_ITERATIONS);
}
