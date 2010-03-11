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
#include <sdpa/daemon/nre/SchedulerNRE.hpp>
#include <seda/StageRegistry.hpp>
#include <gwes/GWES.h>
#include <tests/sdpa/DummyGwes.hpp>

namespace po = boost::program_options;

using namespace std;
using namespace sdpa::tests;

#define NO_GUI ""

CPPUNIT_TEST_SUITE_REGISTRATION( TestComponents );

/*
namespace unit_tests {

		class SchedulerNRE : public sdpa::daemon::SchedulerNRE
		{
		public:
			typedef sdpa::daemon::SynchronizedQueue<std::list<gwes::activity_t*> > ActivityQueue;

			SDPA_DECLARE_LOGGER();

			SchedulerNRE( sdpa::daemon::IComm* pHandler, std::string workerUrl ):
				sdpa::daemon::SchedulerNRE(pHandler, workerUrl),
				SDPA_INIT_LOGGER("sdpa::daemon::Scheduler "+pHandler->name()) {}


			 void execute(const  sdpa::daemon::Job::ptr_t& pJob) throw (std::exception)
			 {
				id_type act_id = pJob->id().str();
				SDPA_LOG_DEBUG("Execute the activity "<<act_id);

				if(!ptr_comm_handler_)
				{
					SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
					stop();
					return;
				}


				// call here the NreWorkerClient
				result_type output; // to be fiile-in by the NreWorkerClient
				ptr_comm_handler_->gwes()->finished(act_id, output);
			 }
	  };


	  class NRE : public sdpa::daemon::NRE
	  {
	  public:
		typedef sdpa::shared_ptr<NRE> ptr_t;
		//SDPA_DECLARE_LOGGER();

		NRE(  const std::string& name, const std::string& url,
			  const std::string& masterName, const std::string& masterUrl,
			  const std::string& workerUrl,  const std::string guiUrl = "",
			  const bool bExtSched = false, const bool bUseDummyWE = false  )
				: 	sdpa::daemon::NRE<DummyGwes>::NRE(  name, url, masterName, masterUrl, workerUrl, guiUrl, bExtSched, bUseDummyWE )
						  //,SDPA_INIT_LOGGER(name)
		{
			//SDPA_LOG_DEBUG("TesNRE constructor called ...");
			//ptr_scheduler_.reset();
			sdpa::daemon::Scheduler* ptr_scheduler =  new sdpa::daemon::SchedulerNRE(this, workerUrl);

		}

		virtual ~NRE()
		{
			//SDPA_LOG_DEBUG("TestNRE destructor called ...");
			daemon_stage_ = NULL;

		}

		static NRE<DummyGwes>::ptr_t create( const std::string& name, const std::string& url,
								  const std::string& masterName, const std::string& masterUrl,
								  const std::string& workerUrl,  const std::string guiUrl = "",
								  const bool bExtSched = false, const bool bUseDummyWE = false )
		{
			 return NRE<DummyGwes>::ptr_t(new NRE( name, url, masterName, masterUrl, workerUrl, guiUrl, bExtSched, bUseDummyWE ));
		}

		static void start(NRE<DummyGwes>::ptr_t ptrNRE)
		{
			dsm::DaemonFSM::create_daemon_stage(ptrNRE);
			ptrNRE->configure_network( ptrNRE->url(), ptrNRE->masterName(), ptrNRE->masterUrl() );
			sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
			dsm::DaemonFSM::start(ptrNRE, ptrCfg);
		}

	  };
}
*/

TestComponents::TestComponents() :
	SDPA_INIT_LOGGER("sdpa.tests.TestComponents"),
    m_nITER(10),
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

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
    cav.push_back("--orchestrator=orchestrator_0");
    cav.push_back("--network.location=orchestrator_0:127.0.0.1:5000");
    config.parse_command_line(cav);

	m_ptrUser = sdpa::client::ClientApi::create( config );
	m_ptrUser->configure_network( config );

	seda::Stage::Ptr user_stage = seda::StageRegistry::instance().lookup(m_ptrUser->input_stage());

	m_strWorkflow = read_workflow("workflows/masterworkflow-sdpa-test.gwdl");
				    //read_workflow("workflows/remig.master.gwdl");

	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);
}

void TestComponents::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	m_ptrUser.reset();
	seda::StageRegistry::instance().clear();
}

