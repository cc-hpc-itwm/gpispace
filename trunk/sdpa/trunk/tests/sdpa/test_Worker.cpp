#include <iostream>

#include "test_Worker.hpp"
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/JobImpl.hpp>

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
  CPPUNIT_ASSERT_EQUAL((*worker.submitted().begin())->id(), job->id());
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
