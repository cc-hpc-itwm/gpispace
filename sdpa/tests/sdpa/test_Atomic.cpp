/*
 * =====================================================================================
 *
 *       Filename:  test_Atomic.cpp
 *
 *    Description:  test all components, each with a real gwes, using a real user client
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#define BOOST_TEST_MODULE testAtomic

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include "tests_config.hpp"
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>
#include "kvs_setup_fixture.hpp"

#include <utils.hpp>

namespace po = boost::program_options;

using namespace std;

BOOST_GLOBAL_FIXTURE (KVSSetup);

BOOST_AUTO_TEST_CASE( testAtomicExecution )
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/atomic.pnet"));

  const std::string atomic_file ("atomic_test.txt");

  const int nInitial (0);
  {
    std::ofstream ofs (atomic_file.c_str ());
    ofs << nInitial << std::endl;
  }

  const sdpa::daemon::Orchestrator::ptr_t ptrOrch
    ( sdpa::daemon::Orchestrator::create_with_start_called
      ("orchestrator_0", "127.0.0.1")
    );

	const sdpa::daemon::Agent::ptr_t ptrAgent
    ( sdpa::daemon::AgentFactory<we::mgmt::layer>::create_with_start_called
      ( "agent_0"
      , "127.0.0.1"
      , sdpa::master_info_list_t (1, sdpa::MasterInfo ("orchestrator_0"))
      )
    );

  {
    const utils::drts_worker worker_0
      ( "drts_0", "agent_0"
      , "ATOMIC"
      , TESTS_EXAMPLE_ATOMIC_MODULES_PATH
      , kvs_host(), kvs_port()
      );
    const utils::drts_worker worker_1
      ("drts_1", "agent_0"
      , "A,B"
      , TESTS_EXAMPLE_ATOMIC_MODULES_PATH
      , kvs_host(), kvs_port()
      );

    utils::client::submit_job_and_wait_for_termination
      (workflow, "sdpac", "orchestrator_0");
  }

	ptrAgent->shutdown();
	ptrOrch->shutdown();

	int nCounterVal (0);
	{
		std::ifstream ifs (atomic_file.c_str());
		BOOST_REQUIRE (ifs.good());
		ifs >> nCounterVal;
	}

	BOOST_REQUIRE_EQUAL( nCounterVal - nInitial
                     , 2 * boost::lexical_cast<int> (TESTS_N_ATOMIC_TASKS)
                     );
}
