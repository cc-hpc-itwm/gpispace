#ifndef SDPA_DeleteJobAckEvent_HPP
#define SDPA_DeleteJobAckEvent_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class DeleteJobAckEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::DeleteJobAckEvent> {
    public:
        typedef sdpa::shared_ptr<DeleteJobAckEvent> Ptr;

        DeleteJobAckEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t(""))
              :  sdpa::events::JobEvent( a_from, a_to, a_job_id )
        { }

    	virtual ~DeleteJobAckEvent() { }

    	std::string str() const { return "DeleteJobAckEvent"; }
    };
}}

#endif
