/*
 * =====================================================================================
 *
 *       Filename:  ConfigOkEvent.hpp
 *
 *    Description:  ConfigOkEvent
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

        ConfigOkEvent(const address_t& a_from="", const address_t& a_to="") : MgmtEvent(a_from, a_to) { }

    	virtual ~ConfigOkEvent() { } 

    	std::string str() const { return "ConfigOkEvent"; }

        virtual void handleBy(EventHandler *handler)
        {
          handler->handleConfigOkEvent(this);
        }
    };
}}

#endif
