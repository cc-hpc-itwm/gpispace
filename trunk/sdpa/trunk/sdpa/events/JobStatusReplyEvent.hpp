#ifndef SDPA_ReplyJobStatusEvent_HPP
#define SDPA_ReplyJobStatusEvent_HPP

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class JobStatusReplyEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobStatusReplyEvent> {
	public:
		typedef sdpa::shared_ptr<JobStatusReplyEvent> Ptr;
        typedef int status_t;

		JobStatusReplyEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id, const status_t &a_status = 0)
          :  sdpa::events::JobEvent(from, to, job_id)
          , status_(a_status)
        {
			// std::cout << "Create event 'JobStatusReplyEvent'"<< std::endl;
		}

		virtual ~JobStatusReplyEvent() {
			// std::cout << "Delete event 'JobStatusReplyEvent'"<< std::endl;
		}

		std::string str() const { return "JobStatusReplyEvent"; }

        const status_t &status() const { return status_; }
    private:
      status_t status_;
	};
}}

#endif
