/*
 * =====================================================================================
 *
 *       Filename:  CancelJobEvent.hpp
 *
 *    Description:  CancelJobEvent
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
#ifndef SDPA_CANCELJOBEVENT_HPP
#define SDPA_CANCELJOBEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>

namespace sdpa { namespace events {
    class CancelJobEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<CancelJobEvent> Ptr;

      CancelJobEvent()
        : JobEvent("", "", "")
        , m_reason ("unknown")
      {}

      CancelJobEvent( const address_t& a_from
                    , const address_t& a_to
                    , const sdpa::job_id_t& a_job_id
                    , const std::string & a_reason
                    )
        : sdpa::events::JobEvent( a_from
                                , a_to
                                , a_job_id
                                )
        , m_reason (a_reason)
      {}

      virtual ~CancelJobEvent() {}

      std::string str() const
      {
        return "CancelJobEvent(" + job_id ().str () + ")";
      }

      std::string const & reason () const
      {
        return m_reason;
      }

      int priority() const { return 1; }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleCancelJobEvent(this);
      }
    private:
      std::string m_reason;
    };
  }}

#endif
