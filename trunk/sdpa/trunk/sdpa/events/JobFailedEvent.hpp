#ifndef SDPA_JOB_FAILED_EVENT_HPP
#define SDPA_JOB_FAILED_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
	class JobFailedEvent : public JobEvent, public sc::event<JobFailedEvent>
#else
	class JobFailedEvent : public JobEvent
#endif
    {
	public:
		typedef sdpa::shared_ptr<JobFailedEvent> Ptr;

        JobFailedEvent()
          : JobEvent("", "", "")
        {}

		JobFailedEvent(	const address_t& a_from,
						const address_t& a_to,
						const sdpa::job_id_t& a_job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( a_from, a_to, a_job_id ) {
		}

		virtual ~JobFailedEvent() {
		}

		std::string str() const { return "JobFailedEvent"; }

        virtual void accept(EventVisitor *visitor)
        {
          visitor->visitJobFailedEvent(this);
        }
	};
}}

#endif
