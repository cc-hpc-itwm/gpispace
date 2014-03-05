#define BOOST_TEST_MODULE SynchronizedQueue

#include <sdpa/daemon/SynchronizedQueue.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (testQueue)
{
	sdpa::daemon::SynchronizedQueue<std::list<std::size_t> > test_queue;

	for (std::size_t cnt(0); cnt < 10; ++cnt)
	{
		test_queue.push (cnt);
  }

  for (std::size_t cnt (0); cnt < 10; ++cnt)
  {
    BOOST_REQUIRE_EQUAL (test_queue.pop(), cnt);
  }

  BOOST_REQUIRE_EQUAL (test_queue.pop(), boost::none);
}
