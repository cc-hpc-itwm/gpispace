#define BOOST_TEST_MODULE SynchronizedQueue

#include <sdpa/sdpa/daemon/SynchronizedQueue.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include <iostream>

namespace
{
  typedef sdpa::daemon::SynchronizedQueue<std::list<int> > queue_type;

  struct thread_data
  {
    thread_data(queue_type *a_q) : q(a_q), val(0) {}
    void operator()()
    {
      try
      {
        for (;;)
        {
          val += q->pop_and_wait();
          std::cout << "-";
        }
      }
      catch (const boost::thread_interrupted &irq)
      {
        std::cout << "x";
      }
    }
    queue_type *q;
    int val;
  };
}

BOOST_AUTO_TEST_CASE (testQueue)
{
	queue_type test_queue;
	thread_data thrd_data (&test_queue);
	boost::thread thrd (boost::ref (thrd_data));

	for (std::size_t cnt(0); cnt < 10; ++cnt)
	{
    std::cout << "+";
		test_queue.push (42);

		if (cnt % 2 == 0)
			boost::this_thread::sleep (boost::posix_time::seconds(1));
  }

  thrd.interrupt();
  if (thrd.joinable())
	  thrd.join();

  BOOST_REQUIRE_GE (thrd_data.val, 42);

  while (!test_queue.empty())
  {
    test_queue.pop();
  }

  BOOST_REQUIRE_THROW (test_queue.pop(), sdpa::daemon::QueueEmpty);
}
