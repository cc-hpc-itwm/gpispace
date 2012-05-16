/*
 * =============================================================================
 *
 *       Filename:  JobFailedEvent.hpp
 *
 *    Description:  JobFailedEvent
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =============================================================================
 */

#ifndef SDPA_JOB_FAILED_EVENT_HPP
#define SDPA_JOB_FAILED_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>

namespace sdpa {
  namespace events {

    class JobFailedEvent : public JobEvent
#ifdef USE_BOOST_SC
                         , public sc::event<JobFailedEvent>
#endif
    {
    public:
      typedef sdpa::shared_ptr<JobFailedEvent> Ptr;

      JobFailedEvent()
        : JobEvent("", "", "")
      {}

      JobFailedEvent( const address_t& a_from
                    , const address_t& a_to
                    , const sdpa::job_id_t& a_job_id
                    , const job_result_t &job_result
                    )
        :  sdpa::events::JobEvent( a_from, a_to, a_job_id )
        , result_(job_result)
        , m_result_code (10)
        , m_error_message ("failed")
      { }

      virtual ~JobFailedEvent() {}

      std::string str() const { return "JobFailedEvent"; }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleJobFailedEvent(this);
      }

      const job_result_t &result() const { return result_; }
      job_result_t &result() { return result_; }

      int result_code () const { return m_result_code; }
      int & result_code () { return m_result_code; }

      std::string const & error_message () const { return m_error_message; }
      std::string & error_message () { return m_error_message; }
    private:
      job_result_t result_;
      int m_result_code;
      std::string m_error_message;
    };
  }
}

#endif
