/*
 * =====================================================================================
 *
 *       Filename:  RetrieveJobResultsEvent.hpp
 *
 *    Description:  RetrieveJobResultsEvent
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
#ifndef SDPA_RETRIEVEJOBRESULTSEVENT_HPP
#define SDPA_RETRIEVEJOBRESULTSEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
  class RetrieveJobResultsEvent : public JobEvent, public sc::event<RetrieveJobResultsEvent>
#else
  class RetrieveJobResultsEvent : public JobEvent
#endif
  {
    public:
      typedef sdpa::shared_ptr<RetrieveJobResultsEvent> Ptr;

      RetrieveJobResultsEvent()
        : JobEvent("", "", "")
      {}

      RetrieveJobResultsEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t())
        : sdpa::events::JobEvent(a_from, a_to, a_job_id)
      {
      }

      virtual ~RetrieveJobResultsEvent() { }

      std::string str() const { return "RetrieveJobResultsEvent"; }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleRetrieveJobResultsEvent(this);
      }
  };
}}


#endif
