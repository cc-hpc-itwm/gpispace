#ifndef SDPA_ConfigReplyEvent_HPP
#define SDPA_ConfigReplyEvent_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
    class ConfigReplyEvent : public MgmtEvent, public sc::event<ConfigReplyEvent> {
#else
    class ConfigReplyEvent : public MgmtEvent {
#endif
    public:
        typedef sdpa::shared_ptr<ConfigReplyEvent> Ptr;

        ConfigReplyEvent()
          : MgmtEvent()
        {}

        ConfigReplyEvent(const address_t& from
                       , const address_t& to)
          : MgmtEvent(from, to)
        {}

    	virtual ~ConfigReplyEvent() { }

    	std::string str() const { return "ConfigReplyEvent"; }
    };
}}

#endif
