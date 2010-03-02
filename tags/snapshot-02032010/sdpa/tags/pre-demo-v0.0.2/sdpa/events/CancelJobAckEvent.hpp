#ifndef SDPA_CANCELJOBACKEVENT_HPP
#define SDPA_CANCELJOBACKEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventVisitor.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
	class CancelJobAckEvent : public JobEvent, public sc::event<CancelJobAckEvent>
#else
	class CancelJobAckEvent : public JobEvent
#endif
    {
	public:
		typedef sdpa::shared_ptr<CancelJobAckEvent> Ptr;

        CancelJobAckEvent()
          : JobEvent("", "", "")
        {}

		CancelJobAckEvent(const address_t &a_from, const address_t &a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( a_from, a_to, a_job_id ) {
		}

		virtual ~CancelJobAckEvent() { }

		std::string str() const { return "CancelJobAckEvent"; }

        virtual void accept(EventVisitor *visitor)
        {
          visitor->visitCancelJobAckEvent(this);
        }
	};
}}

#endif
