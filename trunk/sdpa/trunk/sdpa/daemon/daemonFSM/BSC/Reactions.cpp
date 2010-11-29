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

DaemonFSM::DaemonFSM(	const std::string &name,
		seda::Stage* ptrToMasterStage,
		seda::Stage* ptrToSlaveStage,
		IWorkflowEngine*  pArgSdpa2Gwes)
	: GenericDaemon(name, ptrToMasterStage, ptrToSlaveStage, pArgSdpa2Gwes),
	  SDPA_INIT_LOGGER("sdpa.fsm.bsc.DaemonFSM")
{
	initiate();
	SDPA_LOG_DEBUG("Daemon state machine initialized ...");
}

DaemonFSM::DaemonFSM(  const std::string &name,
						IWorkflowEngine*  pArgSdpa2Gwes,
						const std::string& toMasterStageName,
						const std::string& toSlaveStageName )
				: GenericDaemon(name, toMasterStageName, toSlaveStageName, pArgSdpa2Gwes),
				  SDPA_INIT_LOGGER(name+"FSM")
{
	initiate();
	SDPA_LOG_DEBUG("Daemon state machine initialized ...");
}

DaemonFSM::DaemonFSM( const std::string &name, IWorkflowEngine*  pArgSdpa2Gwes)
	: GenericDaemon(name, pArgSdpa2Gwes),
	  SDPA_INIT_LOGGER(name+"FSM")
{
	initiate();
	SDPA_LOG_DEBUG("Daemon state machine initialized ...");
}

DaemonFSM::~DaemonFSM() {
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
	SDPA_LOG_DEBUG("Process StartUpEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleConfigOkEvent(const ConfigOkEvent* pEvent)
{
	lock_type lock(mtx_);
	SDPA_LOG_DEBUG("Process ConfigOkEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleConfigNokEvent(const ConfigNokEvent* pEvent)
{
	lock_type lock(mtx_);
	SDPA_LOG_DEBUG("Process ConfigNokEvent");
	process_event(*pEvent);
}
void DaemonFSM::handleInterruptEvent(const InterruptEvent* pEvent)
{
	lock_type lock(mtx_);
	SDPA_LOG_DEBUG("Process InterruptEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleWorkerRegistrationEvent(const WorkerRegistrationEvent* pEvent)
{
	lock_type lock(mtx_);
	SDPA_LOG_DEBUG("Process WorkerRegistrationEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleDeleteJobEvent(const DeleteJobEvent* pEvent)
{
	lock_type lock(mtx_);
	SDPA_LOG_DEBUG("Process DeleteJobEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleSubmitJobEvent(const SubmitJobEvent* pEvent)
{
	lock_type lock(mtx_);
	SDPA_LOG_DEBUG("Process SubmitJobEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleLifeSignEvent(const LifeSignEvent* pEvent)
{
	lock_type lock(mtx_);
	SDPA_LOG_DEBUG("Process LifeSignEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleRequestJobEvent(const RequestJobEvent* pEvent)
{
	lock_type lock(mtx_);
	SDPA_LOG_DEBUG("Process RequestJobEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleConfigRequestEvent(const ConfigRequestEvent* pEvent)
{
	lock_type lock(mtx_);
	SDPA_LOG_DEBUG("Process ConfigRequestEvent");
	process_event(*pEvent);
}

void DaemonFSM::handleErrorEvent(const ErrorEvent* pEvent)
{
	lock_type lock(mtx_);
	SDPA_LOG_DEBUG("Process ErrorEvent");
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
	SDPA_LOG_DEBUG("React to  ConfigOkEvent");
	return transit<Up>(&DaemonFSM::action_config_ok, e);
}

sc::result Configuring::react( const ConfigNokEvent& e)
{
	SDPA_LOG_DEBUG("React to  ConfigNokEvent");
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
	SDPA_LOG_DEBUG("React to  InterruptEvent");
	return transit<Down>(&DaemonFSM::action_interrupt, e);
}

sc::result Up::react(const LifeSignEvent& e )
{
	SDPA_LOG_DEBUG("React to  LifeSignEvent");
	return transit<Up>(&DaemonFSM::action_lifesign, e);
}

sc::result Up::react(const DeleteJobEvent& e)
{
	SDPA_LOG_DEBUG("React to  DeleteJobEvent");
	return transit<Up>(&DaemonFSM::action_delete_job, e);
}

sc::result Up::react(const RequestJobEvent& e)
{
	SDPA_LOG_DEBUG("React to  RequestJobEvent");
	return transit<Up>(&DaemonFSM::action_request_job, e);
}

sc::result Up::react(const SubmitJobEvent& e )
{
	SDPA_LOG_DEBUG("React to  SubmitJobEvent");
   	return transit<Up>(&DaemonFSM::action_submit_job, e);
}

sc::result Up::react(const ConfigRequestEvent& e )
{
	SDPA_LOG_DEBUG("React to  ConfigRequestEvent");
   	return transit<Up>(&DaemonFSM::action_config_request, e);
}

sc::result Up::react( const sdpa::events::WorkerRegistrationEvent& e)
{
	SDPA_LOG_DEBUG("React to  WorkerRegistrationEvent");
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
