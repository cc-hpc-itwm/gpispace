/*
 * =====================================================================================
 *
 *       Filename:  DeleteJobAckEvent.hpp
 *
 *    Description:  DeleteJobAckEvent
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
#ifndef SDPA_DeleteJobAckEvent_HPP
#define SDPA_DeleteJobAckEvent_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
    class DeleteJobAckEvent : public JobEvent, public sc::event<DeleteJobAckEvent> {
#else
    class DeleteJobAckEvent : public JobEvent {
#endif
    public:
        typedef sdpa::shared_ptr<DeleteJobAckEvent> Ptr;

        DeleteJobAckEvent()
         :  JobEvent( "", "", "", message_id_type() )
        {}

        DeleteJobAckEvent(const address_t& a_from
                        , const address_t& a_to
                        , const sdpa::job_id_t& a_job_id
						, const message_id_type &mid)
         :  sdpa::events::JobEvent( a_from, a_to, a_job_id, mid )
        {}

    	  virtual ~DeleteJobAckEvent() { }

    	  std::string str() const { return "DeleteJobAckEvent"; }

        virtual void handleBy(EventHandler *handler)
        {
		      handler->handleDeleteJobAckEvent(this);
        }
    };
}}

#endif
