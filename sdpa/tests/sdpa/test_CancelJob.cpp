#define BOOST_TEST_MODULE TestCancelJob

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>
#include <cstdlib>
#include <ctime>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public sdpa::daemon::Agent
 {
  public:
    Worker (const std::string& name, const std::string& master_name)
    : Agent ( name
            , "127.0.0.1"
            , kvs_host()
            , kvs_port()
            , sdpa::master_info_list_t(1, sdpa::MasterInfo(master_name))
            , boost::none)
    {}

    void submit ( const we::layer::id_type& activity_id
                , const we::type::activity_t& activity
                )
    {
      sdpa::daemon::GenericDaemon::submit(activity_id, activity);
      _cond_got_job.notify_one();
    }

    void wait_for_jobs()
    {
      boost::unique_lock<boost::mutex> lock(_mtx_got_job);
      _cond_got_job.wait(lock);
    }

  private:

    boost::mutex _mtx_got_job;
    boost::condition_variable_any _cond_got_job;
 };

BOOST_AUTO_TEST_CASE (cancel_no_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  sdpa::job_id_t job_id(client.submitJob (workflow));
  client.cancelJob(job_id);

  BOOST_REQUIRE_EQUAL
  ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED );
}

BOOST_AUTO_TEST_CASE (cancel_with_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  Worker worker("worker_0", agent._.name());

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  sdpa::job_id_t job_id(client.submitJob (workflow));

  worker.wait_for_jobs();

  client.cancelJob(job_id);

  BOOST_REQUIRE_EQUAL
  ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED );
}

BOOST_AUTO_TEST_CASE (call_cancel_twice_orch)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  sdpa::job_id_t job_id(client.submitJob (workflow));
  client.cancelJob(job_id);

  BOOST_REQUIRE_EQUAL
  ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (call_cancel_twice_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_2", "127.0.0.1", kvs_host(), kvs_port());

  utils::agent agent
      ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  Worker worker("worker_0", agent._.name());

  sdpa::client::Client client (orchestrator.name(),  kvs_host(), kvs_port());
  sdpa::job_id_t job_id(client.submitJob (workflow));

  worker.wait_for_jobs();

  client.cancelJob(job_id);

  BOOST_REQUIRE_EQUAL
  ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}
