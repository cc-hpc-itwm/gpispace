/*
 * =====================================================================================
 *
 *       Filename:  EventHandler.hpp
 *
 *    Description:  EventHandler
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Tiberiu Rotaru
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_EVENT_HANDLER_HPP
#define SDPA_EVENT_HANDLER_HPP 1

#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa {
	namespace events {

	  class CancelJobAckEvent;
	  class CancelJobEvent;
	  class ConfigNokEvent;
	  class ConfigOkEvent;
	  class ConfigReplyEvent;
	  class ConfigRequestEvent;
	  class DeleteJobAckEvent;
	  class DeleteJobEvent;
	  class ErrorEvent;
	  class InterruptEvent;
	  class JobFailedAckEvent;
	  class JobFailedEvent;
	  class JobFinishedAckEvent;
	  class JobFinishedEvent;
	  class JobResultsReplyEvent;
	  class JobStatusReplyEvent;
	  class LifeSignEvent;
	  class QueryJobStatusEvent;
	  class RequestJobEvent;
	  class RetrieveJobResultsEvent;
	  class RunJobEvent;
	  class StartUpEvent;
	  class SubmitJobAckEvent;
	  class SubmitJobEvent;
	  class WorkerRegistrationAckEvent;
	  class WorkerRegistrationEvent;
	  class CapabilitiesGainedEvent;
	  class CapabilitiesLostEvent;

	  class EventHandler
	  {
	  public:
		virtual ~EventHandler() {}

		virtual void handleCancelJobAckEvent(const CancelJobAckEvent *) {}
		virtual void handleCancelJobEvent(const CancelJobEvent *) {}
		virtual void handleConfigNokEvent(const ConfigNokEvent *) {}
		virtual void handleConfigOkEvent(const ConfigOkEvent *) {}
		virtual void handleConfigReplyEvent(const ConfigReplyEvent *) {}
		virtual void handleConfigRequestEvent(const ConfigRequestEvent *) {}
		virtual void handleDeleteJobAckEvent(const DeleteJobAckEvent *) {}
		virtual void handleDeleteJobEvent(const DeleteJobEvent *) {}
		virtual void handleErrorEvent(const ErrorEvent *) {}
		virtual void handleInterruptEvent(const InterruptEvent *){}
		virtual void handleJobFailedAckEvent(const JobFailedAckEvent *){}
		virtual void handleJobFailedEvent(const JobFailedEvent *) {}
		virtual void handleJobFinishedAckEvent(const JobFinishedAckEvent *) {}
		virtual void handleJobFinishedEvent(const JobFinishedEvent *) {}
		virtual void handleJobResultsReplyEvent(const JobResultsReplyEvent *) {}
		virtual void handleJobStatusReplyEvent(const JobStatusReplyEvent *) {}
		virtual void handleLifeSignEvent(const LifeSignEvent *) {}
		virtual void handleQueryJobStatusEvent(const QueryJobStatusEvent *) {}
		virtual void handleRequestJobEvent(const RequestJobEvent *) {}
		virtual void handleRetrieveJobResultsEvent(const RetrieveJobResultsEvent *) {}
		virtual void handleRunJobEvent(const RunJobEvent *) {}
		virtual void handleStartUpEvent(const StartUpEvent *) {}
		virtual void handleSubmitJobAckEvent(const SubmitJobAckEvent *) {}
		virtual void handleSubmitJobEvent(const SubmitJobEvent *) {}
		virtual void handleWorkerRegistrationAckEvent(const WorkerRegistrationAckEvent *) {}
		virtual void handleWorkerRegistrationEvent(const WorkerRegistrationEvent *) {}
		virtual void handleCapabilitiesGainedEvent(const CapabilitiesGainedEvent*) {}
		virtual void handleCapabilitiesLostEvent(const CapabilitiesLostEvent*) {}
	  };
	}
}

#endif
