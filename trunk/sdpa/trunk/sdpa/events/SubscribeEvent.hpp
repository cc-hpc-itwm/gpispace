/*
 * =====================================================================================
 *
 *       Filename:  SubscribeEvent.hpp
 *
 *    Description:  SubscribeEvent
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
#ifndef SDPA_SUBSCRIBE_EVENT_HPP
#define SDPA_SUBSCRIBE_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/assume_abstract.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
  class SubscribeEvent : public MgmtEvent, public sc::event<SubscribeEvent>
#else
  class SubscribeEvent : public MgmtEvent
#endif
  {
    public:
      typedef sdpa::shared_ptr<SubscribeEvent> Ptr;

      SubscribeEvent()
        : MgmtEvent()
      {}

      SubscribeEvent( const address_t& a_from, const address_t& a_to )
		  : MgmtEvent(a_from, a_to),
		    subscriber_(a_from)
      { }

      SubscribeEvent( const SubscribeEvent& subscribeEvt )
        : MgmtEvent (subscribeEvt)
      {
    	  subscriber_ = subscribeEvt.subscriber_;
	  }

    SubscribeEvent& operator=( const SubscribeEvent& subscribeEvt )
    {
    	if(this != &subscribeEvt)
    	{
    		*((MgmtEvent*)(this)) = (MgmtEvent&)(subscribeEvt);
    		subscriber_ 	 	  = subscribeEvt.subscriber_;
    	}

    	return *this;
    }

    virtual ~SubscribeEvent() { }

    std::string str() const { return "SubscribeEvent"; }


    const sdpa::agent_id_t& subscriber() const { return subscriber_;}
    sdpa::agent_id_t& subscriber() { return subscriber_;}

    virtual void handleBy(EventHandler *handler)
    {
    	handler->handleSubscribeEvent(this);
    }

    private:
      sdpa::agent_id_t subscriber_;
  };
}}

#endif
