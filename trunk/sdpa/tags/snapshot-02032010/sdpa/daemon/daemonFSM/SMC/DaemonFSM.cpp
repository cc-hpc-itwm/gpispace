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

void DaemonFSM::handleDaemonEvent(const seda::IEvent::Ptr& pEvent)
{
	lock_type lock(mtx_);

	if( StartUpEvent* ptr = dynamic_cast<StartUpEvent*>(pEvent.get()) )
		GetContext().StartUp(*ptr);
	else if( ConfigOkEvent* ptr = dynamic_cast<ConfigOkEvent*>(pEvent.get()) )
		GetContext().ConfigOk(*ptr);
	else if( ConfigNokEvent* ptr = dynamic_cast<ConfigNokEvent*>(pEvent.get()) )
		GetContext().ConfigNok(*ptr);
	else if( InterruptEvent* ptr = dynamic_cast<InterruptEvent*>(pEvent.get()) )
		GetContext().Interrupt(*ptr);
	else if( WorkerRegistrationEvent* ptr = dynamic_cast<WorkerRegistrationEvent*>(pEvent.get()) )
		GetContext().RegisterWorker(*ptr);
	else if( DeleteJobEvent* ptr = dynamic_cast<DeleteJobEvent*>(pEvent.get()) )
		GetContext().DeleteJob(*ptr);
	else if( SubmitJobEvent* ptr = dynamic_cast<SubmitJobEvent*>(pEvent.get()) )
		GetContext().SubmitJob(*ptr);
	else if( LifeSignEvent* ptr = dynamic_cast<LifeSignEvent*>(pEvent.get()) )
		GetContext().LifeSign(*ptr);
	else if( RequestJobEvent* ptr = dynamic_cast<RequestJobEvent*>(pEvent.get()) )
		GetContext().RequestJob(*ptr);
	else if( ConfigRequestEvent* ptr = dynamic_cast<ConfigRequestEvent*>(pEvent.get()) )
		GetContext().ConfigRequest(*ptr);
	else if( ErrorEvent *ptr = dynamic_cast<ErrorEvent*>(pEvent.get()) )
		GetContext().Error(*ptr);
}
