/*
 * =====================================================================================
 *
 *       Filename:  RunJobEvent.hpp
 *
 *    Description:  RunJobEvent
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
#ifndef SDPA_RUNJOBEVENT_HPP
#define SDPA_RUNJOBEVENT_HPP

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>

namespace sdpa { namespace events {
  class RunJobEvent : public JobEvent
  {
    public:
      typedef sdpa::shared_ptr<RunJobEvent> Ptr;

      RunJobEvent()
        : JobEvent("", "", "")
      {}
      RunJobEvent(const address_t &a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t())
        : JobEvent(a_from, a_to, a_job_id) { }

      virtual ~RunJobEvent() { }

      std::string str() const { return "RunJobEvent"; }
  };
}}


#endif
