#include "test_D2D2DDummyGwes.hpp"

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

class UserStrategy : public seda::Strategy
{
public:
	 typedef std::tr1::shared_ptr<UserStrategy> Ptr;
	 UserStrategy(const std::string& name): seda::Strategy(name), SDPA_INIT_LOGGER(name)  {}
	 void perform(const seda::IEvent::Ptr& pEvt)
	 {

		 if( dynamic_cast<WorkerRegistrationAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received WorkerRegistrationAckEvent!");
		 }
		 else if( dynamic_cast<ConfigReplyEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received ConfigReplyEvent!");
		 }
		 else if( dynamic_cast<SubmitJobAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received SubmitJobAckEvent!");
		 }
		 else if( dynamic_cast<SubmitJobEvent*>(pEvt.get())  )
		 {
		 	 SDPA_LOG_DEBUG("Received SubmitJobEvent!");
		 }
		 else if( dynamic_cast<JobFinishedAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobFinishedAckEvent!");
		 }
		 else if( dynamic_cast<JobFailedAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobFailedAckEvent!");
		 }
		 else if( dynamic_cast<JobStatusReplyEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobStatusReplyEvent!");
		 }
		 else if( dynamic_cast<JobResultsReplyEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobResultsReplyEvent!");
		 }
		 else if( dynamic_cast<JobFinishedEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobFinishedEvent!");
		 }
		 else if( dynamic_cast<DeleteJobAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received DeleteJobAckEvent!");
		 }
		 else if( dynamic_cast<JobResultsReplyEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobResultsReplyEvent!");
		 }
		 else if( dynamic_cast<CancelJobEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received CancelJobEvent!");
		 }
		 else if( dynamic_cast<CancelJobAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received CancelJobAckEvent!");
		 }
		 else if( dynamic_cast<ErrorEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received ErrorEvent!");
		 }
		 else
		 {
		     SDPA_LOG_DEBUG("Received unexpected event of type "<<typeid(*pEvt).name()<<"!");
		 }

		 eventQueue.push(pEvt);
	 }

	 std::string str() const
	 {
		std::ostringstream ostream(std::ostringstream::out);
		eventQueue_t::const_iterator it;
		for (it = eventQueue.begin(); it != eventQueue.end(); it++) {
			ostream <<typeid(*(it->get())).name() << std::endl;
		}
		return ostream.str();
	 }

	 template <typename T>
	 typename T::Ptr WaitForEvent(sdpa::events::ErrorEvent::Ptr& pErrorEvt )
	 {
		 seda::IEvent::Ptr pEvent;
		 pErrorEvt.reset();

		 ostringstream os;
		 std::string strName("");
		 os<<"Waiting for event "<<typeid(T).name()<<" ... ";
		 SDPA_LOG_DEBUG(os.str());

		 typename T::Ptr ptrT;
		 do
		 {
			pEvent = eventQueue.pop_and_wait();
			os.str("");
			os<<"Slave: Popped-up event "<<typeid(*(pEvent.get())).name();
			SDPA_LOG_DEBUG(os.str());

#if USE_STL_TR1
			ptrT = std::tr1::dynamic_pointer_cast<T, seda::IEvent>( pEvent );
			pErrorEvt = std::tr1::dynamic_pointer_cast<sdpa::events::ErrorEvent, seda::IEvent>( pEvent );
#else
			ptrT = boost::dynamic_pointer_cast<T, seda::IEvent>( pEvent );
			pErrorEvt = boost::dynamic_pointer_cast<sdpa::events::ErrorEvent, seda::IEvent>( pEvent );
#endif

		 }while( !ptrT.get() && !pErrorEvt.get());

		 return  ptrT;
	 }

	 typedef SynchronizedQueue<std::list<seda::IEvent::Ptr> > eventQueue_t;

	 eventQueue_t eventQueue;

	 SDPA_DECLARE_LOGGER();
};

CPPUNIT_TEST_SUITE_REGISTRATION( D2D2DDummyGwesTest );

D2D2DDummyGwesTest::D2D2DDummyGwesTest() :
	SDPA_INIT_LOGGER("sdpa.tests.D2D2DDummyGwesTest")
{
}

D2D2DDummyGwesTest::~D2D2DDummyGwesTest()
{}


string D2D2DDummyGwesTest::read_workflow(string strFileName)
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

void D2D2DDummyGwesTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	m_ptrSdpa2GwesOrch = new DummyGwes();
	m_ptrSdpa2GwesAgg  = new DummyGwes();

	m_ptrUserStrategy = seda::Strategy::Ptr( new UserStrategy("User") );
	m_ptrToUserStage  = seda::Stage::Ptr(new seda::Stage("to_master_stage", m_ptrUserStrategy) );

	m_ptrOrch = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::ORCHESTRATOR,
												m_ptrToUserStage.get(),
												NULL,
												m_ptrSdpa2GwesOrch) ); // Orchestrator gwes instance
	DaemonFSM::create_daemon_stage(m_ptrOrch);


	m_ptrAgg = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::AGGREGATOR,
								m_ptrOrch->daemon_stage(),
								NULL,
								m_ptrSdpa2GwesAgg) ); // Aggregator gwes instance
	DaemonFSM::create_daemon_stage(m_ptrAgg);


	m_strWorkflow = read_workflow("workflows/masterworkflow-sdpa-test.gwdl");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);
}

