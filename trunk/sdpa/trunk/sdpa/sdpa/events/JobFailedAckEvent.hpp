/*
 * =====================================================================================
 *
 *       Filename:  JobFailedAckEvent.hpp
 *
 *    Description:  JobFailedAckEvent
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
#ifndef SDPA_JOB_FAILED_ACK_EVENT_HPP
#define SDPA_JOB_FAILED_ACK_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
	class JobFailedAckEvent : public JobEvent, public sc::event<JobFailedAckEvent>
#else
	class JobFailedAckEvent : public JobEvent
#endif
    {
	public:
		typedef sdpa::shared_ptr<JobFailedAckEvent> Ptr;

        JobFailedAckEvent()
          : JobEvent("", "", "", message_id_type())
        {}

		JobFailedAckEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id, const message_id_type &mid)
          :  sdpa::events::JobEvent( a_from, a_to, a_job_id, mid ) {
		}

		virtual ~JobFailedAckEvent() {
		}

		std::string str() const { return "JobFailedAckEvent"; }

        virtual void handleBy(EventHandler *handler)
        {
          handler->handleJobFailedAckEvent(this);
        }
	};
}}

#endif
