#define BOOST_TEST_MODULE testAtomic

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (testAtomicExecution)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/atomic.pnet"));

  const std::string atomic_file ("atomic_test.txt");

  const int nInitial (0);
  {
    std::ofstream ofs (atomic_file.c_str ());
    ofs << nInitial << std::endl;
  }

  {
    const utils::orchestrator orchestrator ("orchestrator_0", "127.0.0.1");

    const utils::agent<we::mgmt::layer> agent
      ("agent_0", "127.0.0.1", orchestrator);

    const utils::drts_worker worker_0
      ( "drts_0", agent
      , "ATOMIC"
      , TESTS_EXAMPLE_ATOMIC_MODULES_PATH
      , kvs_host(), kvs_port()
      );
    const utils::drts_worker worker_1
      ( "drts_1", agent
      , "A,B"
      , TESTS_EXAMPLE_ATOMIC_MODULES_PATH
      , kvs_host(), kvs_port()
      );

    utils::client::submit_job_and_wait_for_termination
      (workflow, "sdpac", orchestrator.name());
  }

	int nCounterVal (0);
	{
		std::ifstream ifs (atomic_file.c_str());
		BOOST_REQUIRE (ifs.good());
		ifs >> nCounterVal;
	}

	BOOST_REQUIRE_EQUAL ( nCounterVal - nInitial
                      , 2 * boost::lexical_cast<int> (TESTS_N_ATOMIC_TASKS)
                      );
}
