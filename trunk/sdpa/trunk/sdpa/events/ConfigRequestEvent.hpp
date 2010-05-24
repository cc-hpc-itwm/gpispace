/*
 * =====================================================================================
 *
 *       Filename:  ConfigRequestEvent.hpp
 *
 *    Description:  ConfigRequestEvent
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
#ifndef SDPA_CONFIGREQUESTEVENT_HPP
#define SDPA_CONFIGREQUESTEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa {
namespace events {
#ifdef USE_BOOST_SC
    class ConfigRequestEvent : public MgmtEvent, public sc::event<ConfigRequestEvent> {
#else
    class ConfigRequestEvent : public MgmtEvent {
#endif
    public:
        typedef sdpa::shared_ptr<ConfigRequestEvent> Ptr;

        ConfigRequestEvent()
          : MgmtEvent()
        {}

        ConfigRequestEvent(const address_t& from
                         , const address_t& to)
          : MgmtEvent(from, to)
        {}

    	virtual ~ConfigRequestEvent() { }

    	std::string str() const { return "ConfigRequestEvent"; }

        virtual void handleBy(EventHandler *handler)
        {
          handler->handleConfigRequestEvent(this);
        }
    };
}}

#endif
