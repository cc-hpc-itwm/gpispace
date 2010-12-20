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
#include <sdpa/types.hpp>

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

      WorkerRegistrationEvent(const address_t& a_from, const address_t& a_to, const int rank, const sdpa::worker_id_t& agent_uuid = "" )
		  : MgmtEvent(a_from, a_to), rank_(rank), agent_uuid_(agent_uuid) { }

      virtual ~WorkerRegistrationEvent() { }

      std::string str() const { return "WorkerRegistrationEvent"; }

      const int& rank() const { return rank_; }
      int& rank() { return rank_; }

      const sdpa::worker_id_t& agent_uuid() const { return agent_uuid_;}
      sdpa::worker_id_t& agent_uuid() { return agent_uuid_;}

      virtual void handleBy(EventHandler *handler)
      {
    	  handler->handleWorkerRegistrationEvent(this);
      }

    private:
      int rank_;
      sdpa::worker_id_t agent_uuid_;
  };
}}

#endif
