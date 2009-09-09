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
  CPPUNIT_ASSERT(! worker.pending().empty()); // still in pending
  CPPUNIT_ASSERT_EQUAL(sdpa::job_id_t("1"), job->id());
}

void WorkerTest::testAcknowledge() {
  Worker worker("w0", "localhost");

  worker.dispatch(Job::ptr_t(new JobImpl("1", "description")));

  CPPUNIT_ASSERT(! worker.pending().empty());

  Job::ptr_t job(worker.get_next_job(Job::invalid_job_id()));
  CPPUNIT_ASSERT(! worker.pending().empty()); // still in pending
  CPPUNIT_ASSERT_EQUAL(sdpa::job_id_t("1"), job->id());

  bool ackResult = worker.acknowledge(job->id());
  CPPUNIT_ASSERT(ackResult);
  CPPUNIT_ASSERT(worker.pending().empty()); // removed from pending
  CPPUNIT_ASSERT(! worker.submitted().empty()); // removed from pending
}
