#ifndef SDPA_SubmitJobEvent_HPP
#define SDPA_SubmitJobEvent_HPP

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
#include <sdpa/types.hpp>

namespace sc = boost::statechart;

namespace sdpa { namespace events {
  class SubmitJobEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::SubmitJobEvent> {
    public:
      typedef sdpa::shared_ptr<SubmitJobEvent> Ptr;

      SubmitJobEvent()
        : sdpa::events::JobEvent("", "", sdpa::job_id_t())
        , desc_()
        , parent_()
      {}

      SubmitJobEvent( const address_t& a_from
          , const address_t& a_to
          , const sdpa::job_id_t& a_job_id = sdpa::job_id_t("")
          , const job_desc_t& a_description = sdpa::job_desc_t("")
          , const sdpa::job_id_t& a_parent_id = sdpa::job_id_t("")) 
        : sdpa::events::JobEvent( a_from, a_to, a_job_id ), desc_(a_description), parent_(a_parent_id)
        { }
        
      virtual ~SubmitJobEvent() { }

      std::string str() const { return "SubmitJobEvent"; }

      const sdpa::job_desc_t & description() const {return desc_;}
      sdpa::job_desc_t & description() {return desc_;}

      const sdpa::job_id_t &parent_id() const { return parent_; }
      sdpa::job_id_t &parent_id() { return parent_; }
    private:
      sdpa::job_desc_t desc_;
      sdpa::job_id_t parent_;
  };
}}

#endif
