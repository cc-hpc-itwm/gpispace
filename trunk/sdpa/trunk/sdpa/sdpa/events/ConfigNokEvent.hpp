/*
 * =====================================================================================
 *
 *       Filename:  ConfigNokEvent.hpp
 *
 *    Description:  ConfigNokEvent
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef SDPA_CONFIGNOKEVENT_HPP
#define SDPA_CONFIGNOKEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/memory.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
    class ConfigNokEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::ConfigNokEvent> {
#else
    class ConfigNokEvent : public sdpa::events::MgmtEvent {
#endif
    public:
        typedef sdpa::shared_ptr<ConfigNokEvent> Ptr;

        ConfigNokEvent() : MgmtEvent("illegal-src", "illegal-dst") { }

    	virtual ~ConfigNokEvent() { }

    	std::string str() const { return "ConfigNokEvent"; }

        virtual void handleBy(EventHandler *handler)
        {
          handler->handleConfigNokEvent(this);
        }
    };
}}

#endif
