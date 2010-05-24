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

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
  class WorkerRegistrationAckEvent : public MgmtEvent, public sc::event<WorkerRegistrationAckEvent>
#else
  class WorkerRegistrationAckEvent : public MgmtEvent
#endif
  {
    public:
      typedef sdpa::shared_ptr<WorkerRegistrationAckEvent> Ptr;

      WorkerRegistrationAckEvent()
        : MgmtEvent()
      {}
      WorkerRegistrationAckEvent(const address_t& a_from, const address_t& a_to) : MgmtEvent(a_from, a_to) { }

      virtual ~WorkerRegistrationAckEvent() { }

      std::string str() const { return "WorkerRegistrationAckEvent"; }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleWorkerRegistrationAckEvent(this);
      }
  };
}}

#endif
