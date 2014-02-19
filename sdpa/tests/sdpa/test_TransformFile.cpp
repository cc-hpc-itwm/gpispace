#define BOOST_TEST_MODULE testTransformFile

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

    void submit ( const we::layer::id_type& activity_id
                , const we::type::activity_t& activity
                )
    {
      sdpa::daemon::GenericDaemon::submit(activity_id, activity);
      workflowEngine()->finished(activity_id, activity);
    }
};

BOOST_AUTO_TEST_CASE (transform_file_with_mock_worker)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  const Worker worker_0( "worker_0", agent._.name());

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
                      , sdpa::status::FINISHED
                      );
}
