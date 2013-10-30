#ifndef SDPA_CONFIGOKEVENT_HPP
#define SDPA_CONFIGOKEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/memory.hpp>

namespace sdpa { namespace events {
    class ConfigOkEvent : public sdpa::events::MgmtEvent {
    public:
        typedef sdpa::shared_ptr<ConfigOkEvent> Ptr;

        ConfigOkEvent(const address_t& a_from="", const address_t& a_to="") : MgmtEvent(a_from, a_to) { }

    	std::string str() const { return "ConfigOkEvent"; }

        virtual void handleBy(EventHandler *handler)
        {
          handler->handleConfigOkEvent(this);
        }
    };
}}

#endif
