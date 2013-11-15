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
	LOG( DEBUG, "***** test_Atomic *****"<<std::endl);

  const std::string workflow
    (utils::require_and_read_file ("workflows/atomic.pnet"));


	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

  const std::string atomic_file ("atomic_test.txt");

  int nInitial (0);
  {
    std::ofstream ofs (atomic_file.c_str ());
    ofs << nInitial << std::endl;
  }

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create_with_start_called("orchestrator_0", addrOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, sdpa::MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create_with_start_called("agent_0", addrAgent, arrAgentMasterInfo);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "ATOMIC", TESTS_EXAMPLE_ATOMIC_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread( &fhg::core::kernel_t::run, drts_0 );

	sdpa::shared_ptr<fhg::core::kernel_t> drts_1( createDRTSWorker("drts_1", "agent_0", "A,B", TESTS_EXAMPLE_ATOMIC_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_1_thread = boost::thread( &fhg::core::kernel_t::run, drts_1 );

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");

	boost::thread threadClient = boost::thread(boost::bind(&utils::client::submit_job_and_wait_for_termination, workflow, cav, "sdpac"));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();
	drts_0->unload_all();

	drts_1->stop();
	drts_1_thread.join();
	drts_1->unload_all();

	ptrAgent->shutdown();
	ptrOrch->shutdown();

	int nCounterVal=0;
	{
		std::ifstream ifs (atomic_file.c_str());
		BOOST_CHECK (ifs.good());
		ifs>>nCounterVal;
	}

	LOG(INFO, "Intial value was "<<nInitial);
	LOG(INFO, "The counter value now is: "<<nCounterVal);

	nCounterVal-=nInitial;

	const int nTasks (boost::lexical_cast<int> (TESTS_N_ATOMIC_TASKS));

	BOOST_CHECK((nCounterVal==2*nTasks));

	LOG( DEBUG, "The test case test_Atomic terminated!");
}
