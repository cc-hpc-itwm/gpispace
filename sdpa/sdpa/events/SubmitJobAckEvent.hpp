/*
 * =====================================================================================
 *
 *       Filename:  SubmitJobAckEvent.hpp
 *
 *    Description:  SubmitJobAckEvent
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
#ifndef SDPA_SubmitJobAckEvent_HPP
#define SDPA_SubmitJobAckEvent_HPP

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa { namespace events {
  class SubmitJobAckEvent : public JobEvent
  {
    public:
      typedef sdpa::shared_ptr<SubmitJobAckEvent> Ptr;

      SubmitJobAckEvent()
        : JobEvent("", "", "", message_id_type())
      { }

      SubmitJobAckEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t & a_job_id, const message_id_type &mid)
        : JobEvent(a_from, a_to, a_job_id, mid)
      {
      }

      virtual ~SubmitJobAckEvent() {
      }

    std::string str() const { return "SubmitJobAckEvent(" + job_id ().str () + ")"; }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleSubmitJobAckEvent(this);
      }
  };
}}

#endif
