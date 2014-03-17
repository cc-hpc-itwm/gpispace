#define BOOST_TEST_MODULE restart_worker_with_coallocation_workflow

#include "kvs_setup_fixture.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (restart_workers_while_job_requiring_coallocation_is_running)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);

  utils::client::client_t client (orchestrator);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (2).to_string()));

  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  fhg::util::thread::event<std::string> job_submitted_0;
  utils::fake_drts_worker_notifying_module_call_submission worker_0
    (boost::bind (&fhg::util::thread::event<std::string>::notify, &job_submitted_0, _1), agent);

  {
    fhg::util::thread::event<> job_submitted_1;

    const utils::fake_drts_worker_notifying_module_call_submission worker_1
      ( worker_id
      , boost::bind (&fhg::util::thread::event<>::notify, &job_submitted_1)
      , agent
      );

    std::string ignore;
    job_submitted_0.wait (ignore);
    job_submitted_1.wait();

    //! \todo At this point, worker_0 should get a cancelJob event for `ignore`!
  }

  const utils::fake_drts_worker_directly_finishing_jobs restarted_worker
    (worker_id, agent);
  std::string job_id_0;
  job_submitted_0.wait (job_id_0);
  worker_0.finish (job_id_0);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_AUTO_TEST_CASE (restart_workers_while_job_is_running_and_partial_result_is_missing)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);

  utils::client::client_t client (orchestrator);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (2).to_string()));

  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  fhg::util::thread::event<std::string> job_submitted_0;
  utils::fake_drts_worker_notifying_module_call_submission worker_0
    (boost::bind (&fhg::util::thread::event<std::string>::notify, &job_submitted_0, _1), agent);

  {
    fhg::util::thread::event<> job_submitted_1;

    const utils::fake_drts_worker_notifying_module_call_submission worker_1
      ( worker_id
      , boost::bind (&fhg::util::thread::event<>::notify, &job_submitted_1)
      , agent
      );

    std::string name_of_job_on_surviving_worker;
    job_submitted_0.wait (name_of_job_on_surviving_worker);
    worker_0.finish (name_of_job_on_surviving_worker);

    //! \todo Don't sleep but somehow check that agent received the
    //! event, allowing us to continue. If we do not wait here,
    //! worker_1 may be killed before the agent handles the finish
    //! event, thus deleting the job and the finish failing. If it
    //! fails, the worker stays marked as reserved on the agent, thus
    //! can't get a new job. Thus, the test case stalls.
    sleep (1);

    job_submitted_1.wait();
  }

  const utils::fake_drts_worker_directly_finishing_jobs restarted_worker
    (worker_id, agent);
  std::string job_id_0;
  job_submitted_0.wait (job_id_0);
  worker_0.finish (job_id_0);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
