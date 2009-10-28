#include "test_C2D2D2DDummyGwes.hpp"

#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util/util.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/ConfigRequestEvent.hpp>
#include <sdpa/events/ConfigReplyEvent.hpp>

#include <boost/shared_ptr.hpp>
#include "DummyGwes.hpp"

#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>

#include <sdpa/sdpa-config.hpp>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;
using namespace sdpa::daemon;
using namespace sdpa::fsm::smc;

const int NITER = 1;
const int user_user_sleep_interval = 10000;

string answerStrategy = "";

class SchedulerNRE : public SchedulerImpl
{
public:
	SchedulerNRE(sdpa::Sdpa2Gwes* ptr_Sdpa2Gwes, sdpa::daemon::IComm* pHandler):
		SchedulerImpl(ptr_Sdpa2Gwes,  pHandler) {}

	virtual ~SchedulerNRE() { }

	void run()
	{
		SDPA_LOG_DEBUG("Scheduler thread running ...");

		while(!bStopRequested)
		{
			try
			{
				Job::ptr_t pJob = jobs_to_be_scheduled.pop_and_wait(m_timeout);

				// execute the job and ...
				// ... submit a JobFinishedEvent to the master
				if( answerStrategy == "finished" )
				{
					SDPA_LOG_DEBUG("Slave: send JobFinishedEvent to "<<ptr_comm_handler_->master());
					JobFinishedEvent::Ptr pJobFinEvt( new JobFinishedEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master(), pJob->id() ) );
					ptr_comm_handler_->sendEvent(ptr_comm_handler_->to_master_stage(), pJobFinEvt);
				}
				else if( answerStrategy == "failed" )
				{
					SDPA_LOG_DEBUG("Slave: send JobFailedEvent to "<<ptr_comm_handler_->master());
					JobFailedEvent::Ptr pJobFailEvt( new JobFailedEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master(), pJob->id() ) );
					ptr_comm_handler_->sendEvent(ptr_comm_handler_->to_master_stage(), pJobFailEvt);
				}
				else if( answerStrategy == "cancelled" )
				{
					SDPA_LOG_DEBUG("Slave: send CancelJobAckEvent to "<<ptr_comm_handler_->master());
					CancelJobAckEvent::Ptr pCancelAckEvt( new CancelJobAckEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master(), pJob->id() ) );
					ptr_comm_handler_->sendEvent(ptr_comm_handler_->to_master_stage(), pCancelAckEvt);
				}

				check_post_request();
			}
			catch( const boost::thread_interrupted & )
			{
				SDPA_LOG_DEBUG("Thread interrupted ...");
				bStopRequested = true;
			}
			catch( const sdpa::daemon::QueueEmpty &)
			{
				//SDPA_LOG_DEBUG("Queue empty exception");
				check_post_request();
			}
		}
	}
};


class NreDaemon : public DaemonFSM
{
public:
	SDPA_DECLARE_LOGGER();
	NreDaemon(	const std::string &name,
							seda::Stage* ptrToMasterStage,
							seda::Stage* ptrToSlaveStage,
							sdpa::Sdpa2Gwes*  pArgSdpa2Gwes)
	: DaemonFSM( name, ptrToMasterStage, ptrToSlaveStage, pArgSdpa2Gwes),
	  SDPA_INIT_LOGGER(name)
	{
		ptr_scheduler_ =  Scheduler::ptr_t(new SchedulerNRE(pArgSdpa2Gwes, this));
	}

	 virtual ~NreDaemon() {  }

};


CPPUNIT_TEST_SUITE_REGISTRATION( C2D2D2DDummyGwesTest );

C2D2D2DDummyGwesTest::C2D2D2DDummyGwesTest() :
	SDPA_INIT_LOGGER("sdpa.tests.C2D2D2DDummyGwesTest")
{
}

C2D2D2DDummyGwesTest::~C2D2D2DDummyGwesTest()
{}


string C2D2D2DDummyGwesTest::read_workflow(string strFileName)
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

