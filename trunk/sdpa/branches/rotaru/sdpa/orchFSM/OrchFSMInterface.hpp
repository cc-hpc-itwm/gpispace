#ifndef ORCHFSMITF_HPP
#define ORCHFSMITF_HPP 1

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/events/ConfigNokEvent.hpp>
#include <sdpa/events/ConfigOkEvent.hpp>
#include <sdpa/events/ConfigRequestEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/InterruptEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/StartUpEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>

namespace sdpa {
	namespace fsm {
		class OrchFSMInterface {
			public:
				virtual void action_configure(const sdpa::events::StartUpEvent&);
				virtual void action_config_ok(const sdpa::events::ConfigOkEvent&);
				virtual void action_config_nok(const sdpa::events::ConfigNokEvent&);
				virtual void action_interrupt(const sdpa::events::InterruptEvent& );
				virtual void action_lifesign(const sdpa::events::LifeSignEvent& );
				virtual void action_delete_job(const sdpa::events::DeleteJobEvent& );
				virtual void action_request_job(const sdpa::events::RequestJobEvent& );
				virtual void action_submit_job(const sdpa::events::SubmitJobEvent& );
				virtual void action_submit_job_ack(const sdpa::events::SubmitJobAckEvent& );
				virtual void action_config_request(const sdpa::events::ConfigRequestEvent& );
		};
	}
}

#endif
