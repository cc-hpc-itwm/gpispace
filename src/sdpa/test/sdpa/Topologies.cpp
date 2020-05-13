#include <sdpa/test/sdpa/utils.hpp>

#include <test/certificates_data.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

BOOST_DATA_TEST_CASE
  (orchestrator_agent_worker, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);

  const utils::agent agent (orchestrator, certificates);

  utils::client client (orchestrator, certificates);

  fhg::util::thread::event<std::string> job_submitted;
  utils::fake_drts_worker_waiting_for_finished_ack worker
    ( [&job_submitted] (std::string s) { job_submitted.notify (s); }
    , agent
    , certificates
    );

  auto const job_id (client.submit_job (utils::module_call()));

  worker.finish_and_wait_for_ack (job_submitted.wait());

  BOOST_REQUIRE_EQUAL
     (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_DATA_TEST_CASE (chained_agents, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);

  //! \note "variable agents #" was hardcoded to 1 when this test got
  //! rewritten. Probably should be more, so got bumped to 2.
  const utils::agent agent_0 (orchestrator, certificates);
  const utils::agent agent_1 (agent_0, certificates);

  utils::client client (orchestrator, certificates);

  fhg::util::thread::event<std::string> job_submitted;
  utils::fake_drts_worker_waiting_for_finished_ack worker
    ( [&job_submitted] (std::string s) { job_submitted.notify (s); }
    , agent_1
    , certificates
    );

  auto const job_id (client.submit_job (utils::module_call()));

  worker.finish_and_wait_for_ack (job_submitted.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_DATA_TEST_CASE
  (two_workers_with_seperate_master_agent, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);

  const utils::agent agent_0 (orchestrator, certificates);
  const utils::agent agent_1 (agent_0, certificates);
  const utils::agent agent_2 (agent_0, certificates);

  utils::client client (orchestrator, certificates);

  fhg::util::thread::event<std::string> job_submitted_0;
  utils::fake_drts_worker_waiting_for_finished_ack worker_0
    ( [&job_submitted_0] (std::string s) { job_submitted_0.notify (s); }
    , agent_1
    , certificates
    );

  fhg::util::thread::event<std::string> job_submitted_1;
  utils::fake_drts_worker_waiting_for_finished_ack worker_1
    ( [&job_submitted_1] (std::string s) { job_submitted_1.notify (s); }
    , agent_2
    , certificates
    );

  auto const job_id_0 (client.submit_job (utils::module_call()));
  auto const job_id_1 (client.submit_job (utils::module_call()));

  worker_0.finish_and_wait_for_ack (job_submitted_0.wait());
  worker_1.finish_and_wait_for_ack (job_submitted_1.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id_0), sdpa::status::FINISHED);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id_1), sdpa::status::FINISHED);
}

BOOST_DATA_TEST_CASE
  (agent_with_multiple_master_agents, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);

  const utils::agent agent_0 (orchestrator, certificates);
  const utils::agent agent_1 (orchestrator, certificates);
  const utils::agent agent_2 (agent_0, agent_1, certificates);

  utils::client client (orchestrator, certificates);

  fhg::util::thread::event<std::string> job_submitted;
  utils::fake_drts_worker_waiting_for_finished_ack worker
    ( [&job_submitted] (std::string s) { job_submitted.notify (s); }
    , agent_2
    , certificates
    );

  auto const job_id (client.submit_job (utils::module_call()));

  worker.finish_and_wait_for_ack (job_submitted.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
