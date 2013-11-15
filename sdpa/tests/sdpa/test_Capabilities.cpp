/*
 * =====================================================================================
 *
 *       Filename:  test_Capabilities.cpp
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
#define BOOST_TEST_MODULE testCapabilities

#include <boost/test/unit_test.hpp>
#include "tests_config.hpp"
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include "kvs_setup_fixture.hpp"

#include <utils.hpp>

namespace po = boost::program_options;

using namespace std;

BOOST_GLOBAL_FIXTURE (KVSSetup);

BOOST_AUTO_TEST_CASE( Test1 )
{
	LOG( INFO, "***** Test capabilities *****"<<std::endl);
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

  typedef void OrchWorkflowEngine;

  const std::string workflow
    (utils::require_and_read_file ("workflows/capabilities.pnet"));

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create_with_start_called("orchestrator_0", addrOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, sdpa::MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create_with_start_called("agent_0", addrAgent, arrAgentMasterInfo);

  {
    const utils::drts_worker worker_0 ("drts_0", "agent_1", "A", TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH,  kvs_host(), kvs_port());
    const utils::drts_worker worker_1 ("drts_1", "agent_1", "B", TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH,  kvs_host(), kvs_port());
    const utils::drts_worker worker_2 ("drts_2", "agent_1", "A", TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH,  kvs_host(), kvs_port());

    boost::thread threadClient = boost::thread(boost::bind(&utils::client::submit_job_and_wait_for_termination, workflow, "sdpac", "orchestrator_0"));

    if(threadClient.joinable())
      threadClient.join();
    LOG( INFO, "The client thread joined the main thread!" );
  }

  ptrAgent->shutdown();
  ptrOrch->shutdown();

  LOG( INFO, "The test case Test1 terminated!");
}

BOOST_AUTO_TEST_CASE( testCapabilities_NoMandatoryReq )
{
	LOG( DEBUG, "***** Test capabilities (no mandatory) *****"<<std::endl);
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

  typedef void OrchWorkflowEngine;

  const std::string workflow
    (utils::require_and_read_file ("workflows/capabilities_no_mandatory.pnet"));

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create_with_start_called("orchestrator_0", addrOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, sdpa::MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create_with_start_called("agent_1", addrAgent, arrAgentMasterInfo);

  {
    const utils::drts_worker worker_0 ("drts_0", "agent_1", "", TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH,  kvs_host(), kvs_port());
    const utils::drts_worker worker_1 ("drts_1", "agent_1", "", TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH,  kvs_host(), kvs_port());
    const utils::drts_worker worker_2 ("drts_2", "agent_1", "", TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH,  kvs_host(), kvs_port());

    boost::thread threadClient = boost::thread(boost::bind(&utils::client::submit_job_and_wait_for_termination, workflow, "sdpac", "orchestrator_0"));

    if(threadClient.joinable())
      threadClient.join();
    LOG( INFO, "The client thread joined the main thread!" );
  }

  ptrAgent->shutdown();
  ptrOrch->shutdown();

  LOG( INFO, "The test case Test2 terminated!");
}
