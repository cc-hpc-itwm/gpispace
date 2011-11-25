/*
 * =====================================================================================
 *
 *       Filename:  SubscribeAckEvent.hpp
 *
 *    Description:  SubscribeAckEvent
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
#ifndef SDPA_SUBSCRIBE_ACK_EVENT_HPP
#define SDPA_SUBSCRIBE_ACK_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/MgmtEvent.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/assume_abstract.hpp>


namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
  class SubscribeAckEvent : public MgmtEvent, public sc::event<SubscribeAckEvent>
#else
  class SubscribeAckEvent : public MgmtEvent
#endif
  {
    public:
      typedef sdpa::shared_ptr<SubscribeAckEvent> Ptr;

      SubscribeAckEvent()
        : MgmtEvent()
      {}

      SubscribeAckEvent(const address_t& a_from, const address_t& a_to, const job_id_list_t& listJobIds )
	  	  : MgmtEvent(a_from, a_to),
	  	    listJobIds_(listJobIds)
      { }

      SubscribeAckEvent( const SubscribeAckEvent& subscribeEvt )
             : MgmtEvent (subscribeEvt)
      {
		  listJobIds_ = subscribeEvt.listJobIds_;
	  }

      SubscribeAckEvent& operator=( const SubscribeAckEvent& subscribeEvt )
      {
    	  if(this != &subscribeEvt)
    	  {
    		  listJobIds_ = subscribeEvt.listJobIds_;
    	  }

    	  return *this;
      }

      virtual ~SubscribeAckEvent() { }

      const sdpa::job_id_list_t& listJobIds() const { return listJobIds_;}
      sdpa::job_id_list_t& listJobIds() { return listJobIds_;}

      virtual void handleBy(EventHandler *handler)
      {
    	  handler->handleSubscribeAckEvent(this);
      }

      std::string str() const { return "SubscribeAckEvent"; }

    private:
      sdpa::job_id_list_t listJobIds_;
  };
}}

#endif
