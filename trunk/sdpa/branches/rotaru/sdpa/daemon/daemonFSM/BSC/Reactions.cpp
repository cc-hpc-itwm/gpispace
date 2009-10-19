#include "DaemonFSM.hpp"
#include "boost/cast.hpp"
#include <iostream>
#include <string>
#include <stdexcept>

using namespace sdpa::events;
using namespace sdpa::daemon;
using namespace sdpa::fsm::bsc;

DaemonFSM ::DaemonFSM(const std::string &name, seda::Stage* ptrOutStage, sdpa::Sdpa2Gwes* ptrGwes)
	: GenericDaemon(name, ptrOutStage, ptrGwes),
	  SDPA_INIT_LOGGER("sdpa.fsm.bsc.DaemonFSM")
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

void DaemonFSM::handleDaemonEvent(const seda::IEvent::Ptr& pEvent)
{
	if( StartUpEvent* ptr = dynamic_cast<StartUpEvent*>(pEvent.get()) ){
		SDPA_LOG_DEBUG("Process StartUpEvent");
		SDPA_LOG_DEBUG("Call proces_event(e) ...");
		process_event(*ptr);
	}
	else if( ConfigOkEvent* ptr = dynamic_cast<ConfigOkEvent*>(pEvent.get()) ){
		SDPA_LOG_DEBUG("Process ConfigOkEvent");
		process_event(*ptr);
	}
	else if( ConfigNokEvent* ptr = dynamic_cast<ConfigNokEvent*>(pEvent.get()) ) {
		SDPA_LOG_DEBUG("Process ConfigOkEvent");
		process_event(*ptr);
	}
	else if( InterruptEvent* ptr = dynamic_cast<InterruptEvent*>(pEvent.get()) ) {
		SDPA_LOG_DEBUG("Process InterruptEvent");
		process_event(*ptr);
	}
	else if( WorkerRegistrationEvent* ptr = dynamic_cast<WorkerRegistrationEvent*>(pEvent.get()) ) {
		SDPA_LOG_DEBUG("Process WorkerRegistrationEvent");
		process_event(*ptr);
	}
	else if( DeleteJobEvent* ptr = dynamic_cast<DeleteJobEvent*>(pEvent.get()) ) {
		SDPA_LOG_DEBUG("Process DeleteJobEvent");
		process_event(*ptr);
	}
	else if( SubmitJobEvent* ptr = dynamic_cast<SubmitJobEvent*>(pEvent.get()) ) {
		SDPA_LOG_DEBUG("Process SubmitJobEvent");
		process_event(*ptr);
	}
	else if( LifeSignEvent* ptr = dynamic_cast<LifeSignEvent*>(pEvent.get()) ) {
		SDPA_LOG_DEBUG("Process LifeSignEvent");
		process_event(*ptr);
	}
	else if( RequestJobEvent* ptr = dynamic_cast<RequestJobEvent*>(pEvent.get()) ){
		SDPA_LOG_DEBUG("Process RequestJobEvent");
		process_event(*ptr);
	}
	else if( ConfigRequestEvent* ptr = dynamic_cast<ConfigRequestEvent*>(pEvent.get()) ){
		SDPA_LOG_DEBUG("Process ConfigRequestEvent");
		process_event(*ptr);
	}
}


sc::result Down::react(const StartUpEvent& e)
{
	return transit<Configurring>(&DaemonFSM::action_configure, e); // successfully configured and all services started-up
}

sc::result Down::react(const sc::exception_thrown & e)
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


sc::result Configurring::react( const ConfigOkEvent& e)
{
	SDPA_LOG_DEBUG("React to  ConfigOkEvent");
	return transit<Up>(&DaemonFSM::action_config_ok, e);
}

sc::result Configurring::react( const ConfigNokEvent& e)
{
	SDPA_LOG_DEBUG("React to  ConfigNokEvent");
	return transit<Down>(&DaemonFSM::action_config_nok, e);
}

sc::result Configurring::react(const sc::exception_thrown & e)
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

sc::result Up::react(const sc::exception_thrown & e)
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
