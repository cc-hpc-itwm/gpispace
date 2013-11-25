#define BOOST_TEST_MODULE encode_and_decode_events

#include <sdpa/events/Codec.hpp>
#include <sdpa/events/JobRunningEvent.hpp>
#include <sdpa/events/JobStalledEvent.hpp>

#include <boost/test/unit_test.hpp>

BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::capabilities_set_t);
BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::job_id_t);
BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::worker_id_list_t);
BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::job_id_list_t);

namespace
{
  using namespace sdpa::events;

  template<typename T> T* encode_decode_sdpa_event (T e)
  {
    static Codec codec;

    SDPAEvent* d (codec.decode (codec.encode (&e)));
    T* r (dynamic_cast<T*> (d));

    BOOST_REQUIRE (r);

    BOOST_REQUIRE_EQUAL (r->str(), e.str());

    BOOST_REQUIRE_EQUAL (r->from(), e.from());
    BOOST_REQUIRE_EQUAL (r->to(), e.to());

    BOOST_REQUIRE_EQUAL (r->priority(), e.priority());

    return r;
  }

  template<typename T> T* encode_decode_job_event (T e)
  {
    T* r (encode_decode_sdpa_event (e));

    BOOST_REQUIRE_EQUAL (r->job_id(), e.job_id());

    return r;
  }

  template<typename T> T* encode_decode_mgmt_event (T e)
  {
    T* r (encode_decode_sdpa_event (e));
    return r;
  }
}

BOOST_AUTO_TEST_CASE (CancelJobAck)
{
  CancelJobAckEvent e ("foo", "bar", "job-id-1", "result");
  CancelJobAckEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->result(), e.result());
}

BOOST_AUTO_TEST_CASE (CancelJob)
{
  CancelJobEvent e ("foo", "bar", "job-id-1", "reason");
  CancelJobEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->reason(), e.reason());
}

BOOST_AUTO_TEST_CASE (CapabilitiesGained)
{
  sdpa::capabilities_set_t set;
  set.insert (sdpa::Capability ("foo"));
  set.insert (sdpa::Capability ("bar"));

  CapabilitiesGainedEvent e ("from", "to", set);
  CapabilitiesGainedEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->capabilities(), e.capabilities());
}

BOOST_AUTO_TEST_CASE (CapabilitiesLost)
{
  sdpa::capabilities_set_t set;
  set.insert (sdpa::Capability ("foo"));
  set.insert (sdpa::Capability ("bar"));

  CapabilitiesLostEvent e ("from", "to", set);
  CapabilitiesLostEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->capabilities(), e.capabilities());
}

BOOST_AUTO_TEST_CASE (DeleteJobAck)
{
  DeleteJobAckEvent e ("foo", "bar", "job-id-1");
  DeleteJobAckEvent* r (encode_decode_job_event (e));
}

BOOST_AUTO_TEST_CASE (DeleteJob)
{
  DeleteJobEvent e ("foo", "bar", "job-id-1");
  DeleteJobEvent* r (encode_decode_job_event (e));
}

BOOST_AUTO_TEST_CASE (Error)
{
  ErrorEvent e ("from", "to", ErrorEvent::SDPA_EUNKNOWN, "testing", "job-id");
  ErrorEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->reason(), e.reason());
  BOOST_REQUIRE_EQUAL (r->error_code(), e.error_code());
  BOOST_REQUIRE_EQUAL (r->job_id(), e.job_id());
}

BOOST_AUTO_TEST_CASE (JobFailedAck)
{
  JobFailedAckEvent e ("foo", "bar", "job-id-1");
  JobFailedAckEvent* r (encode_decode_job_event (e));
}

BOOST_AUTO_TEST_CASE (JobFailed)
{
  JobFailedEvent e ("foo", "bar", "job-id-1", "no result", fhg::error::UNASSIGNED_ERROR, "testing");
  JobFailedEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->result(), e.result());
  BOOST_REQUIRE_EQUAL (r->error_code(), e.error_code());
  BOOST_REQUIRE_EQUAL (r->error_message(), e.error_message());
}

BOOST_AUTO_TEST_CASE (JobFinishedAck)
{
  JobFinishedAckEvent e ("foo", "bar", "job-id-1");
  JobFinishedAckEvent* r (encode_decode_job_event (e));
}

