#define BOOST_TEST_MODULE testCoallocation

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public utils::BasicWorker
{
  public:
    Worker (const std::string& name, const std::string& master_name, const std::string cpb_name)
      :  utils::BasicWorker (name, master_name, cpb_name)
    {}

    void handleSubmitJobEvent (const sdpa::events::SubmitJobEvent* pEvt)
    {
      sdpa::events::SubmitJobAckEvent::Ptr
      pSubmitJobAckEvt(new sdpa::events::SubmitJobAckEvent( _name
                                                          , pEvt->from()
                                                          , *pEvt->job_id()));
      _network_strategy->perform (pSubmitJobAckEvt);

      sdpa::events::JobFinishedEvent::Ptr
      pJobFinishedEvt(new sdpa::events::JobFinishedEvent( _name
                                                        , pEvt->from()
                                                        , *pEvt->job_id()
                                                        , pEvt->description() ));

      _network_strategy->perform (pJobFinishedEvt);
    }

    void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent* ){}
};

BOOST_AUTO_TEST_CASE (testCoallocationWorkflow)
{
  const std::string workflow
    (utils::require_and_read_file ("coallocation_test2.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  const Worker worker_0( "worker_0", agent._.name(), "A");
  const Worker worker_1( "worker_1", agent._.name(), "A");
  const Worker worker_2( "worker_2", agent._.name(), "B");
  const Worker worker_3( "worker_3", agent._.name(), "B");
  const Worker worker_4( "worker_4", agent._.name(), "B");

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (workflow, orchestrator)
    , sdpa::status::FINISHED
    );
}
