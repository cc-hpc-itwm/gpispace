/*
 * =====================================================================================
 *
 *       Filename:  WorkerRegistrationAckEvent.hpp
 *
 *    Description:  WorkerRegistrationAckEvent
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
#ifndef SDPA_WORKER_REGISTRATION_ACK_EVENT_HPP
#define SDPA_WORKER_REGISTRATION_ACK_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa { namespace events {
  class WorkerRegistrationAckEvent : public MgmtEvent
  {
    public:
      typedef sdpa::shared_ptr<WorkerRegistrationAckEvent> Ptr;

      WorkerRegistrationAckEvent()
        : MgmtEvent()
      {}
      WorkerRegistrationAckEvent(const address_t& a_from, const address_t& a_to) : MgmtEvent(a_from, a_to) { }

      virtual ~WorkerRegistrationAckEvent() { }

      virtual void handleBy(EventHandler *handler)
      {
      	  handler->handleWorkerRegistrationAckEvent(this);
      }


      std::string str() const { return "WorkerRegistrationAckEvent"; }


  };
}}

#endif
