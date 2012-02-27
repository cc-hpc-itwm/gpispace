#define BOOST_TEST_MODULE TestWorker
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/JobId.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <sdpa/logging.hpp>

using namespace sdpa::daemon;
using namespace sdpa;

struct MyFixture
{
	MyFixture() :SDPA_INIT_LOGGER("sdpa.tests.testWorker"){}
	~MyFixture(){}
	 SDPA_DECLARE_LOGGER();

	 typedef SynchronizedQueue<std::list<int> > queue_type;

	 struct thread_data
	 {
	   thread_data(queue_type *a_q) : q(a_q), val(0) {}
	   void operator()() {
	     try
	     {
	       for (;;)
	       {
	         val += q->pop_and_wait();
	         std::cout<<"-";
	       }
	     }
	     catch (const boost::thread_interrupted &irq)
	     {
	    	 std::cout<<"x";
	     }
	   }
	   queue_type *q;
	   int val;
	 };
};

BOOST_FIXTURE_TEST_SUITE( test_Worker, MyFixture )

BOOST_AUTO_TEST_CASE(testDispatch) {
  Worker worker("w0", 100);
  BOOST_CHECK(worker.pending().empty());

  job_id_t jobId("1");
  worker.dispatch(jobId);

  BOOST_CHECK(!worker.pending().empty());
  BOOST_CHECK( *worker.pending().begin() == jobId );
  worker.pending().clear();
}

BOOST_AUTO_TEST_CASE(testGetNextJob) {
  Worker worker("w0", 100);

  job_id_t jobId("1");
  worker.dispatch(jobId);

  BOOST_CHECK(!worker.pending().empty());

  job_id_t jobIdNext = worker.get_next_job(sdpa::job_id_t::invalid_job_id());
  BOOST_CHECK(worker.pending().empty()); // pending is empty now
  BOOST_CHECK(!worker.submitted().empty()); // submitted has one job
  BOOST_CHECK(jobIdNext == jobId);
}

BOOST_AUTO_TEST_CASE(testAcknowledge)
{
  Worker worker("w0", 100);

  job_id_t jobId("1");
  worker.dispatch(jobId);

  BOOST_CHECK(! worker.pending().empty());

  job_id_t jobIdNext = worker.get_next_job(sdpa::job_id_t::invalid_job_id());
  BOOST_CHECK(worker.pending().empty()); // pending is empty
  BOOST_CHECK(! worker.submitted().empty()); // submitted is not empty anymore

  bool ackResult = worker.acknowledge(jobIdNext);
  BOOST_CHECK(ackResult);
  BOOST_CHECK(worker.pending().empty()); // pending still empty
  BOOST_CHECK(worker.submitted().empty()); // submitted is now empty
  BOOST_CHECK(!worker.acknowledged().empty()); // added to acknowledged
}
  
BOOST_AUTO_TEST_CASE(testQueue)
{
	SDPA_LOG_INFO("testing synchronized queue...");
	queue_type test_queue;
	thread_data thrd_data(&test_queue);
	boost::thread thrd(boost::ref(thrd_data));

	SDPA_LOG_INFO("pushing...");
	for (std::size_t cnt(0); cnt < 10; ++cnt)
	{
		SDPA_LOG_INFO("+");
		test_queue.push(42);

		if (cnt % 2 == 0)
			boost::this_thread::sleep(boost::posix_time::seconds(1));
  }

  thrd.interrupt();
  thrd.join();

  SDPA_LOG_INFO(thrd_data.val);

  BOOST_CHECK(thrd_data.val >= 42);

  SDPA_LOG_INFO("removing queue entries...");

  while (! test_queue.empty()) test_queue.pop();
  SDPA_LOG_INFO("done.");

  try
  {
	  SDPA_LOG_INFO("popping from empty queue");
	  test_queue.pop_and_wait(boost::posix_time::milliseconds(10));
	  SDPA_LOG_INFO("FAILED");
  }
  catch (const QueueEmpty &)
  {
	  SDPA_LOG_INFO("OK");
  }
}

BOOST_AUTO_TEST_SUITE_END()