BOOST_AUTO_TEST_CASE (JobFinished)
{
  JobFinishedEvent e ("foo", "bar", "job-id-1", "result");
  JobFinishedEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->result(), e.result());
}

BOOST_AUTO_TEST_CASE (JobResultsReply)
{
  JobResultsReplyEvent e ("foo", "bar", "job-id-1", "result");
  JobResultsReplyEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->result(), e.result());
}

BOOST_AUTO_TEST_CASE (JobRunning)
{
  JobRunningEvent e ("foo", "bar", "job-id-1");
  JobRunningEvent* r (encode_decode_job_event (e));
}

BOOST_AUTO_TEST_CASE (JobStalled)
{
  JobStalledEvent e ("foo", "bar", "job-id-1");
  JobStalledEvent* r (encode_decode_job_event (e));
}

BOOST_AUTO_TEST_CASE (JobStatusReply)
{
  JobStatusReplyEvent e ("foo", "bar", "job-id-1", sdpa::status::RUNNING, fhg::error::UNASSIGNED_ERROR, "testing");
  JobStatusReplyEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->status(), e.status());
  BOOST_REQUIRE_EQUAL (r->error_code(), e.error_code());
  BOOST_REQUIRE_EQUAL (r->error_message(), e.error_message());
}

BOOST_AUTO_TEST_CASE (QueryJobStatus)
{
  QueryJobStatusEvent e ("foo", "bar", "job-id-1");
  QueryJobStatusEvent* r (encode_decode_job_event (e));
}

BOOST_AUTO_TEST_CASE (RetrieveJobResults)
{
  RetrieveJobResultsEvent e ("foo", "bar", "job-id-1");
  RetrieveJobResultsEvent* r (encode_decode_job_event (e));
}

BOOST_AUTO_TEST_CASE (SubmitJobAck)
{
  SubmitJobAckEvent e ("foo", "bar", "job-id-1");
  SubmitJobAckEvent* r (encode_decode_job_event (e));
}

BOOST_AUTO_TEST_CASE (SubmitJob)
{
  sdpa::worker_id_list_t workers;
  workers.push_back ("foo");
  workers.push_back ("bar");

  SubmitJobEvent e ("foo", "bar", "job-id-1", "pnet", "parent job", workers);
  SubmitJobEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->description(), e.description());
  BOOST_REQUIRE_EQUAL (r->parent_id(), e.parent_id());
  BOOST_REQUIRE_EQUAL (r->worker_list(), e.worker_list());
}

BOOST_AUTO_TEST_CASE (SubscribeAck)
{
  sdpa::job_id_list_t jobs;
  jobs.push_back ("job-1");
  jobs.push_back ("job-2");

  SubscribeAckEvent e ("foo", "bar", jobs);
  SubscribeAckEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->listJobIds(), e.listJobIds());
}

BOOST_AUTO_TEST_CASE (Subscribe)
{
  sdpa::job_id_list_t jobs;
  jobs.push_back ("job-1");
  jobs.push_back ("job-2");

  SubscribeEvent e ("foo", "bar", jobs);
  SubscribeEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->listJobIds(), e.listJobIds());
  BOOST_REQUIRE_EQUAL (r->subscriber(), e.subscriber());
}

BOOST_AUTO_TEST_CASE (WorkerRegistrationAck)
{
  WorkerRegistrationAckEvent e ("foo", "bar");
  WorkerRegistrationAckEvent* r (encode_decode_mgmt_event (e));
}

BOOST_AUTO_TEST_CASE (WorkerRegistration)
{
  sdpa::capabilities_set_t caps;
  caps.insert (sdpa::Capability ("foo"));
  caps.insert (sdpa::Capability ("bar"));

  WorkerRegistrationEvent e ("foo", "bar", 10, caps, 50, "fooagent");
  WorkerRegistrationEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->capacity(), e.capacity());
  BOOST_REQUIRE_EQUAL (r->rank(), e.rank());
  BOOST_REQUIRE_EQUAL (r->capabilities(), e.capabilities());
  BOOST_REQUIRE_EQUAL (r->agent_uuid(), e.agent_uuid());
}
