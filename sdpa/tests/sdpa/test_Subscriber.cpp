#define BOOST_TEST_MODULE TestSubscriber

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public utils::BasicAgent
{
  public:
    Worker (const std::string& name, const utils::agent& master_agent, const std::string cpb_name = "")
      :  utils::BasicAgent (name, master_agent, cpb_name)
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

BOOST_AUTO_TEST_CASE (execute_workflow_with_subscribed_client)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);
  const utils::fake_drts_worker_directly_finishing_jobs worker (agent);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (utils::module_call(), orchestrator)
    , sdpa::status::FINISHED
    );
}

BOOST_AUTO_TEST_CASE (execute_workflow_and_subscribe_with_second_client)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);
  const utils::fake_drts_worker_directly_finishing_jobs worker (agent);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber_with_two_different_clients
      (utils::module_call(), orchestrator)
    , sdpa::status::FINISHED
    );
}
