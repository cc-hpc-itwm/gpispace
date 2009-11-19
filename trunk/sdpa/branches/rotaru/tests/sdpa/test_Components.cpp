/*
 * =====================================================================================
 *
 *       Filename:  test_Components.cpp
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
#include "test_Components.hpp"
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/aggregator/Aggregator.hpp>
#include <sdpa/daemon/nre/NRE.hpp>
#include <seda/StageRegistry.hpp>
#include <gwes/GWES.h>


using namespace std;
using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( TestComponents );

TestComponents::TestComponents() :
	SDPA_INIT_LOGGER("sdpa.tests.TestComponents"),
    m_nITER(1),
    m_sleep_interval(1000000)
{
}

TestComponents::~TestComponents()
{}


string TestComponents::read_workflow(string strFileName)
{
	ifstream f(strFileName.c_str());
	ostringstream os;
	os.str("");

	if( f.is_open() )
	{
		char c;
		while (f.get(c)) os<<c;
		f.close();
	}else
		cout<<"Unable to open file " << strFileName << ", error: " <<strerror(errno);

	return os.str();
}

void TestComponents::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	const sdpa::client::config_t config = sdpa::client::ClientApi::config();
	m_ptrUser = sdpa::client::ClientApi::create( config, "orchestrator_0" );
	m_ptrUser->configure_network(config);

	seda::Stage::Ptr user_stage = seda::StageRegistry::instance().lookup(m_ptrUser->input_stage());

//	m_strWorkflow = read_workflow("workflows/masterworkflow-sdpa-test.gwdl");
	m_strWorkflow = read_workflow("workflows/remig.master.gwdl");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);
}

void TestComponents::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	m_ptrUser.reset();

	seda::StageRegistry::instance().clear();
}

void TestComponents::testComponents()
{
	SDPA_LOG_DEBUG("*****testComponents*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create( "orchestrator_0", "127.0.0.1:5000");
	sdpa::daemon::Orchestrator::start(ptrOrch);

	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::Aggregator::create( "aggregator_0",  "127.0.0.1:5001",
																			   "orchestrator_0", "127.0.0.1:5000");
	sdpa::daemon::Aggregator::start(ptrAgg);

	sdpa::daemon::NRE::ptr_t ptrNRE_0 = sdpa::daemon::NRE::create( "NRE_0",  "127.0.0.1:5002",
																   "aggregator_0", "127.0.0.1:5001",
																   "127.0.0.1:8000" );
	sdpa::daemon::NRE::start(ptrNRE_0);

	/*sdpa::daemon::NRE::ptr_t ptrNRE_1 = sdpa::daemon::NRE::create( "NRE_1",  "127.0.0.1:5003",
																	 "aggregator_0", "127.0.0.1:5001",
																	 "127.0.0.1:8001" );
	sdpa::daemon::NRE::start(ptrNRE_1);*/

	for(int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrUser->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrUser->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrUser->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrUser->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrUser->deleteJob(job_id_user);
	}

	sdpa::daemon::Orchestrator::shutdown(ptrOrch);
	sdpa::daemon::Aggregator::shutdown(ptrAgg);
	sdpa::daemon::NRE::shutdown(ptrNRE_0);
	//sdpa::daemon::NRE::shutdown(ptrNRE_1);

    sleep(1);
	SDPA_LOG_DEBUG("Test finished!");
}
