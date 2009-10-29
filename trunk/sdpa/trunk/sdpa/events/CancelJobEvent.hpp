#ifndef SDPA_CANCELJOBEVENT_HPP
#define SDPA_CANCELJOBEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
	class CancelJobEvent : public JobEvent, public sc::event<CancelJobEvent>
#else
	class CancelJobEvent : public JobEvent
#endif
    {
	public:
		typedef sdpa::shared_ptr<CancelJobEvent> Ptr;

        CancelJobEvent()
          : JobEvent("", "", "")
        {}

		CancelJobEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( a_from, a_to, a_job_id ) {
		}

		virtual ~CancelJobEvent() { }

		std::string str() const { return "CancelJobEvent"; }

        virtual void accept(EventVisitor *visitor)
        {
          visitor->visitCancelJobEvent(this);
        }
	};
}}

#endif
