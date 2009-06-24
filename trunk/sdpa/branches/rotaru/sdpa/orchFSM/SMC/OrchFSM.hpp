#ifndef SDPASMCFSM_HPP
#define SDPASMCFSM_HPP 1

#include <sdpa/events/ConfigNokEvent.hpp>
#include <sdpa/events/ConfigOkEvent.hpp>
#include <sdpa/events/ConfigRequestEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/InterruptEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/StartUpEvent.hpp>
#include <sdpa/events/SubmitAckEvent.hpp>

#include <sdpa/orchFSM/SMC/OrchFSM_sm.h>
#include <sdpa/logging.hpp>

#include <list>

namespace sdpa {
	namespace fsm {
		class OrchFSM {
			public:
				typedef std::tr1::shared_ptr<OrchFSM> Ptr;

				OrchFSM();
				virtual ~OrchFSM();

				void action_configure(sdpa::events::StartUpEvent&);
				void action_config_ok(sdpa::events::ConfigOkEvent&);
				void action_config_nok(sdpa::events::ConfigNokEvent&);
				void action_interrupt(sdpa::events::InterruptEvent& );
				void action_lifesign(sdpa::events::LifeSignEvent& );
				void action_delete_job(sdpa::events::DeleteJobEvent& );
				void action_request_job(sdpa::events::RequestJobEvent& );
				void action_request_job(sdpa::events::SubmitAckEvent& );
				void action_config_request(sdpa::events::ConfigRequestEvent& );

				sdpa::fsm::OrchFSMContext& GetContext() { return m_fsmContext; }
			//private:
				SDPA_DECLARE_LOGGER();
				sdpa::fsm::OrchFSMContext m_fsmContext;
		};
	}
}

#endif
