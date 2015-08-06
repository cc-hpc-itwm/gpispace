#define BOOST_TEST_MODULE encode_and_decode_events

#include <sdpa/events/Codec.hpp>
#include <boost/test/unit_test.hpp>
#include <we/test/operator_equal.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/printer/set.hpp>
#include <util-generic/testing/random_integral.hpp>
#include <util-generic/testing/random_string.hpp>
#include <util-generic/testing/require_exception.hpp>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (sdpa::Capability, os, capability)
{
  os << "Capability ["
     << " name = " << capability.name()
     << ", owner = " << capability.owner()
     << ", depth = " << capability.depth()
     << "]";
}

namespace
{
  using namespace sdpa::events;

  template<typename T> T* encode_decode_sdpa_event (T e)
  {
    static Codec codec;

    SDPAEvent* d (codec.decode (codec.encode (&e)));
    T* r (dynamic_cast<T*> (d));

    BOOST_REQUIRE (r);

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
  CancelJobAckEvent e ("job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (CancelJob)
{
  CancelJobEvent e ("job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (CapabilitiesGained)
{
  sdpa::capabilities_set_t set;
  set.insert (sdpa::Capability ("foo", fhg::util::testing::random_string()));
  set.insert (sdpa::Capability ("bar", fhg::util::testing::random_string()));

  CapabilitiesGainedEvent e (set);
  CapabilitiesGainedEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->capabilities(), e.capabilities());
}

BOOST_AUTO_TEST_CASE (CapabilitiesLost)
{
  sdpa::capabilities_set_t set;
  set.insert (sdpa::Capability ("foo", fhg::util::testing::random_string()));
  set.insert (sdpa::Capability ("bar", fhg::util::testing::random_string()));

  CapabilitiesLostEvent e (set);
  CapabilitiesLostEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->capabilities(), e.capabilities());
}

BOOST_AUTO_TEST_CASE (DeleteJobAck)
{
  DeleteJobAckEvent e ("job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (DeleteJob)
{
  DeleteJobEvent e ("job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (Error)
{
  ErrorEvent e (ErrorEvent::SDPA_EUNKNOWN, "testing", sdpa::job_id_t ("job-id"));
  ErrorEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->reason(), e.reason());
  BOOST_REQUIRE_EQUAL (r->error_code(), e.error_code());
  BOOST_REQUIRE_EQUAL (r->job_id(), e.job_id());
}

BOOST_AUTO_TEST_CASE (JobFailedAck)
{
  JobFailedAckEvent e ("job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (JobFailed)
{
  JobFailedEvent e ("job-id-1", "testing");
  JobFailedEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->error_message(), e.error_message());
}

BOOST_AUTO_TEST_CASE (JobFinishedAck)
{
  JobFinishedAckEvent e ("job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (JobFinished)
{
  JobFinishedEvent e ("job-id-1", "result");
  JobFinishedEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->result(), e.result());
}

BOOST_AUTO_TEST_CASE (JobResultsReply)
{
  JobResultsReplyEvent e ("job-id-1", "result");
  JobResultsReplyEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->result(), e.result());
}

BOOST_AUTO_TEST_CASE (JobStatusReply)
{
  JobStatusReplyEvent e ("job-id-1", sdpa::status::RUNNING, "testing");
  JobStatusReplyEvent* r (encode_decode_job_event (e));

  BOOST_REQUIRE_EQUAL (r->status(), e.status());
  BOOST_REQUIRE_EQUAL (r->error_message(), e.error_message());
}

BOOST_AUTO_TEST_CASE (QueryJobStatus)
{
  QueryJobStatusEvent e ("job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (RetrieveJobResults)
{
  RetrieveJobResultsEvent e ("job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (SubmitJobAck)
{
  SubmitJobAckEvent e ("job-id-1");
  encode_decode_job_event (e);
}

BOOST_AUTO_TEST_CASE (SubmitJob)
{
  std::set<sdpa::worker_id_t> const workers {"foo", "bar"};

  SubmitJobEvent e (sdpa::job_id_t("job-id-1"), "pnet", workers);
  SubmitJobEvent* r (encode_decode_sdpa_event (e));

  BOOST_REQUIRE_EQUAL (r->job_id(), e.job_id());
  BOOST_REQUIRE_EQUAL (r->description(), e.description());
  BOOST_REQUIRE_EQUAL (r->workers(), e.workers());
}

BOOST_AUTO_TEST_CASE (SubscribeAck)
{
  SubscribeAckEvent e (fhg::util::testing::random_string());
  SubscribeAckEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->job_id(), e.job_id());
}

BOOST_AUTO_TEST_CASE (Subscribe)
{
  SubscribeEvent e (fhg::util::testing::random_string());
  SubscribeEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->job_id(), e.job_id());
}

BOOST_AUTO_TEST_CASE (worker_registration_response_)
{
  {
    worker_registration_response e (boost::none);
    worker_registration_response* r (encode_decode_mgmt_event (e));
    r->get();
  }

  {
    std::string const error (fhg::util::testing::random_string_without_zero());
    worker_registration_response e
      (std::make_exception_ptr (std::runtime_error (error)));
    worker_registration_response* r (encode_decode_mgmt_event (e));
    fhg::util::testing::require_exception
      ([r] { r->get(); }, std::runtime_error (error));
  }
}

BOOST_AUTO_TEST_CASE (WorkerRegistration)
{
  sdpa::capabilities_set_t caps;
  caps.insert (sdpa::Capability ("foo", fhg::util::testing::random_string()));
  caps.insert (sdpa::Capability ("bar", fhg::util::testing::random_string()));

  WorkerRegistrationEvent e
    ( fhg::util::testing::random_string(), caps
    , fhg::util::testing::random_integral<unsigned long>()
    , true, fhg::util::testing::random_string()
    );
  WorkerRegistrationEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->name(), e.name());
  BOOST_REQUIRE_EQUAL (r->capabilities(), e.capabilities());
  BOOST_REQUIRE_EQUAL (r->allocated_shared_memory_size(), e.allocated_shared_memory_size());
  BOOST_REQUIRE_EQUAL (r->children_allowed(), e.children_allowed());
  BOOST_REQUIRE_EQUAL (r->hostname(), e.hostname());
}

BOOST_AUTO_TEST_CASE (DiscoverJobStates)
{
  DiscoverJobStatesEvent e("job_0", "disc_0");
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

  DiscoverJobStatesReplyEvent e("disc_0", disc_res);
  DiscoverJobStatesReplyEvent* r (encode_decode_mgmt_event (e));

  BOOST_REQUIRE_EQUAL (r->discover_id(), e.discover_id());
  BOOST_REQUIRE(r->discover_result() == e.discover_result());
}
