#define BOOST_TEST_MODULE testTransformFile

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (testTransformFile1)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
                      , sdpa::status::FINISHED
                      );

	// tr [a-z] [A-Z] < in.txt > out.txt.expected
}
