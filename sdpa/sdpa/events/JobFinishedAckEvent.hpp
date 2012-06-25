/*
 * =====================================================================================
 *
 *       Filename:  JobFinishedAckEvent.hpp
 *
 *    Description:  JobFinishedAckEvent
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
#ifndef SDPA_JOB_FINISHED_ACK_EVENT_HPP
#define SDPA_JOB_FINISHED_ACK_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
	class JobFinishedAckEvent : public JobEvent, public sc::event<JobFinishedAckEvent>
#else
	class JobFinishedAckEvent : public JobEvent
#endif
    {
	public:
		typedef sdpa::shared_ptr<JobFinishedAckEvent> Ptr;

        JobFinishedAckEvent()
          : JobEvent("", "", "")
        {}

		JobFinishedAckEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id)
          :  sdpa::events::JobEvent( a_from, a_to, a_job_id ) {
		}

		virtual ~JobFinishedAckEvent() {
		}

		std::string str() const { return "JobFinishedAckEvent"; }

        virtual void handleBy(EventHandler *handler)
        {
          handler->handleJobFinishedAckEvent(this);
        }
	};
}}

#endif
