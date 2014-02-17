#define BOOST_TEST_MODULE TestTopologies

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public sdpa::daemon::Agent
 {
  public:
    Worker (const std::string& name, const std::string& master_name)
      : Agent (name, "127.0.0.1", kvs_host(), kvs_port(), sdpa::master_info_list_t(1, sdpa::MasterInfo(master_name)), boost::none)
    {}

    Worker (const std::string& name,const utils::agents_t& masters)
          : Agent (name, "127.0.0.1", kvs_host(), kvs_port(), utils::assemble_master_info_list (masters), boost::none)
        {}

    void submit ( const we::layer::id_type& activity_id
                , const we::type::activity_t& activity
                )
    {
      sdpa::daemon::GenericDaemon::submit(activity_id, activity);
      workflowEngine()->finished(activity_id, activity);
    }
 };

BOOST_AUTO_TEST_CASE (orchestrator_agent_worker)
{
  // O
  // |
  // A
  // |
  // W

  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  const Worker worker( "worker_0", "agent_0");

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
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

  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  //! \note "variable agents #" was hardcoded to 1 when this test got
  //! rewritten. Probably should be more, so got bumped to 2.
  const utils::agent agent_0
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);
  const utils::agent agent_1
    ("agent_1", "127.0.0.1", kvs_host(), kvs_port(), agent_0);

  const Worker worker( "worker_0", "agent_1");

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
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

  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent_0
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);
  const utils::agent agent_1
    ("agent_1", "127.0.0.1", kvs_host(), kvs_port(), agent_0);
  const utils::agent agent_2
    ("agent_2", "127.0.0.1", kvs_host(), kvs_port(), agent_0);

  const Worker worker_0( "worker_0", "agent_1");
  const Worker worker_1( "worker_0", "agent_2");

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_AUTO_TEST_CASE (one_worker_with_multiple_master_agents)
{
  // O-+-+
  // | | |
  // A ? A -> variable agents #
  // | | |
  // W-+-+

  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  //! \note "variable agents #" was hardcoded to 1 when this test got
  //! rewritten. Probably should be more, so got bumped to 2.
  const utils::agent agent_0
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);
  const utils::agent agent_1
    ("agent_1", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  utils::agents_t agents;
  agents.push_back (boost::cref (agent_0));
  agents.push_back (boost::cref (agent_1));

  const Worker worker ("worker_0", agents);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
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

  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  const utils::agent agent_0

    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);
  const utils::agent agent_1
    ("agent_1", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  utils::agents_t agents;
  agents.push_back (boost::cref (agent_0));
  agents.push_back (boost::cref (agent_1));

  const utils::agent agent_2
    ("agent_2", "127.0.0.1", kvs_host(), kvs_port(), agents);

  const Worker worker("worker_0", "agent_2");

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
                      , sdpa::status::FINISHED
                      );
}
