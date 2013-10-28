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

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/assume_abstract.hpp>

namespace sdpa { namespace events {
  class SubscribeEvent : public MgmtEvent
  {
    public:
      typedef sdpa::shared_ptr<SubscribeEvent> Ptr;

      SubscribeEvent()
        : MgmtEvent()
      {}

      SubscribeEvent( const address_t& a_from, const address_t& a_to, const job_id_list_t& listJobIds )
		  : MgmtEvent(a_from, a_to),
		    subscriber_(a_from),
		    listJobIds_(listJobIds)
      { }

    virtual ~SubscribeEvent() { }

    std::string str() const { return "SubscribeEvent"; }

    const sdpa::agent_id_t& subscriber() const { return subscriber_;}
    sdpa::agent_id_t& subscriber() { return subscriber_;}

    const sdpa::job_id_list_t& listJobIds() const { return listJobIds_;}
    sdpa::job_id_list_t& listJobIds() { return listJobIds_;}

    virtual void handleBy(EventHandler *handler)
    {
    	handler->handleSubscribeEvent(this);
    }

    private:
    	sdpa::agent_id_t subscriber_;
    	sdpa::job_id_list_t listJobIds_;
  };
}}

#endif
