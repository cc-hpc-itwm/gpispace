#ifndef SDPA_SubmitJobEvent_HPP
#define SDPA_SubmitJobEvent_HPP

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/types.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
  class SubmitJobEvent : public JobEvent, public sc::event<sdpa::events::SubmitJobEvent> {
#else
  class SubmitJobEvent : public JobEvent {
#endif
    public:
      typedef sdpa::shared_ptr<SubmitJobEvent> Ptr;

      SubmitJobEvent()
        : JobEvent("", "", sdpa::job_id_t())
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

      std::string encode() const;
      void decode(const std::string &);
    private:
      sdpa::job_desc_t desc_;
      sdpa::job_id_t parent_;
  };
}}

#endif
