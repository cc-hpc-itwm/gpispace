#ifndef SDPA_CONFIGOKEVENT_HPP
#define SDPA_CONFIGOKEVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class ConfigOkEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::ConfigOkEvent> {
    public:
        typedef sdpa::shared_ptr<ConfigOkEvent> Ptr;

        ConfigOkEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) { }

    	virtual ~ConfigOkEvent() { } 

    	std::string str() const { return "ConfigOkEvent"; }
    };
}}

#endif
