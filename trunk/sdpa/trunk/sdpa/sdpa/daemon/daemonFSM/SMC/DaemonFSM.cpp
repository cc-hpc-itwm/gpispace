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
}

void DaemonFSM::handleConfigOkEvent(const ConfigOkEvent* pEvent)
{
	lock_type lock(mtx_);
	GetContext().ConfigOk(*pEvent);
}

void DaemonFSM::handleConfigNokEvent(const ConfigNokEvent* pEvent)
{
	lock_type lock(mtx_);
	GetContext().ConfigNok(*pEvent);
}
void DaemonFSM::handleInterruptEvent(const InterruptEvent* pEvent)
{
	lock_type lock(mtx_);
	GetContext().Interrupt(*pEvent);
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

void DaemonFSM::handleLifeSignEvent(const LifeSignEvent* pEvent)
{
	lock_type lock(mtx_);
	GetContext().LifeSign(*pEvent);
}

void DaemonFSM::handleRequestJobEvent(const RequestJobEvent* pEvent)
{
	lock_type lock(mtx_);
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

