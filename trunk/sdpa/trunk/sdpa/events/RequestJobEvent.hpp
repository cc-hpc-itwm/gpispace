#ifndef SDPA_REQUESTJOBEVENT_HPP
#define SDPA_REQUESTJOBEVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class RequestJobEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::RequestJobEvent> {
    public:
        typedef sdpa::shared_ptr<RequestJobEvent> Ptr;

        RequestJobEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
        }

    	virtual ~RequestJobEvent() {
    	}

    	const sdpa::job_id_t & last_job_id() const { return last_job_id_; }

    	std::string str() const { return "RequestJobEvent"; }
    private:
            sdpa::job_id_t last_job_id_;
    };
}}

#endif
