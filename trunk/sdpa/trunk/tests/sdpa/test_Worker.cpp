#include <iostream>

#include "test_Worker.hpp"
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

using namespace sdpa::tests;
using namespace sdpa::daemon;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::WorkerTest );

WorkerTest::WorkerTest() {
}

WorkerTest::~WorkerTest() {
}

void WorkerTest::setUp() {
}

void WorkerTest::tearDown() {
}

void WorkerTest::testDispatch() {
  Worker worker("w0", "localhost");
  CPPUNIT_ASSERT(worker.pending().empty());

  Job::ptr_t job(new JobImpl("1", "description"));
  worker.dispatch(job);

  CPPUNIT_ASSERT(! worker.pending().empty());
  CPPUNIT_ASSERT( (*worker.pending().begin())->id() == job->id());
  worker.pending().clear();
}

void WorkerTest::testGetNextJob() {
  Worker worker("w0", "localhost");

  worker.dispatch(Job::ptr_t(new JobImpl("1", "description")));

  CPPUNIT_ASSERT(! worker.pending().empty());

  Job::ptr_t job(worker.get_next_job(Job::invalid_job_id()));
  CPPUNIT_ASSERT(worker.pending().empty()); // pending is empty now
  CPPUNIT_ASSERT(! worker.submitted().empty()); // submitted has one job
  CPPUNIT_ASSERT_EQUAL(sdpa::job_id_t("1"), job->id());
}

void WorkerTest::testAcknowledge() {
  Worker worker("w0", "localhost");

  worker.dispatch(Job::ptr_t(new JobImpl("1", "description")));

  CPPUNIT_ASSERT(! worker.pending().empty());

  Job::ptr_t job(worker.get_next_job(Job::invalid_job_id()));
  CPPUNIT_ASSERT(worker.pending().empty()); // pending is empty
  CPPUNIT_ASSERT(! worker.submitted().empty()); // submitted is not empty anymore

  bool ackResult = worker.acknowledge(job->id());
  CPPUNIT_ASSERT(ackResult);
  CPPUNIT_ASSERT(worker.pending().empty()); // pending still empty
  CPPUNIT_ASSERT(worker.submitted().empty()); // submitted is now empty
  CPPUNIT_ASSERT(! worker.acknowledged().empty()); // added to acknowledged
}


typedef SynchronizedQueue<std::list<int> > queue_type;
struct thread_data {
  thread_data(queue_type *a_q) : q(a_q), val(0) {}
  void operator()() {
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
  
void WorkerTest::testQueue() {
  std::cout << "testing synchronized queue..." << std::endl;
  queue_type test_queue;
  thread_data thrd_data(&test_queue);
  boost::thread thrd(boost::ref(thrd_data));
  std::cout << "pushing..." << std::endl;
  for (std::size_t cnt(0); cnt < 10; ++cnt)
  {
    std::cout << "+";
    test_queue.push(42);
    std::cout.flush();
    if (cnt % 2 == 0) sleep(1);
  }
  thrd.interrupt();
  thrd.join();
  std::cout << std::endl;
  std::cout << thrd_data.val << std::endl;
  CPPUNIT_ASSERT(thrd_data.val >= 42);
}

