#ifndef SDPA_DAEMON_GENERIC_DAEMON_ACTIONS
#define SDPA_DAEMON_GENERIC_DAEMON_ACTIONS 1

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/events/ConfigNokEvent.hpp>
#include <sdpa/events/ConfigOkEvent.hpp>
#include <sdpa/events/ConfigRequestEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/InterruptEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/StartUpEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/JobFailedAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>


namespace sdpa { namespace daemon {
	  class GenericDaemonActions {
	  public:
			virtual void action_configure(const sdpa::events::StartUpEvent&)=0;
			virtual void action_config_ok(const sdpa::events::ConfigOkEvent&)=0;
			virtual void action_config_nok(const sdpa::events::ConfigNokEvent&)=0;
			virtual void action_interrupt(const sdpa::events::InterruptEvent& )=0;
			virtual void action_lifesign(const sdpa::events::LifeSignEvent& )=0;
			virtual void action_delete_job(const sdpa::events::DeleteJobEvent& )=0;
			virtual void action_request_job(const sdpa::events::RequestJobEvent& )=0;
			virtual void action_submit_job(const sdpa::events::SubmitJobEvent& )=0;
			virtual void action_config_request(const sdpa::events::ConfigRequestEvent& )=0;
			virtual void action_register_worker(const sdpa::events::WorkerRegistrationEvent& )=0;
  };
}}

#endif
