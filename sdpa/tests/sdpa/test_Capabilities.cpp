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
  const std::string workflow
    (utils::require_and_read_file ("workflows/capabilities.pnet"));

  {
    const utils::orchestrator orchestrator ("orchestrator_0", "127.0.0.1");
    const utils::agent<we::mgmt::layer> agent
      ("agent_0", "127.0.0.1", orchestrator);

    {
      const utils::drts_worker worker_0
        ( "drts_0", agent
        , "A"
        , TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH
        , kvs_host(), kvs_port()
        );
      const utils::drts_worker worker_1
        ( "drts_1", agent
        , "B"
        , TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH
        , kvs_host(), kvs_port()
        );
      const utils::drts_worker worker_2
        ( "drts_2", agent
        , "A"
        , TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH
        , kvs_host(), kvs_port()
        );

      utils::client::submit_job_and_wait_for_termination
        (workflow, "sdpac", orchestrator.name());
    }
  }
}

BOOST_AUTO_TEST_CASE( testCapabilities_NoMandatoryReq )
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/capabilities_no_mandatory.pnet"));

  {
    const utils::orchestrator orchestrator ("orchestrator_0", "127.0.0.1");
    const utils::agent<we::mgmt::layer> agent
      ("agent_0", "127.0.0.1", orchestrator);

    {
      const utils::drts_worker worker_0
        ( "drts_0", agent
        , ""
        , TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH
        , kvs_host(), kvs_port()
        );
      const utils::drts_worker worker_1
        ( "drts_1", agent
        , ""
        , TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH
        , kvs_host(), kvs_port()
        );
      const utils::drts_worker worker_2
        ( "drts_2", agent
        , ""
        , TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH
        , kvs_host(), kvs_port()
        );

      utils::client::submit_job_and_wait_for_termination
        (workflow, "sdpac", orchestrator.name());
    }
  }
}