void C2D2D2DDummyGwesTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	m_ptrSdpa2GwesOrch = new DummyGwes();
	m_ptrSdpa2GwesAgg  = new DummyGwes();

	m_ptrUser = sdpa::client::ClientApi::create("empty config", sdpa::daemon::USER, sdpa::daemon::ORCHESTRATOR);

	seda::Stage::Ptr user_stage = seda::StageRegistry::instance().lookup(sdpa::daemon::USER);


	m_ptrOrch = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::ORCHESTRATOR,
												 user_stage.get(),
												 NULL,
												 m_ptrSdpa2GwesOrch) ); // Orchestrator gwes instance
	DaemonFSM::create_daemon_stage(m_ptrOrch);


	m_ptrAgg = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::AGGREGATOR,
								m_ptrOrch->daemon_stage(),
								NULL,
								m_ptrSdpa2GwesAgg) ); // Aggregator gwes instance
	DaemonFSM::create_daemon_stage(m_ptrAgg);

	m_ptrNRE = DaemonFSM::ptr_t (new NreDaemon( sdpa::daemon::NRE,
										m_ptrAgg->daemon_stage(),
										NULL,
										NULL) ); // No gwes
	DaemonFSM::create_daemon_stage(m_ptrNRE);


	m_ptrAgg->set_to_slave_stage(m_ptrNRE->daemon_stage());
	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());

	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);
	DaemonFSM::start(m_ptrNRE);


	m_strWorkflow = read_workflow("workflows/masterworkflow-sdpa-test.gwdl");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);
}

void C2D2D2DDummyGwesTest::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	m_ptrUser.reset();

	m_ptrNRE->stop();
	m_ptrAgg->stop();
	m_ptrOrch->stop();

	seda::StageRegistry::instance().clear();

	//m_ptrOrch.reset();
	delete m_ptrSdpa2GwesOrch;
	delete m_ptrSdpa2GwesAgg;
}

void C2D2D2DDummyGwesTest::testDaemonFSM_JobFinished()
{
	SDPA_LOG_DEBUG("************************************testDaemonFSM_JobFinished******************************************"<<std::endl);
	answerStrategy = "finished";

	for(int k=0; k<NITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrUser->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("***********JOB #"<<k<<"************");

		std::string job_status =  m_ptrUser->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrUser->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(user_user_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrUser->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrUser->deleteJob(job_id_user);
	}

	InterruptEvent::Ptr pEvtIntNRE( new InterruptEvent(m_ptrNRE->name(), m_ptrNRE->name() ));
	m_ptrNRE->daemon_stage()->send(pEvtIntNRE);

	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	SDPA_LOG_DEBUG("User finished!");
}

void C2D2D2DDummyGwesTest::testDaemonFSM_JobFailed()
{
	SDPA_LOG_DEBUG("************************************testDaemonFSM_JobFailed******************************************"<<std::endl);
	answerStrategy = "failed";

	for(int k=0; k<NITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrUser->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("***********JOB #"<<k<<"************");

		std::string job_status =  m_ptrUser->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrUser->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(user_user_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrUser->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrUser->deleteJob(job_id_user);
	}

	InterruptEvent::Ptr pEvtIntNRE( new InterruptEvent(m_ptrNRE->name(), m_ptrNRE->name() ));
	m_ptrNRE->daemon_stage()->send(pEvtIntNRE);

	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	SDPA_LOG_DEBUG("User finished!");
}


void C2D2D2DDummyGwesTest::testDaemonFSM_JobCancelled()
{
	SDPA_LOG_DEBUG("************************************testDaemonFSM_JobCancelled******************************************"<<std::endl);
	answerStrategy = "cancelled";

	for(int k=0; k<NITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrUser->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("***********JOB #"<<k<<"************");

		std::string job_status =  m_ptrUser->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrUser->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(user_user_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrUser->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrUser->deleteJob(job_id_user);
	}

	InterruptEvent::Ptr pEvtIntNRE( new InterruptEvent(m_ptrNRE->name(), m_ptrNRE->name() ));
	m_ptrNRE->daemon_stage()->send(pEvtIntNRE);

	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	SDPA_LOG_DEBUG("User finished!");
}
