/*
 * =====================================================================================
 *
 *       Filename:  JobFinishedEvent.hpp
 *
 *    Description:  JobFinishedEvent
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
#ifndef SDPA_JOB_FINISHED_EVENT_HPP
#define SDPA_JOB_FINISHED_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
  class JobFinishedEvent : public JobEvent, public sc::event<JobFinishedEvent>
#else
  class JobFinishedEvent : public JobEvent
#endif
  {
    public:
      typedef sdpa::shared_ptr<JobFinishedEvent> Ptr;

      JobFinishedEvent()
        : JobEvent("", "", "")
      {}
      JobFinishedEvent(const address_t &a_from
                     , const address_t &a_to
                     , const sdpa::job_id_t &a_job_id
                     , const job_result_t &job_result)
        : sdpa::events::JobEvent( a_from, a_to, a_job_id )
        , result_(job_result)
      { }

      virtual ~JobFinishedEvent() { }

      std::string str() const { return "JobFinishedEvent"; }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleJobFinishedEvent(this);
      }

      const job_result_t &result() const { return result_; }
      job_result_t &result() { return result_; }
    private:
      job_result_t result_;
  };
}}

#endif
