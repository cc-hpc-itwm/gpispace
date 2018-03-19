#include <utils.hpp>

void test_remove_workers_in_a_coallocation_job_and_add_them_again
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  utils::orchestrator const orchestrator (_logger, certificates);
  utils::agent const agent (orchestrator, _logger, certificates);

  utils::client client (orchestrator, certificates);

  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_two_children_requiring_n_workers (2)));

  {
    fhg::util::thread::event<std::string> submitted_1;

    utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent, certificates);
    utils::fake_drts_worker_notifying_module_call_submission worker_1
      ([&] (std::string s) { submitted_1.notify (s); }, agent, certificates);

    submitted_1.wait();
  }

  {
    fhg::util::thread::event<std::string> submitted_1;

    utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent, certificates);
    utils::fake_drts_worker_waiting_for_finished_ack worker_1
      ([&] (std::string s) { submitted_1.notify (s); }, agent, certificates);

    worker_1.finish_and_wait_for_ack (submitted_1.wait());
  }

  {
    fhg::util::thread::event<std::string> submitted_1;

    utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent, certificates);
    utils::fake_drts_worker_waiting_for_finished_ack worker_1
      ([&] (std::string s) { submitted_1.notify (s); }, agent, certificates);

    worker_1.finish_and_wait_for_ack (submitted_1.wait());
  }

  BOOST_REQUIRE_EQUAL ( client.wait_for_terminal_state_and_cleanup (job_id)
                      , sdpa::status::FINISHED
                      );
}

BOOST_FIXTURE_TEST_CASE
  (remove_workers_in_a_coallocation_job_and_add_them_again, setup_logging)
{
  test_remove_workers_in_a_coallocation_job_and_add_them_again (_logger, boost::none);

  if (test_certificates)
  {
    test_remove_workers_in_a_coallocation_job_and_add_them_again (_logger, test_certificates);
  }
}
