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

      WorkerRegistrationEvent(	const address_t& a_from,
    		  	  	  	  	  	const address_t& a_to,
    		            		const unsigned int rank = 0,
    		            		const unsigned int capacity = 2,
    		            		const sdpa::worker_id_t& agent_uuid = "" )
		  : MgmtEvent(a_from, a_to),
		    rank_(rank),
		    capacity_(capacity),
		    agent_uuid_(agent_uuid)
      { }

      virtual ~WorkerRegistrationEvent() { }

      std::string str() const { return "WorkerRegistrationEvent"; }

      const unsigned int& rank() const { return rank_; }
      unsigned int& rank() { return rank_; }

      const unsigned int& capacity() const { return capacity_; }
      unsigned int& capacity() { return capacity_; }

      const sdpa::worker_id_t& agent_uuid() const { return agent_uuid_;}
      sdpa::worker_id_t& agent_uuid() { return agent_uuid_;}

      virtual void handleBy(EventHandler *handler)
      {
    	  handler->handleWorkerRegistrationEvent(this);
      }

    private:
      unsigned int rank_;
      unsigned int capacity_;
      sdpa::worker_id_t agent_uuid_;
  };
}}

#endif
