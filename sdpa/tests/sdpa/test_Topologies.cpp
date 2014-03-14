#define BOOST_TEST_MODULE TestTopologies

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public utils::BasicAgent
{
  public:
    Worker (const utils::agent& master_agent, const std::string cpb_name = "")
      :  utils::BasicAgent (utils::random_peer_name(), master_agent, cpb_name)
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


BOOST_AUTO_TEST_CASE (orchestrator_agent_worker)
{
  // O
  // |
  // A
  // |
  // W

  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (kvs_host(), kvs_port(), orchestrator);

  const Worker worker (agent);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_AUTO_TEST_CASE (chained_agents)
{
	// O
	// |
	// A
  // |
  // ? -> variable agents #
	// |
	// A
	// |
	// W

  const utils::orchestrator orchestrator (kvs_host(), kvs_port());

  //! \note "variable agents #" was hardcoded to 1 when this test got
  //! rewritten. Probably should be more, so got bumped to 2.
  const utils::agent agent_0 (kvs_host(), kvs_port(), orchestrator);
  const utils::agent agent_1 (kvs_host(), kvs_port(), agent_0);

  const Worker worker (agent_1);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_AUTO_TEST_CASE (two_workers_with_seperate_master_agent)
{
  // O
  // |
  // A-+
  // | |
  // A A
  // | |
  // W W

  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent_0 (kvs_host(), kvs_port(), orchestrator);
  const utils::agent agent_1 (kvs_host(), kvs_port(), agent_0);
  const utils::agent agent_2 (kvs_host(), kvs_port(), agent_0);

  const Worker worker_0 (agent_1);
  const Worker worker_1 (agent_2);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_AUTO_TEST_CASE (agent_with_multiple_master_agents)
{
  // O-+
  // | |
  // A A
  // | |
  // A-+
  // |
  // W

  const utils::orchestrator orchestrator (kvs_host(), kvs_port());

  const utils::agent agent_0 (kvs_host(), kvs_port(), orchestrator);
  const utils::agent agent_1 (kvs_host(), kvs_port(), orchestrator);

  utils::agents_t agents;
  agents.push_back (boost::cref (agent_0));
  agents.push_back (boost::cref (agent_1));

  const utils::agent agent_2 (kvs_host(), kvs_port(), agents);

  const Worker worker (agent_2);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}