void D2D2DDummyGwesTest::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	seda::StageRegistry::instance().lookup(m_ptrToUserStage->name())->stop();

	m_ptrNRE->stop();
	m_ptrAgg->stop();
	m_ptrOrch->stop();

	seda::StageRegistry::instance().clear();

	//m_ptrOrch.reset();
	delete m_ptrSdpa2GwesOrch;
	delete m_ptrSdpa2GwesAgg;
}

void D2D2DDummyGwesTest::testDaemonFSM_JobFinished()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobFinished******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	answerStrategy = "finished";
	string strFromUser(sdpa::daemon::USER);
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

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrToUserStage);
	m_ptrToUserStage->start();

	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	UserStrategy* pUserStr = dynamic_cast<UserStrategy*>(m_ptrUserStrategy.get());

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		// check if the job finished
		SDPA_LOG_DEBUG("User: query "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
		QueryJobStatusEvent::Ptr pEvtQueryStatus(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtQueryStatus);

		// wait for a JobStatusReplyEvent
		JobStatusReplyEvent::Ptr pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());

		while( pJobStatusReplyEvent->status().find("Finished") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Failed") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Cancelled") == std::string::npos)
		{
			SDPA_LOG_DEBUG("User: ask the "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
			QueryJobStatusEvent::Ptr pEvtQueryStNew(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
			m_ptrOrch->daemon_stage()->send(pEvtQueryStNew);

			// wait for a JobStatusReplyEvent
			pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());
			usleep(user_user_sleep_interval);
		}

			// if the job is in the finished or failed state, one is allowed
		// to retriieve the results now
		SDPA_LOG_DEBUG("User: retrieve the results of the job "<<job_id_user);
		RetrieveJobResultsEvent::Ptr pEvtRetrieveRes(new RetrieveJobResultsEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtRetrieveRes);
		// wait for a JobStatusReplyEvent
		pUserStr->WaitForEvent<sdpa::events::JobResultsReplyEvent>(pErrorEvt);

		// check the job status. if the job is in a final state, send a DeletJobEvent
		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		DeleteJobEvent::Ptr pEvtDelJob( new DeleteJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pEvtDelJob);

		// wait for an acknowledgment from Orchestrator that job was deleted
		sdpa::job_id_t jobid = pUserStr->WaitForEvent<sdpa::events::DeleteJobAckEvent>(pErrorEvt)->job_id();
		os.str("");
		os<<"Successfully deleted the job "<<jobid;
		SDPA_LOG_DEBUG(os.str());

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

void D2D2DDummyGwesTest::testDaemonFSM_JobFailed()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobFailed******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	answerStrategy = "failed";
	string strFromUser(sdpa::daemon::USER);
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

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrToUserStage);
	m_ptrToUserStage->start();

	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	UserStrategy* pUserStr = dynamic_cast<UserStrategy*>(m_ptrUserStrategy.get());

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		// check if the job finished
		SDPA_LOG_DEBUG("User: query "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
		QueryJobStatusEvent::Ptr pEvtQueryStatus(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtQueryStatus);

		// wait for a JobStatusReplyEvent
		JobStatusReplyEvent::Ptr pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());

		while( pJobStatusReplyEvent->status().find("Finished") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Failed") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Cancelled") == std::string::npos)
		{
			SDPA_LOG_DEBUG("User: ask the "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
			QueryJobStatusEvent::Ptr pEvtQueryStNew(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
			m_ptrOrch->daemon_stage()->send(pEvtQueryStNew);

			// wait for a JobStatusReplyEvent
			pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());
			usleep(user_user_sleep_interval);
		}

			// if the job is in the finished or failed state, one is allowed
		// to retriieve the results now
		SDPA_LOG_DEBUG("User: retrieve the results of the job "<<job_id_user);
		RetrieveJobResultsEvent::Ptr pEvtRetrieveRes(new RetrieveJobResultsEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtRetrieveRes);
		// wait for a JobStatusReplyEvent
		pUserStr->WaitForEvent<sdpa::events::JobResultsReplyEvent>(pErrorEvt);

		// check the job status. if the job is in a final state, send a DeletJobEvent
		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		DeleteJobEvent::Ptr pEvtDelJob( new DeleteJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pEvtDelJob);

		// wait for an acknowledgment from Orchestrator that job was deleted
		sdpa::job_id_t jobid = pUserStr->WaitForEvent<sdpa::events::DeleteJobAckEvent>(pErrorEvt)->job_id();
		os.str("");
		os<<"Successfully deleted the job "<<jobid;
		SDPA_LOG_DEBUG(os.str());

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

void D2D2DDummyGwesTest::testDaemonFSM_JobCancelled()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobCancelled******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	answerStrategy = "cancelled";
	string strFromUser(sdpa::daemon::USER);
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

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrToUserStage);
	m_ptrToUserStage->start();

	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	UserStrategy* pUserStr = dynamic_cast<UserStrategy*>(m_ptrUserStrategy.get());

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		sleep(1);
		// Now, the user sends a CancelJob  message
		CancelJobEvent::Ptr pCancelJobEvt( new CancelJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pCancelJobEvt);

		// the user expects now a CancelJobAckEvent
		CancelJobAckEvent::Ptr pCancelAckEvt = pUserStr->WaitForEvent<sdpa::events::CancelJobAckEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("USER: The job "<<pCancelAckEvt->job_id()<<" has been successfully cancelled!");

		// waits until the job was explicitely cancelled
		SDPA_LOG_DEBUG("User: query "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
		QueryJobStatusEvent::Ptr pEvtQueryStatus(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtQueryStatus);

		// wait for a JobStatusReplyEvent
		JobStatusReplyEvent::Ptr pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());

		while( pJobStatusReplyEvent->status().find("Finished") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Failed") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Cancelled") == std::string::npos)
		{
			SDPA_LOG_DEBUG("User: ask the "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
			QueryJobStatusEvent::Ptr pEvtQueryStNew(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
			m_ptrOrch->daemon_stage()->send(pEvtQueryStNew);

			// wait for a JobStatusReplyEvent
			pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());
			usleep(user_user_sleep_interval);
		}

			// if the job is in the finished or failed state, one is allowed
		// to retriieve the results now
		SDPA_LOG_DEBUG("User: retrieve the results of the job "<<job_id_user);
		RetrieveJobResultsEvent::Ptr pEvtRetrieveRes(new RetrieveJobResultsEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtRetrieveRes);
		// wait for a JobStatusReplyEvent
		pUserStr->WaitForEvent<sdpa::events::JobResultsReplyEvent>(pErrorEvt);

		// check the job status. if the job is in a final state, send a DeletJobEvent
		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		DeleteJobEvent::Ptr pEvtDelJob( new DeleteJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pEvtDelJob);

		// wait for an acknowledgment from Orchestrator that job was deleted
		sdpa::job_id_t jobid = pUserStr->WaitForEvent<sdpa::events::DeleteJobAckEvent>(pErrorEvt)->job_id();
		os.str("");
		os<<"Successfully deleted the job "<<jobid;
		SDPA_LOG_DEBUG(os.str());

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

void D2D2DDummyGwesTest::testDaemonFSM_JobCancelled_from_Pending()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobCancelled_from_Pending******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	answerStrategy = "cancelled";
	string strFromUser(sdpa::daemon::USER);
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

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrToUserStage);
	m_ptrToUserStage->start();

	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	UserStrategy* pUserStr = dynamic_cast<UserStrategy*>(m_ptrUserStrategy.get());

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		sleep(1);
		// Now, the user sends a CancelJob  message
		CancelJobEvent::Ptr pCancelJobEvt( new CancelJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pCancelJobEvt);

		// the user expects now a CancelJobAckEvent
		CancelJobAckEvent::Ptr pCancelAckEvt = pUserStr->WaitForEvent<sdpa::events::CancelJobAckEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("USER: The job "<<pCancelAckEvt->job_id()<<" has been successfully cancelled!");

		// waits until the job was explicitely cancelled
		SDPA_LOG_DEBUG("User: query "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
		QueryJobStatusEvent::Ptr pEvtQueryStatus(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtQueryStatus);

		// wait for a JobStatusReplyEvent
		JobStatusReplyEvent::Ptr pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());

		while( pJobStatusReplyEvent->status().find("Finished") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Failed") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Cancelled") == std::string::npos)
		{
			SDPA_LOG_DEBUG("User: ask the "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
			QueryJobStatusEvent::Ptr pEvtQueryStNew(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
			m_ptrOrch->daemon_stage()->send(pEvtQueryStNew);

			// wait for a JobStatusReplyEvent
			pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());
			usleep(user_user_sleep_interval);
		}

			// if the job is in the finished or failed state, one is allowed
		// to retriieve the results now
		SDPA_LOG_DEBUG("User: retrieve the results of the job "<<job_id_user);
		RetrieveJobResultsEvent::Ptr pEvtRetrieveRes(new RetrieveJobResultsEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtRetrieveRes);
		// wait for a JobStatusReplyEvent
		pUserStr->WaitForEvent<sdpa::events::JobResultsReplyEvent>(pErrorEvt);

		// check the job status. if the job is in a final state, send a DeletJobEvent
		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		DeleteJobEvent::Ptr pEvtDelJob( new DeleteJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pEvtDelJob);

		// wait for an acknowledgment from Orchestrator that job was deleted
		sdpa::job_id_t jobid = pUserStr->WaitForEvent<sdpa::events::DeleteJobAckEvent>(pErrorEvt)->job_id();
		os.str("");
		os<<"Successfully deleted the job "<<jobid;
		SDPA_LOG_DEBUG(os.str());

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
