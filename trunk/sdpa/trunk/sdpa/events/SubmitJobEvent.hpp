#ifndef SDPA_SubmitJobEvent_HPP
#define SDPA_SubmitJobEvent_HPP

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
#include <sdpa/types.hpp>

namespace sc = boost::statechart;

namespace sdpa {
  namespace events {
    class SubmitJobEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::SubmitJobEvent> {
      public:
        typedef sdpa::shared_ptr<SubmitJobEvent> Ptr;

        SubmitJobEvent( const address_t& a_from
                      , const address_t& a_to
                      , const job_desc_t& a_description = sdpa::job_desc_t("")
                      , const sdpa::job_id_t& a_job_id = sdpa::job_id_t(""))
          : sdpa::events::JobEvent( a_from, a_to, a_job_id )
          , desc_(a_description)
        {
        }

        virtual ~SubmitJobEvent() {
        }

        std::string str() const { return "SubmitJobEvent"; }

        const sdpa::job_desc_t & description() const {return desc_;}
      private:
        sdpa::job_desc_t desc_;
    };
  }
}

#endif
