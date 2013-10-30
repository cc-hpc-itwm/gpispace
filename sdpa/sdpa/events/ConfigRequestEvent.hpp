#ifndef SDPA_CONFIGREQUESTEVENT_HPP
#define SDPA_CONFIGREQUESTEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa {
namespace events {
    class ConfigRequestEvent : public MgmtEvent {
    public:
        typedef sdpa::shared_ptr<ConfigRequestEvent> Ptr;

        ConfigRequestEvent()
          : MgmtEvent()
        {}

        ConfigRequestEvent(const address_t& from
                         , const address_t& to)
          : MgmtEvent(from, to)
        {}

    	std::string str() const { return "ConfigRequestEvent"; }

        virtual void handleBy(EventHandler *handler)
        {
          handler->handleConfigRequestEvent(this);
        }
    };
}}

#endif
