#define BOOST_TEST_MODULE encode_and_decode_events

#include <sdpa/events/Codec.hpp>
#include <boost/test/unit_test.hpp>
#include <we/test/operator_equal.hpp>

#include <fhg/util/random_string.hpp>

BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::capabilities_set_t);
BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::worker_id_list_t);

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
  CancelJobAckEvent e ("foo", "bar", "job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (CancelJob)
{
  CancelJobEvent e ("foo", "bar", "job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (CapabilitiesGained)
{
  sdpa::capabilities_set_t set;
  set.insert (sdpa::Capability ("foo", fhg::util::random_string()));
  set.insert (sdpa::Capability ("bar", fhg::util::random_string()));

  CapabilitiesGainedEvent e ("from", "to", set);
  CapabilitiesGainedEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->capabilities(), e.capabilities());
}

BOOST_AUTO_TEST_CASE (CapabilitiesLost)
{
  sdpa::capabilities_set_t set;
  set.insert (sdpa::Capability ("foo", fhg::util::random_string()));
  set.insert (sdpa::Capability ("bar", fhg::util::random_string()));

  CapabilitiesLostEvent e ("from", "to", set);
  CapabilitiesLostEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->capabilities(), e.capabilities());
}

BOOST_AUTO_TEST_CASE (DeleteJobAck)
{
  DeleteJobAckEvent e ("foo", "bar", "job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (DeleteJob)
{
  DeleteJobEvent e ("foo", "bar", "job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (Error)
{
  ErrorEvent e ("from", "to", ErrorEvent::SDPA_EUNKNOWN, "testing", sdpa::job_id_t ("job-id"));
  ErrorEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->reason(), e.reason());
  BOOST_REQUIRE_EQUAL (r->error_code(), e.error_code());
  BOOST_REQUIRE_EQUAL (r->job_id(), e.job_id());
}

BOOST_AUTO_TEST_CASE (JobFailedAck)
{
  JobFailedAckEvent e ("foo", "bar", "job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (JobFailed)
{
  JobFailedEvent e ("foo", "bar", "job-id-1", rand(), "testing");
  JobFailedEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->error_code(), e.error_code());
  BOOST_REQUIRE_EQUAL (r->error_message(), e.error_message());
}

BOOST_AUTO_TEST_CASE (JobFinishedAck)
{
  JobFinishedAckEvent e ("foo", "bar", "job-id-1");
  encode_decode_job_event (e);
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

BOOST_AUTO_TEST_CASE (JobStatusReply)
{
  JobStatusReplyEvent e ("foo", "bar", "job-id-1", sdpa::status::RUNNING, rand(), "testing");
  JobStatusReplyEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->status(), e.status());
  BOOST_REQUIRE_EQUAL (r->error_code(), e.error_code());
  BOOST_REQUIRE_EQUAL (r->error_message(), e.error_message());
}

BOOST_AUTO_TEST_CASE (QueryJobStatus)
{
  QueryJobStatusEvent e ("foo", "bar", "job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (RetrieveJobResults)
{
  RetrieveJobResultsEvent e ("foo", "bar", "job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (SubmitJobAck)
{
  SubmitJobAckEvent e ("foo", "bar", "job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (SubmitJob)
{
  sdpa::worker_id_list_t workers;
  workers.push_back ("foo");
  workers.push_back ("bar");

  SubmitJobEvent e ("foo", "bar", sdpa::job_id_t("job-id-1"), "pnet", workers);
  SubmitJobEvent* r (encode_decode_sdpa_event (e));

  BOOST_REQUIRE_EQUAL (r->job_id(), e.job_id());
  BOOST_REQUIRE_EQUAL (r->description(), e.description());
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
  encode_decode_mgmt_event (e);
}

BOOST_AUTO_TEST_CASE (WorkerRegistration)
{
  sdpa::capabilities_set_t caps;
  caps.insert (sdpa::Capability ("foo", fhg::util::random_string()));
  caps.insert (sdpa::Capability ("bar", fhg::util::random_string()));

  WorkerRegistrationEvent e ("foo", "bar", 10, caps, 50, "fooagent");
  WorkerRegistrationEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->capacity(), e.capacity());
  BOOST_REQUIRE_EQUAL (r->rank(), e.rank());
  BOOST_REQUIRE_EQUAL (r->capabilities(), e.capabilities());
  BOOST_REQUIRE_EQUAL (r->agent_uuid(), e.agent_uuid());
}

BOOST_AUTO_TEST_CASE (DiscoverJobStates)
{
  DiscoverJobStatesEvent e("foo", "bar", "job_0", "disc_0");
  DiscoverJobStatesEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->discover_id(), e.discover_id());
}

BOOST_AUTO_TEST_CASE (DiscoverJobStatesReply)
{
  sdpa::discovery_info_t disc_info_child_1("job_0_1", sdpa::status::PENDING, sdpa::discovery_info_set_t());
  sdpa::discovery_info_t disc_info_child_2("job_0_2", sdpa::status::FINISHED, sdpa::discovery_info_set_t());
  sdpa::discovery_info_t disc_info_child_3("job_0_3", sdpa::status::FAILED, sdpa::discovery_info_set_t());

  sdpa::discovery_info_set_t disc_info_set;
  disc_info_set.insert(disc_info_child_1);
  disc_info_set.insert(disc_info_child_2);
  disc_info_set.insert(disc_info_child_3);

  sdpa::discovery_info_t disc_res("job_0", boost::none, disc_info_set);

  DiscoverJobStatesReplyEvent e("foo", "bar", "disc_0", disc_res);
  DiscoverJobStatesReplyEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->discover_id(), e.discover_id());
  BOOST_REQUIRE(r->discover_result() == e.discover_result());
}
