#include <utils.hpp>

BOOST_FIXTURE_TEST_CASE
  (remove_workers_in_a_coallocation_job_and_add_them_again, setup_logging)
{
  utils::orchestrator const orchestrator (_logger);
  utils::agent const agent (orchestrator, _logger);

  utils::client client (orchestrator);

  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_two_children_requiring_n_workers (2)));

  {
    fhg::util::thread::event<std::string> submitted_1;

    utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent);
    utils::fake_drts_worker_notifying_module_call_submission worker_1
      ([&] (std::string s) { submitted_1.notify (s); }, agent);

    submitted_1.wait();
  }

  {
    fhg::util::thread::event<std::string> submitted_1;

    utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent);
    utils::fake_drts_worker_waiting_for_finished_ack worker_1
      ([&] (std::string s) { submitted_1.notify (s); }, agent);

    worker_1.finish_and_wait_for_ack (submitted_1.wait());
  }

  {
    fhg::util::thread::event<std::string> submitted_1;

    utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent);
    utils::fake_drts_worker_waiting_for_finished_ack worker_1
      ([&] (std::string s) { submitted_1.notify (s); }, agent);

    worker_1.finish_and_wait_for_ack (submitted_1.wait());
  }

  BOOST_REQUIRE_EQUAL ( client.wait_for_terminal_state_and_cleanup (job_id)
                      , sdpa::status::FINISHED
                      );
}
