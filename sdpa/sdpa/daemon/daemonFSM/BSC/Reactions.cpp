/*
 * =====================================================================================
 *
 *       Filename:  Reactions.cpp
 *
 *    Description:  Daemon state chart reactions (boost)
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
#include "boost/cast.hpp"
#include <iostream>
#include <string>
#include <stdexcept>

using namespace sdpa::events;
using namespace sdpa::daemon;
using namespace sdpa::fsm::bsc;


DaemonFSM::DaemonFSM( const std::string &name,
                      const std::vector<std::string>& arrMasterNames = std::vector<std::string>(),
                      unsigned int cap,
                      IWorkflowEngine*  pArgSdpa2Gwes)
  : GenericDaemon(name, arrMasterNames, cap, pArgSdpa2Gwes),
    SDPA_INIT_LOGGER(name+"FSM")
{
  initiate();
  SDPA_LOG_DEBUG("Daemon state machine initialized ...");
}

DaemonFSM::~DaemonFSM()
{
  terminate();
  SDPA_LOG_DEBUG("Daemon state machine destroyed ...");
}

void DaemonFSM :: print_states()
{
  for( state_iterator it = state_begin(); it != state_end(); it++ )
    std::cout<<"State "<<typeid(*it).name()<<std::endl;
}

void DaemonFSM::handleStartUpEvent(const StartUpEvent* pEvent)
{
    lock_type lock(mtx_);
    //SDPA_LOG_DEBUG("Process StartUpEvent");
    process_event(*pEvent);

    if( m_bConfigOk )
    {
        SDPA_LOG_INFO("Starting the scheduler...");
        //sdpa::daemon::Scheduler::ptr_t ptrSched(this->create_scheduler());
        //ptr_scheduler_ = ptrSched;
        ptr_scheduler_->start(this);

        // start the network stage
        ptr_to_master_stage_->start();

        m_bRequestsAllowed = true;

        // if the configuration step was ok send a ConfigOkEvent
        ConfigOkEvent::Ptr pEvtConfigOk( new ConfigOkEvent(name(), name()));
        sendEventToSelf(pEvtConfigOk);
    }
    else //if not
    {
        m_bRequestsAllowed = false;

        // if the configuration step was ok send a ConfigOkEvent
        ConfigNokEvent::Ptr pEvtConfigNok( new ConfigNokEvent(name(), name()));
        sendEventToSelf(pEvtConfigNok);
    }
}

void DaemonFSM::handleConfigOkEvent(const ConfigOkEvent* pEvent)
{
	//SDPA_LOG_DEBUG("Process ConfigOkEvent");
	{
		lock_type lock(mtx_);
		process_event(*pEvent);
		m_bStarted = true;
	}

	cond_can_start_.notify_one();
}

void DaemonFSM::handleConfigNokEvent(const ConfigNokEvent* pEvent)
{
	//SDPA_LOG_DEBUG("Process ConfigNokEvent");
	{
		lock_type lock(mtx_);
		process_event(*pEvent);

		m_bStarted = true;
		m_bConfigOk = false;
		m_bStopped = true;
	}

	cond_can_start_.notify_one();
}

void DaemonFSM::handleInterruptEvent(const InterruptEvent* pEvent)
{
	//SDPA_LOG_DEBUG("Process InterruptEvent");
	{
		lock_type lock(mtx_);
		process_event(*pEvent);
		m_bStopped = true;
	}

	cond_can_stop_.notify_one();
}

void DaemonFSM::handleWorkerRegistrationEvent(const WorkerRegistrationEvent* pEvent)
{
	lock_type lock(mtx_);
	//SDPA_LOG_DEBUG("Process WorkerRegistrationEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleDeleteJobEvent(const DeleteJobEvent* pEvent)
{
	lock_type lock(mtx_);
	//SDPA_LOG_DEBUG("Process DeleteJobEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleSubmitJobEvent(const SubmitJobEvent* pEvent)
{
	lock_type lock(mtx_);
	//SDPA_LOG_DEBUG("Process SubmitJobEvent");
	process_event(*pEvent);
}

/*
void DaemonFSM::handleLifeSignEvent(const LifeSignEvent* pEvent)
{
	lock_type lock(mtx_);
	//SDPA_LOG_DEBUG("Process LifeSignEvent");
	process_event(*pEvent);
}*/

