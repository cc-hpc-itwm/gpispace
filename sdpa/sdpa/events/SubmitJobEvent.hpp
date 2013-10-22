/*
 * =====================================================================================
 *
 *       Filename:  SubmitJobEvent.hpp
 *
 *    Description:  SubmitJobEvent
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
#ifndef SDPA_SubmitJobEvent_HPP
#define SDPA_SubmitJobEvent_HPP

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/types.hpp>

namespace sdpa { namespace events {
  class SubmitJobEvent : public JobEvent {
    public:
      typedef sdpa::shared_ptr<SubmitJobEvent> Ptr;

      SubmitJobEvent()
        : JobEvent("", "", sdpa::job_id_t())
        , desc_()
        , parent_()
      {}

      SubmitJobEvent( const address_t& a_from
          , const address_t& a_to
          , const sdpa::job_id_t& a_job_id
          , const job_desc_t& a_description
          , const sdpa::job_id_t& a_parent_id
          , const sdpa::worker_id_list_t& worker_list = sdpa::worker_id_list_t() )
        : sdpa::events::JobEvent( a_from, a_to, a_job_id ),
          desc_(a_description),
          parent_(a_parent_id),
          worker_list_(worker_list)
        { }

      virtual ~SubmitJobEvent() { }

    std::string str() const { return "SubmitJobEvent(" + job_id ().str () + ")"; }

      const sdpa::job_desc_t& description() const {return desc_;}
      sdpa::job_desc_t& description() {return desc_;}

      const sdpa::job_id_t& parent_id() const { return parent_; }
      sdpa::job_id_t& parent_id() { return parent_; }

      const sdpa::worker_id_list_t& worker_list() const { return worker_list_; }
      sdpa::worker_id_list_t& worker_list() { return worker_list_; }

      virtual void handleBy(EventHandler *handler)
      {
          handler->handleSubmitJobEvent(this);
      }
    private:
      sdpa::job_desc_t desc_;
      sdpa::job_id_t parent_;
      sdpa::worker_id_list_t worker_list_;
  };
}}

#endif
