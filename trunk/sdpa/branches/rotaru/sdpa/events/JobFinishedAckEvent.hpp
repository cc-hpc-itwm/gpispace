#ifndef SDPA_JOB_FINISHED_ACK_EVENT_HPP
#define SDPA_JOB_FINISHED_ACK_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
	class JobFinishedAckEvent : public JobEvent, public sc::event<JobFinishedAckEvent>
#else
	class JobFinishedAckEvent : public JobEvent
#endif
    {
	public:
		typedef sdpa::shared_ptr<JobFinishedAckEvent> Ptr;

        JobFinishedAckEvent()
          : JobEvent("", "", "", 0)
        {}

		JobFinishedAckEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id, const message_id_type &mid)
          :  sdpa::events::JobEvent( a_from, a_to, a_job_id, mid ) {
		}

		virtual ~JobFinishedAckEvent() {
		}

		std::string str() const { return "JobFinishedAckEvent"; }

        virtual void accept(EventVisitor *visitor)
        {
          visitor->visitJobFinishedAckEvent(this);
        }
	};
}}

#endif