void DaemonFSM::handleRequestJobEvent(const RequestJobEvent* pEvent)
{
	lock_type lock(mtx_);
	//SDPA_LOG_DEBUG("Process RequestJobEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleConfigRequestEvent(const ConfigRequestEvent* pEvent)
{
	lock_type lock(mtx_);
	//SDPA_LOG_DEBUG("Process ConfigRequestEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleErrorEvent(const ErrorEvent* pEvent)
{
	lock_type lock(mtx_);
	//SDPA_LOG_DEBUG("Process ErrorEvent");
	process_event(*pEvent);
}


sc::result Down::react(const StartUpEvent& e)
{
	return transit<Configuring>(&DaemonFSM::action_configure, e); // successfully configured and all services started-up
}

sc::result Down::react(const ErrorEvent& e)
{
	return transit<Down>(&DaemonFSM::action_error_event, e); // successfully configured and all services started-up
}

sc::result Down::react(const sc::exception_thrown &)
{
	try
	{
	  throw;
	}
	catch ( ... )
	{
	  // ... all other exceptions are forwarded to our outer
	  // state(s). The state machine is terminated and the
	  // exception rethrown if the outer state(s) can't
	  // React to  it either...
	  return forward_event();
	}
}

sc::result Configuring::react( const ConfigOkEvent& e)
{
	//SDPA_LOG_DEBUG("React to  ConfigOkEvent");
	return transit<Up>(&DaemonFSM::action_config_ok, e);
}

sc::result Configuring::react( const ConfigNokEvent& e)
{
	//SDPA_LOG_DEBUG("React to  ConfigNokEvent");
	return transit<Down>(&DaemonFSM::action_config_nok, e);
}

sc::result Configuring::react(const ErrorEvent& e)
{
	return transit<Configuring>(&DaemonFSM::action_error_event, e); // successfully configured and all services started-up
}

sc::result Configuring::react(const sc::exception_thrown &)
{
	try
	{
	  throw;
	}
	catch ( ... )
	{
	  // ... all other exceptions are forwarded to our outer
	  // state(s). The state machine is terminated and the
	  // exception rethrown if the outer state(s) can't
	  // React to  it either...
	  return forward_event();
	}
}

sc::result Up::react(const InterruptEvent& e)
{
	////SDPA_LOG_DEBUG("React to  InterruptEvent");
	return transit<Down>(&DaemonFSM::action_interrupt, e);
}

/*sc::result Up::react(const LifeSignEvent& e )
{
	////SDPA_LOG_DEBUG("React to  LifeSignEvent");
	return transit<Up>(&DaemonFSM::action_lifesign, e);
}*/

sc::result Up::react(const DeleteJobEvent& e)
{
	////SDPA_LOG_DEBUG("React to  DeleteJobEvent");
	return transit<Up>(&DaemonFSM::action_delete_job, e);
}

sc::result Up::react(const RequestJobEvent& e)
{
	////SDPA_LOG_DEBUG("React to  RequestJobEvent");
	return transit<Up>(&DaemonFSM::action_request_job, e);
}

sc::result Up::react(const SubmitJobEvent& e )
{
	////SDPA_LOG_DEBUG("React to  SubmitJobEvent");
   	return transit<Up>(&DaemonFSM::action_submit_job, e);
}

sc::result Up::react(const ConfigRequestEvent& e )
{
	////SDPA_LOG_DEBUG("React to  ConfigRequestEvent");
   	return transit<Up>(&DaemonFSM::action_config_request, e);
}

sc::result Up::react( const sdpa::events::WorkerRegistrationEvent& e)
{
	//SDPA_LOG_DEBUG("React to  WorkerRegistrationEvent");
	return transit<Up>(&DaemonFSM::action_register_worker, e);
}

sc::result Up::react(const ErrorEvent& e)
{
	return transit<Up>(&DaemonFSM::action_error_event, e); // successfully configured and all services started-up
}

sc::result Up::react(const sc::exception_thrown &)
{
	try
	{
	  throw;
	}
	catch ( ... )
	{
	  // ... all other exceptions are forwarded to our outer
	  // state(s). The state machine is terminated and the
	  // exception re-thrown if the outer state(s) can't
	  // React to  it either...
	  return forward_event();
	}
}
