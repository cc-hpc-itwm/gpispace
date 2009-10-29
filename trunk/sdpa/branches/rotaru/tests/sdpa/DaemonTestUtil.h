#ifndef DAEMONTESTUTIL_H_
#define DAEMONTESTUTIL_H_
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


class SchedulerNRE : public SchedulerImpl
{
public:
	static string answerStrategy;

	SchedulerNRE(sdpa::Sdpa2Gwes* ptr_Sdpa2Gwes, sdpa::daemon::IComm* pHandler, std::string& answerStrategy):
		SchedulerImpl(ptr_Sdpa2Gwes,  pHandler),
		m_answerStrategy(answerStrategy)
		{}

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
				if( m_answerStrategy == "finished" )
				{
					SDPA_LOG_DEBUG("Slave: send JobFinishedEvent to "<<ptr_comm_handler_->master());
					JobFinishedEvent::Ptr pJobFinEvt( new JobFinishedEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master(), pJob->id() ) );
					ptr_comm_handler_->sendEvent(ptr_comm_handler_->to_master_stage(), pJobFinEvt);
				}
				else if( m_answerStrategy == "failed" )
				{
					SDPA_LOG_DEBUG("Slave: send JobFailedEvent to "<<ptr_comm_handler_->master());
					JobFailedEvent::Ptr pJobFailEvt( new JobFailedEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master(), pJob->id() ) );
					ptr_comm_handler_->sendEvent(ptr_comm_handler_->to_master_stage(), pJobFailEvt);
				}
				else if( m_answerStrategy == "cancelled" )
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

private:
	 std::string m_answerStrategy;
};

class NreDaemon : public DaemonFSM
{
public:
	SDPA_DECLARE_LOGGER();
	NreDaemon(	const std::string &name,
				seda::Stage* ptrToMasterStage,
				seda::Stage* ptrToSlaveStage,
				sdpa::Sdpa2Gwes*  pArgSdpa2Gwes,
				std::string& answerStrategy)
	: DaemonFSM( name, ptrToMasterStage, ptrToSlaveStage, pArgSdpa2Gwes),
	  SDPA_INIT_LOGGER(name)
	{
		ptr_scheduler_ =  Scheduler::ptr_t(new SchedulerNRE(pArgSdpa2Gwes, this, answerStrategy));
	}

	NreDaemon(  const std::string &name,
								sdpa::Sdpa2Gwes*  pArgSdpa2Gwes,
								const std::string& toMasterStageName,
								const std::string& toSlaveStageName,
								std::string& answerStrategy)
	: DaemonFSM( name, pArgSdpa2Gwes, toMasterStageName, toSlaveStageName ),
	  SDPA_INIT_LOGGER(name)
	{
		ptr_scheduler_ =  Scheduler::ptr_t(new SchedulerNRE(pArgSdpa2Gwes, this, answerStrategy));
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


#endif /* DAEMONTESTUTIL_H_ */
