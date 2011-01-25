/*
 * =====================================================================================
 *
 *       Filename:  DaemonFSM.cpp
 *
 *    Description:  Daemon state machine (state machine compiler)
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
#include "DaemonFSM.hpp"

using namespace sdpa::fsm::smc;
using namespace sdpa::events;

void DaemonFSM::handleStartUpEvent(const StartUpEvent* pEvent)
{
	lock_type lock(mtx_);
	GetContext().StartUp(*pEvent);

	//SDPA_LOG_INFO("Agent "<<name()<<"'s current state is: "<<getStatus());

	if( m_bConfigOk )
	{
		SDPA_LOG_INFO("Starting the scheduler...");
		//sdpa::daemon::Scheduler::ptr_t ptrSched(this->create_scheduler());
		//ptr_scheduler_ = ptrSched;
		//dynamic_cast<sdpa::daemon::SchedulerImpl*>(ptr_scheduler_.get())->ptr_comm_handler_ = this;
		ptr_scheduler_->start(this);

		// start the network stage
		ptr_to_master_stage_->start();

		m_bRequestsAllowed = true;

		// if the configuration step was ok send a ConfigOkEvent
		SDPA_LOG_INFO( "Sending to self a ConfigOkEvent!");
		ConfigOkEvent::Ptr pEvtConfigOk( new ConfigOkEvent(name(), name()));
		sendEventToSelf(pEvtConfigOk);
	}
	else //if not
	{
		m_bRequestsAllowed = false;

		// if the configuration step was ok send a ConfigOkEvent
		SDPA_LOG_INFO( "Sending to self a ConfigNokEvent!");
		ConfigNokEvent::Ptr pEvtConfigNok( new ConfigNokEvent(name(), name()));
		sendEventToSelf(pEvtConfigNok);
	}
}

void DaemonFSM::handleConfigOkEvent(const ConfigOkEvent* pEvent)
{
	{
		lock_type lock(mtx_);
		GetContext().ConfigOk(*pEvent);
		m_bStarted = true;
	}

	//SDPA_LOG_INFO("Agent "<<name()<<"'s current state is: "<<getStatus());
	cond_can_start_.notify_one();

}

void DaemonFSM::handleConfigNokEvent(const ConfigNokEvent* pEvent)
{
	{
		lock_type lock(mtx_);
		GetContext().ConfigNok(*pEvent);
		m_bStarted = true;
		m_bConfigOk = false;
		m_bStopped = true;
	}

	//SDPA_LOG_INFO("Agent "<<name()<<"'s current state is: "<<getStatus());
	cond_can_start_.notify_one();
}


void DaemonFSM::handleInterruptEvent(const InterruptEvent* pEvent)
{
	{
		lock_type lock(mtx_);
		GetContext().Interrupt(*pEvent);
		m_bStopped = true;
	}

	cond_can_stop_.notify_one();
}

void DaemonFSM::handleWorkerRegistrationEvent(const WorkerRegistrationEvent* pEvent)
{
	lock_type lock(mtx_);
	GetContext().RegisterWorker(*pEvent);
}

void DaemonFSM::handleDeleteJobEvent(const DeleteJobEvent* pEvent)
{
	lock_type lock(mtx_);
	GetContext().DeleteJob(*pEvent);
}

void DaemonFSM::handleSubmitJobEvent(const SubmitJobEvent* pEvent)
{
	lock_type lock(mtx_);
	GetContext().SubmitJob(*pEvent);
}

void DaemonFSM::handleRequestJobEvent(const RequestJobEvent* pEvent)
{
	lock_type lock(mtx_);
	//SDPA_LOG_INFO("Agent "<<name()<<"'s current state is: "<<getStatus());
	GetContext().RequestJob(*pEvent);
}

void DaemonFSM::handleConfigRequestEvent(const ConfigRequestEvent* pEvent)
{
	lock_type lock(mtx_);
	GetContext().ConfigRequest(*pEvent);
}

void DaemonFSM::handleErrorEvent(const ErrorEvent* pEvent)
{
	lock_type lock(mtx_);
	GetContext().Error(*pEvent);
}

sdpa::status_t DaemonFSM ::getStatus()
{
	lock_type lock(mtx_);
	std::string strStatus("UNDEFINED_STATE");

	try {
		return m_fsmContext.getState().getName();
	}
	catch( const statemap::StateUndefinedException& ex )
	{
		LOG(TRACE, "Oh, oh, UNDEFINED_STATE!!!!!");
		return strStatus;
	}
}
