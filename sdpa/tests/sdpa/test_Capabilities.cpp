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
#include <tests/sdpa/CreateDrtsWorker.hpp>
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


  sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "A", TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH, kvs_host(), kvs_port()) );
  boost::thread drts_0_thread = boost::thread( &fhg::core::kernel_t::run, drts_0 );

  sdpa::shared_ptr<fhg::core::kernel_t> drts_1( createDRTSWorker("drts_1", "agent_0", "B", TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH,  kvs_host(), kvs_port()) );
  boost::thread drts_1_thread = boost::thread( &fhg::core::kernel_t::run, drts_1 );

  sdpa::shared_ptr<fhg::core::kernel_t> drts_2( createDRTSWorker("drts_2", "agent_0", "A", TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH,  kvs_host(), kvs_port()) );
  boost::thread drts_2_thread = boost::thread( &fhg::core::kernel_t::run, drts_2 );

  boost::thread threadClient = boost::thread(boost::bind(&utils::client::submit_job_and_wait_for_termination, workflow, "sdpac", "orchestrator_0"));

  if(threadClient.joinable())
    threadClient.join();
  LOG( INFO, "The client thread joined the main thread!" );

  drts_0->stop();
  if(drts_0_thread.joinable())
    drts_0_thread.join();

  drts_1->stop();
  if(drts_1_thread.joinable())
    drts_1_thread.join();

  drts_2->stop();
  if(drts_2_thread.joinable())
    drts_2_thread.join();

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


  sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_1", "", TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH,  kvs_host(), kvs_port()) );
  boost::thread drts_0_thread = boost::thread( &fhg::core::kernel_t::run, drts_0 );

  sdpa::shared_ptr<fhg::core::kernel_t> drts_1( createDRTSWorker("drts_1", "agent_1", "", TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH,  kvs_host(), kvs_port()) );
  boost::thread drts_1_thread = boost::thread( &fhg::core::kernel_t::run, drts_1 );

  sdpa::shared_ptr<fhg::core::kernel_t> drts_2( createDRTSWorker("drts_2", "agent_1", "", TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH,  kvs_host(), kvs_port()) );
  boost::thread drts_2_thread = boost::thread( &fhg::core::kernel_t::run, drts_2 );

  boost::thread threadClient = boost::thread(boost::bind(&utils::client::submit_job_and_wait_for_termination, workflow, "sdpac", "orchestrator_0"));

  if(threadClient.joinable())
    threadClient.join();
  LOG( INFO, "The client thread joined the main thread!" );

  drts_0->stop();
  if(drts_0_thread.joinable())
          drts_0_thread.join();

  drts_1->stop();
  if(drts_1_thread.joinable())
    drts_1_thread.join();

  drts_2->stop();
  if(drts_2_thread.joinable())
    drts_2_thread.join();

  ptrAgent->shutdown();
  ptrOrch->shutdown();

  LOG( INFO, "The test case Test2 terminated!");
}
