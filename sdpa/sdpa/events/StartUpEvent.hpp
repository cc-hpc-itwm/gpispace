/*
 * =====================================================================================
 *
 *       Filename:  StartUpEvent.hpp
 *
 *    Description:  StartUpEvent
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
#ifndef SDPA_STARTUPEVENT_HPP
#define SDPA_STARTUPEVENT_HPP

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/memory.hpp>

namespace sdpa { namespace events {
  class StartUpEvent : public MgmtEvent {
    public:
      typedef sdpa::shared_ptr<StartUpEvent> Ptr;

      StartUpEvent(const address_t& a_from="", const address_t& a_to="", const std::string& strCfgFile="")
      	  : MgmtEvent(a_from, a_to), m_strCfgFile(strCfgFile) { }

      virtual ~StartUpEvent() { }

      std::string str() const { return "StartUpEvent"; }
      std::string cfgFile() const { return m_strCfgFile; }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleStartUpEvent(this);
      }

    private:
      std::string m_strCfgFile;
  };
}}

#endif
