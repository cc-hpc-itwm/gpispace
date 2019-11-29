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

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent, certificates);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator, certificates)
                      , sdpa::status::FINISHED
                      );
}

BOOST_DATA_TEST_CASE (chained_agents, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);

  //! \note "variable agents #" was hardcoded to 1 when this test got
  //! rewritten. Probably should be more, so got bumped to 2.
  const utils::agent agent_0 (orchestrator, certificates);
  const utils::agent agent_1 (agent_0, certificates);

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent_1, certificates);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator, certificates)
                      , sdpa::status::FINISHED
                      );
}

BOOST_DATA_TEST_CASE
  (two_workers_with_seperate_master_agent, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);

  const utils::agent agent_0 (orchestrator, certificates);
  const utils::agent agent_1 (agent_0, certificates);
  const utils::agent agent_2 (agent_0, certificates);

  const utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent_1, certificates);
  const utils::fake_drts_worker_directly_finishing_jobs worker_1 (agent_2, certificates);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator, certificates)
                      , sdpa::status::FINISHED
                      );
}

BOOST_DATA_TEST_CASE
  (agent_with_multiple_master_agents, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);

  const utils::agent agent_0 (orchestrator, certificates);
  const utils::agent agent_1 (orchestrator, certificates);
  const utils::agent agent_2 (agent_0, agent_1, certificates);

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent_2, certificates);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator, certificates)
                      , sdpa::status::FINISHED
                      );
}
