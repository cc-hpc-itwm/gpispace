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


#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/memory.hpp>

namespace sdpa { namespace events {
    class ConfigNokEvent : public sdpa::events::MgmtEvent {
    public:
        typedef sdpa::shared_ptr<ConfigNokEvent> Ptr;

        ConfigNokEvent(const address_t& a_from="", const address_t& a_to="") : MgmtEvent(a_from, a_to) { }

    	std::string str() const { return "ConfigNokEvent"; }

        virtual void handleBy(EventHandler *handler)
        {
          handler->handleConfigNokEvent(this);
        }
    };
}}

#endif