/*
void TestComponents::testComponentsRealGWES()
{
	SDPA_LOG_DEBUG("*****testComponents*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";
	string strGuiUrl = "";

	sdpa::daemon::Orchestrator<DummyGwes>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<DummyGwes>::create("orchestrator_0", "127.0.0.1:7000", "workflows" );
	sdpa::daemon::Orchestrator<DummyGwes>::start(ptrOrch);

	sdpa::daemon::Aggregator<DummyGwes>::ptr_t ptrAgg = sdpa::daemon::Aggregator<DummyGwes>::create("aggregator_0", "127.0.0.1:7001","orchestrator_0", "127.0.0.1:7000");
	sdpa::daemon::Aggregator<DummyGwes>::start(ptrAgg);

	// use external scheduler and real GWES
	sdpa::daemon::NRE<DummyGwes>::ptr_t ptrNRE_0 = sdpa::daemon::NRE<DummyGwes>::create("NRE_0",  "127.0.0.1:7002","aggregator_0", "127.0.0.1:7001", "127.0.0.1:8000", strGuiUrl, true );
	//sdpa::daemon::NRE<DummyGwes>::ptr_t ptrNRE_1 = sdpa::daemon::NRE<DummyGwes>::create( "NRE_1",  "127.0.0.1:7003","aggregator_0", "127.0.0.1:7001" );

    try
    {
    	sdpa::daemon::NRE<DummyGwes>::start(ptrNRE_0);
    	//sdpa::daemon::NRE<DummyGwes>::start(ptrNRE_1);
    }
    catch (const std::exception &ex)
    {
    	LOG(FATAL, "could not start NRE: " << ex.what());
    	LOG(WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

    	sdpa::daemon::Orchestrator<DummyGwes>::shutdown(ptrOrch);
    	sdpa::daemon::Aggregator<DummyGwes>::shutdown(ptrAgg);
    	sdpa::daemon::NRE<DummyGwes>::shutdown(ptrNRE_0);
    	//sdpa::daemon::NRE<DummyGwes>::shutdown(ptrNRE_1);

    	return;
    }

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

	sdpa::daemon::Orchestrator<DummyGwes>::shutdown(ptrOrch);
	sdpa::daemon::Aggregator<DummyGwes>::shutdown(ptrAgg);
	sdpa::daemon::NRE<DummyGwes>::shutdown(ptrNRE_0);
	//sdpa::daemon::NRE<DummyGwes>::shutdown(ptrNRE_1);

    sleep(1);
	SDPA_LOG_DEBUG("Test finished!");
}
*/

void TestComponents::testComponentsDummyGWES()
{
	SDPA_LOG_DEBUG("*****testComponents*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	bool bUseExtSched  = false;
	string strGuiUrl   = "";

	sdpa::daemon::Orchestrator<DummyGwes>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<DummyGwes>::create("orchestrator_0", "127.0.0.1:7000", "workflows");
	sdpa::daemon::Orchestrator<DummyGwes>::start(ptrOrch);

	sdpa::daemon::Aggregator<DummyGwes>::ptr_t ptrAgg = sdpa::daemon::Aggregator<DummyGwes>::create("aggregator_0", "127.0.0.1:7001","orchestrator_0", "127.0.0.1:7000");
	sdpa::daemon::Aggregator<DummyGwes>::start(ptrAgg);

	// use external scheduler and dummy GWES
	sdpa::daemon::NRE<DummyGwes>::ptr_t ptrNRE_0 = sdpa::daemon::NRE<DummyGwes>::create("NRE_0",  "127.0.0.1:7002","aggregator_0", "127.0.0.1:7001", "127.0.0.1:8000", strGuiUrl, bUseExtSched );
	//sdpa::daemon::NRE<DummyGwes>::ptr_t ptrNRE_1 = sdpa::daemon::NRE<DummyGwes>::create( "NRE_1",  "127.0.0.1:7003","aggregator_0", "127.0.0.1:7001" );

    try
    {
    	sdpa::daemon::NRE<DummyGwes>::start(ptrNRE_0);
    	//sdpa::daemon::NRE<DummyGwes>::start(ptrNRE_1);
    }
    catch (const std::exception &ex)
    {
    	LOG(FATAL, "could not start NRE: " << ex.what());
    	LOG(WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

    	sdpa::daemon::Orchestrator<DummyGwes>::shutdown(ptrOrch);
    	sdpa::daemon::Aggregator<DummyGwes>::shutdown(ptrAgg);
    	sdpa::daemon::NRE<DummyGwes>::shutdown(ptrNRE_0);
    	//sdpa::daemon::NRE<DummyGwes>::shutdown(ptrNRE_1);

    	return;
    }

	for( int k=0; k<m_nITER; k++ )
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

	sdpa::daemon::Orchestrator<DummyGwes>::shutdown(ptrOrch);
	sdpa::daemon::Aggregator<DummyGwes>::shutdown(ptrAgg);
	sdpa::daemon::NRE<DummyGwes>::shutdown(ptrNRE_0);
	//sdpa::daemon::NRE<DummyGwes>::shutdown(ptrNRE_1);

    sleep(1);
	SDPA_LOG_DEBUG("Test finished!");
}
