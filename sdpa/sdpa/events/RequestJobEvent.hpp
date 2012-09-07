/*
 * =====================================================================================
 *
 *       Filename:  RequestJobEvent.hpp
 *
 *    Description:  RequestJobEvent
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
#ifndef SDPA_REQUESTJOBEVENT_HPP
#define SDPA_REQUESTJOBEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
  class RequestJobEvent : public MgmtEvent, public sc::event<RequestJobEvent>
#else
  class RequestJobEvent : public MgmtEvent
#endif
  {
    public:
      typedef sdpa::shared_ptr<RequestJobEvent> Ptr;

      RequestJobEvent()
        : MgmtEvent("", "")
        , last_job_id_(sdpa::job_id_t::invalid_job_id())
      {}

      RequestJobEvent(const address_t &a_from
                    , const address_t &a_to
                    , const sdpa::job_id_t &a_job_id = sdpa::job_id_t::invalid_job_id())
        : MgmtEvent(a_from, a_to)
        , last_job_id_(a_job_id)
      {
      }

      virtual ~RequestJobEvent() { }

      const sdpa::job_id_t & last_job_id() const { return last_job_id_; }
      sdpa::job_id_t & last_job_id() { return last_job_id_; }

      std::string str() const
      {
        return "RequestJobEvent(" + last_job_id ().str () + ")";
      }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleRequestJobEvent(this);
      }
    private:
      sdpa::job_id_t last_job_id_;
  };
}}

#endif
