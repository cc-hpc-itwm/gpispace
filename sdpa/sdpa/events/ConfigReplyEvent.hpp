/*
 * =====================================================================================
 *
 *       Filename:  ConfigReplyEvent.hpp
 *
 *    Description:  ConfigReplyEvent
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
#ifndef SDPA_ConfigReplyEvent_HPP
#define SDPA_ConfigReplyEvent_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa { namespace events {
    class ConfigReplyEvent : public MgmtEvent {
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

        virtual void handleBy(EventHandler *handler)
        {
          handler->handleConfigReplyEvent(this);
        }
    };
}}

#endif
