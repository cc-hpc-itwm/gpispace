#ifndef SDPA_CONFIGREQUESTEVENT_HPP
#define SDPA_CONFIGREQUESTEVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class ConfigRequestEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::ConfigRequestEvent> {
    public:
        typedef sdpa::shared_ptr<ConfigRequestEvent> Ptr;

        ConfigRequestEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) { }

    	virtual ~ConfigRequestEvent() { }

    	std::string str() const { return "ConfigRequestEvent"; }
    };
}}

#endif
