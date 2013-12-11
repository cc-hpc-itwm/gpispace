#define BOOST_TEST_MODULE FhgUtilThreadPool
#include <boost/test/unit_test.hpp>

#include <stdlib.h>

#include <iostream>

#include <vector>

#include <fhg/util/thread/atomic.hpp>
#include <fhg/util/get_cpucount.h>
#include <fhg/util/thread/pool.hpp>
#include <fhg/util/thread/async.hpp>

static fhg::thread::atomic<int> s_work_count (0);

static void s_work ()
{
  usleep (rand () % 500);
}

static void s_work_callback ()
{
  ++s_work_count;
}

BOOST_AUTO_TEST_CASE (simple_pool)
{
  static const size_t NUM_ITERATIONS = 100;

  using namespace fhg::thread;
  int ncpu = fhg_get_cpucount ();
  if (ncpu < 0)
    ncpu = 1;

  pool_t pool (ncpu);
  pool.start ();

  s_work_count = 0;
  for (size_t i = 0 ; i < NUM_ITERATIONS; ++i)
    pool.execute (s_work, s_work_callback);

  pool.stop ();

  BOOST_REQUIRE_EQUAL (s_work_count, NUM_ITERATIONS);
}

BOOST_AUTO_TEST_CASE (async_test)
{
  static const size_t NUM_ITERATIONS = 100;

  using namespace fhg::thread;

  s_work_count = 0;
  for (size_t i = 0 ; i < NUM_ITERATIONS; ++i)
    async (s_work, s_work_callback);

  while (s_work_count != NUM_ITERATIONS)
    usleep (10);

  BOOST_REQUIRE_EQUAL (s_work_count, NUM_ITERATIONS);
}
