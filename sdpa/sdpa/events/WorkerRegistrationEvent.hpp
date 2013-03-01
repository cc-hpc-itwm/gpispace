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

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/capability.hpp>

namespace sdpa { namespace events {
  class WorkerRegistrationEvent : public MgmtEvent
  {
    public:
      typedef sdpa::shared_ptr<WorkerRegistrationEvent> Ptr;
      typedef enum { PUSH, REQ } service_model_t;

      WorkerRegistrationEvent()
        : MgmtEvent()
      {}

      WorkerRegistrationEvent(	const address_t& a_from,
    		  	  	  	  	  	const address_t& a_to,
    		            		const unsigned int& capacity = 2,
    		            		const capabilities_set_t& cpbset = capabilities_set_t(),
    		            		const unsigned int& agent_rank = 0,
    		            		const sdpa::worker_id_t& agent_uuid = "" )
		  : MgmtEvent(a_from, a_to),
		    capacity_(capacity),
		    cpbset_(cpbset),
		    rank_(agent_rank),
		    agent_uuid_(agent_uuid)
      { }

      WorkerRegistrationEvent( const WorkerRegistrationEvent& regEvt )
        : MgmtEvent (regEvt)
      {
    	  capacity_ 	 = regEvt.capacity_;
    	  cpbset_ 		 = regEvt.cpbset_;
    	  rank_			 = regEvt.rank_,
    	  agent_uuid_ 	 = regEvt.agent_uuid_;
	  }

    WorkerRegistrationEvent& operator=( const WorkerRegistrationEvent& regEvt )
    {
    	if(this != &regEvt)
    	{
    		*((MgmtEvent*)(this)) = (MgmtEvent&)(regEvt);
    		capacity_ 	 		  = regEvt.capacity_;
    		cpbset_ 			  = regEvt.cpbset_;
    		rank_			 	  = regEvt.rank_,
    		agent_uuid_ 		  = regEvt.agent_uuid_;
    	}

    	return *this;
    }

    virtual ~WorkerRegistrationEvent() { }

    std::string str() const { return "WorkerRegistrationEvent"; }

    const unsigned int& capacity() const { return capacity_; }
    unsigned int& capacity() { return capacity_; }

    const unsigned int& rank() const { return rank_; }
    unsigned int& rank() { return rank_; }

    const capabilities_set_t& capabilities() const { return cpbset_; }
    capabilities_set_t& capabilities() { return cpbset_; }

    const sdpa::worker_id_t& agent_uuid() const { return agent_uuid_;}
    sdpa::worker_id_t& agent_uuid() { return agent_uuid_;}


    template <class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
    	ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(*this);
		ar & capacity_;
		ar & cpbset_;
		ar & rank_;
		ar & agent_uuid_;
	}

	friend class boost::serialization::access;

	 virtual void handleBy(EventHandler *handler)
	  {
		  handler->handleWorkerRegistrationEvent(this);
	  }


    private:
      unsigned int capacity_;
      capabilities_set_t cpbset_;
      unsigned int rank_;
      sdpa::worker_id_t agent_uuid_;
  };
}}

#endif
