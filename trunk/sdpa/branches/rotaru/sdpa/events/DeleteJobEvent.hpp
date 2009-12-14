#ifndef SDPA_DELETE_JOB_EVENT_HPP
#define SDPA_DELETE_JOB_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
	class DeleteJobEvent : public JobEvent, public sc::event<DeleteJobEvent>
#else
	class DeleteJobEvent : public JobEvent
#endif
    {
	public:
		typedef sdpa::shared_ptr<DeleteJobEvent> Ptr;

        DeleteJobEvent()
          : JobEvent("","","")
        {}

		DeleteJobEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id)
          :  sdpa::events::JobEvent( from, to, job_id ) {
		}

		virtual ~DeleteJobEvent() {
		}

		std::string str() const { return "DeleteJobEvent"; }

        virtual void accept(EventVisitor *visitor)
        {
          visitor->visitDeleteJobEvent(this);
        }
	};
}}

#endif
