#define BOOST_TEST_MODULE TestSubscriber

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
                , const we::type::activity_t& activity )
    {
      sdpa::daemon::GenericDaemon::submit(activity_id, activity);
      workflowEngine()->finished(activity_id, activity);
    }
 };

BOOST_AUTO_TEST_CASE (execute_workflow_with_subscribed_client)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  const Worker worker( "worker_0", agent._.name());

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (workflow, orchestrator)
    , sdpa::status::FINISHED
    );
}

BOOST_AUTO_TEST_CASE (execute_workflow_and_subscribe_with_second_client)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  const Worker worker( "worker_0", agent._.name());

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber_with_two_different_clients
      (workflow, orchestrator)
    , sdpa::status::FINISHED
    );
}
