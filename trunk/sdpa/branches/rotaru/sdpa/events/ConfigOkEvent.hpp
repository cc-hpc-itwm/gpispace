#ifndef SDPA_CONFIGOKEVENT_HPP
#define SDPA_CONFIGOKEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/memory.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
    class ConfigOkEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::ConfigOkEvent> {
#else
    class ConfigOkEvent : public sdpa::events::MgmtEvent {
#endif
    public:
        typedef sdpa::shared_ptr<ConfigOkEvent> Ptr;

        ConfigOkEvent() : MgmtEvent("","") { }

    	virtual ~ConfigOkEvent() { } 

    	std::string str() const { return "ConfigOkEvent"; }

        virtual void accept(EventVisitor *visitor)
        {
          visitor->visitConfigOkEvent(this);
        }
    };
}}

#endif
