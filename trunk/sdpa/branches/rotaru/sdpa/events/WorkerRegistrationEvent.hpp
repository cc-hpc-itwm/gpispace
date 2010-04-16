/*
 * =====================================================================================
 *
 *       Filename:  WorkerRegistrationEvent.hpp
 *
 *    Description:  WorkerRegistrationEvent
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
#ifndef SDPA_WORKER_REGISTRATION_EVENT_HPP
#define SDPA_WORKER_REGISTRATION_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
  class WorkerRegistrationEvent : public MgmtEvent, public sc::event<WorkerRegistrationEvent>
#else
  class WorkerRegistrationEvent : public MgmtEvent
#endif
  {
    public:
      typedef sdpa::shared_ptr<WorkerRegistrationEvent> Ptr;

      WorkerRegistrationEvent()
        : MgmtEvent()
      {}

      WorkerRegistrationEvent(const address_t& a_from, const address_t& a_to) : MgmtEvent(a_from, a_to) { }

      virtual ~WorkerRegistrationEvent() { }

      std::string str() const { return "WorkerRegistrationEvent"; }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleWorkerRegistrationEvent(this);
      }
  };
}}

#endif
