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

namespace po = boost::program_options;

using namespace std;
using namespace sdpa::tests;

#define NO_GUI ""

CPPUNIT_TEST_SUITE_REGISTRATION( TestComponents );


namespace unit_tests {

		class SchedulerNRE : public sdpa::daemon::SchedulerNRE
		{
		public:
			typedef sdpa::daemon::SynchronizedQueue<std::list<gwes::activity_t*> > ActivityQueue;

			SDPA_DECLARE_LOGGER();

			SchedulerNRE( sdpa::daemon::IComm* pHandler, std::string workerUrl ):
				sdpa::daemon::SchedulerNRE(pHandler, workerUrl),
				SDPA_INIT_LOGGER("Scheduler "+pHandler->name()) {}

			virtual ~SchedulerNRE()
	        {
	          try
	          {
	            stop();
	          }
	          catch (const std::exception &ex)
	          {
	            LOG(ERROR, "could not stop nre-scheduler: " << ex.what());
	          }
	          catch (...)
	          {
	            LOG(ERROR, "could not stop nre-scheduler (unknown error)");
	          }
	        }

			void execute(const gwes::activity_t& activity) throw (std::exception)
			{
				SDPA_LOG_DEBUG("Execute activity ...");

				gwes::Activity& gwes_act = (gwes::Activity&)(activity);
				sdpa::wf::Activity result;
				try
				{
					sdpa::wf::Activity act(sdpa::wf::glue::wrap(gwes_act));

					ptr_comm_handler_->activityStarted(gwes_act);

					//result = m_worker_.execute(act, act.properties().get<unsigned long>("walltime", 0));
					result.state () = sdpa::wf::Activity::ACTIVITY_FINISHED;
					result.reason() = "";

					sdpa::wf::glue::unwrap(result, gwes_act);
				}
				catch( const boost::thread_interrupted &)
				{
					SDPA_LOG_ERROR("could not execute activity: interrupted" );
					result.state () = sdpa::wf::Activity::ACTIVITY_FAILED;
					result.reason() = "interrupted";
				}
				catch (const std::exception &ex)
				{
					SDPA_LOG_ERROR("could not execute activity: " << ex.what());
					result.state () = sdpa::wf::Activity::ACTIVITY_FAILED;
					result.reason() = ex.what();
				}

				sdpa::parameter_list_t output = *gwes_act.getTransitionOccurrence()->getTokens();

				gwes::activity_id_t act_id = activity.getID();
				gwes::workflow_id_t wf_id  = activity.getOwnerWorkflowID();

				SDPA_LOG_DEBUG("Finished activity execution: notify GWES ...");
				if( result.state() == sdpa::wf::Activity::ACTIVITY_FINISHED )
				{
					ptr_comm_handler_->activityFinished(gwes_act);
					ptr_comm_handler_->gwes()->activityFinished(wf_id, act_id, output);
				}
				else if( result.state() == sdpa::wf::Activity::ACTIVITY_FAILED )
				{
					ptr_comm_handler_->activityFailed(gwes_act);
					ptr_comm_handler_->gwes()->activityFailed(wf_id, act_id, output);
				}
				else if( result.state() == sdpa::wf::Activity::ACTIVITY_CANCELED )
				{
					ptr_comm_handler_->activityCancelled(gwes_act);
					ptr_comm_handler_->gwes()->activityCanceled(wf_id, act_id);
				}
				else
				{
					ptr_comm_handler_->activityFailed(gwes_act);
					ptr_comm_handler_->gwes()->activityFailed(wf_id, act_id, output);
				}
			}

			void start()throw (std::exception)
			{
				SchedulerImpl::start();
				m_threadExecutor = boost::thread(boost::bind(&SchedulerNRE::runExecutor, this));
				SDPA_LOG_DEBUG("Executor thread started ...");
			}

			void stop()
			{
				SchedulerImpl::stop();
				m_threadExecutor.interrupt();

				SDPA_LOG_DEBUG("Executor thread before join ...");
				m_threadExecutor.join();
			}
	  };


	  class NRE : public sdpa::daemon::NRE
	  {
	  public:
		typedef sdpa::shared_ptr<NRE> ptr_t;
		//SDPA_DECLARE_LOGGER();

		NRE(  const std::string& name, const std::string& url,
			  const std::string& masterName, const std::string& masterUrl,
			  const std::string& workerUrl,  const std::string guiUrl = "" )
				: 	sdpa::daemon::NRE::NRE(  name, url, masterName, masterUrl, workerUrl, guiUrl, true )
						  //,SDPA_INIT_LOGGER(name)
		{
			//SDPA_LOG_DEBUG("TesNRE constructor called ...");
			//ptr_scheduler_.reset();
			sdpa::daemon::Scheduler* ptr_scheduler =  new unit_tests::SchedulerNRE(this, workerUrl);

		}

		virtual ~NRE()
		{
			//SDPA_LOG_DEBUG("TestNRE destructor called ...");
			daemon_stage_ = NULL;

		}

		static NRE::ptr_t create( const std::string& name, const std::string& url,
								  const std::string& masterName, const std::string& masterUrl,
								  const std::string& workerUrl,  const std::string guiUrl = "")
		{
			 return NRE::ptr_t(new NRE( name, url, masterName, masterUrl, workerUrl, guiUrl ));
		}

		static void start(NRE::ptr_t ptrNRE)
		{
			dsm::DaemonFSM::create_daemon_stage(ptrNRE);
			ptrNRE->configure_network( ptrNRE->url(), ptrNRE->masterName(), ptrNRE->masterUrl() );
			sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
			dsm::DaemonFSM::start(ptrNRE, ptrCfg);
		}

	  };
}

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

void TestComponents::testComponents()
{
	SDPA_LOG_DEBUG("*****testComponents*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create("orchestrator_0", "127.0.0.1:7000", "workflows" );
	sdpa::daemon::Orchestrator::start(ptrOrch);

	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::Aggregator::create("aggregator_0", "127.0.0.1:7001","orchestrator_0", "127.0.0.1:7000");
	sdpa::daemon::Aggregator::start(ptrAgg);

	unit_tests::NRE::ptr_t ptrNRE_0 = unit_tests::NRE::create("NRE_0",  "127.0.0.1:7002","aggregator_0", "127.0.0.1:7001", "127.0.0.1:8000" );
	//sdpa::daemon::NRE::ptr_t ptrNRE_1 = sdpa::daemon::NRE::create( "NRE_1",  "127.0.0.1:7003","aggregator_0", "127.0.0.1:7001" );

    try
    {
    	unit_tests::NRE::start(ptrNRE_0);
    	//sdpa::daemon::NRE::start(ptrNRE_1);
    }
    catch (const std::exception &ex)
    {
    	LOG(FATAL, "could not start NRE: " << ex.what());
    	LOG(WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");
    	/* CPPUNIT_ASSERT_MESSAGE("could not start NRE", false); */
    	sdpa::daemon::Orchestrator::shutdown(ptrOrch);
    	sdpa::daemon::Aggregator::shutdown(ptrAgg);
    	unit_tests::NRE::shutdown(ptrNRE_0);
    	//sdpa::daemon::NRE::shutdown(ptrNRE_1);

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

	sdpa::daemon::Orchestrator::shutdown(ptrOrch);
	sdpa::daemon::Aggregator::shutdown(ptrAgg);
	unit_tests::NRE::shutdown(ptrNRE_0);
	//sdpa::daemon::NRE::shutdown(ptrNRE_1);

    sleep(1);
	SDPA_LOG_DEBUG("Test finished!");
}
